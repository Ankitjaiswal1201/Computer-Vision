#include <iostream>
#include <math.h>

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "Segmentation.h"
#include "Filter.h"

////////////////////////////////////////////////////////////////////////////////////
// constructor and destructor
////////////////////////////////////////////////////////////////////////////////////
Segmentation::Segmentation(){}

Segmentation::~Segmentation(){}

////////////////////////////////////////////////////////////////////////////////////
// Cut template image from input and save as file
////////////////////////////////////////////////////////////////////////////////////
void Segmentation::cutAndSave(const cv::Mat &input, cv::Point origin, cv::Size size, const cv::string &filename)
{
    // define rectangle from origin and size
    cv::Rect rect(origin, size);

    // cut the rectangle and create template
    cv::Mat templ = input(rect);

    // save template to file
    cv::imwrite(filename, templ);
}

////////////////////////////////////////////////////////////////////////////////////
// Find brightest pixel and return its coordinates as Point
////////////////////////////////////////////////////////////////////////////////////
cv::Point Segmentation::findMaximum(const cv::Mat &input)
{
    // declare array to hold the indizes
    int maxIndex[2];

    // find the maximum
    cv::minMaxIdx(input, 0, 0, 0, maxIndex);

    // create Point and return
    return cv::Point(maxIndex[1], maxIndex[0]);
}

////////////////////////////////////////////////////////////////////////////////////
// Add a black rectangle to an image
////////////////////////////////////////////////////////////////////////////////////
void Segmentation::drawRect(const cv::Mat &input, cv::Point origin, cv::Size size, cv::Mat &output)
{
    // define rectangle from origin and size
    cv::Rect rect(origin, size);

    // copy input image to output
    output = input.clone();

    // draw the rectangle
    cv::rectangle(output, rect, 0, 2);
}

///////////////////////////////////////////////////////////////////////////////
// scale a Hough image for better displaying
///////////////////////////////////////////////////////////////////////////////
void Segmentation::scaleHoughImage(const cv::Mat &input, cv::Mat &output)
{
    // find max value
    double max;
    cv::minMaxLoc(input, 0, &max);

    // scale the image
    input.convertTo(output, CV_32F, 1.0f / max, 0);
}


////////////////////////////////////////////////////////////////////////////////////
// Compute Hough Transformation for cirlces
////////////////////////////////////////////////////////////////////////////////////

// cellStep: accumulator cell size
void Segmentation::houghCircle(const cv::Mat &input, cv::Mat &output,
                               const int radius, const float cellStep,
                               const float phiStep)
{
    int rows = input.rows;
    int cols = input.cols;

    // accumulator dimensions
    // a: [0-radius , cols-1+radius]
    // b: [0-radius , rows-1+radius]
    int dimA = ceil(float(cols + radius + radius) / cellStep);
    int dimB = ceil(float(rows + radius + radius) / cellStep);

    // shift in accumulator space
    int shift = round(float(radius) / cellStep);

    // scale the accumulator cell
    float scaleFloat = 1.0f / cellStep;
    int scaleInt = round(scaleFloat);

    // create accumulator
    output.release();
    output = cv::Mat::zeros(dimB, dimA, CV_32S); // 32 bit integer

    // phi deg->rad
    const float phiRadStart = 0.0f;
    const float phiRadEnd = 360.0f * CV_PI / 180.0f;
    const float phiRadStep = phiStep * CV_PI / 180.0f;

    // radius as float
    float r = float(radius);

    // Hough Transformation (circle)
    for (int y = 0; y < rows; ++y) {
        const uchar *pInput = input.ptr<uchar>(y);
        for (int x = 0; x < cols; ++x) {
            if (*pInput++ == 0)
                continue;

            // voting for pixel(y,x)
            for (float phiRad = phiRadStart; phiRad < phiRadEnd;
                phiRad += phiRadStep) {
                int a = (scaleInt * x) - round(r * cos(phiRad) * scaleFloat);
                int b = (scaleInt * y) - round(r * sin(phiRad) * scaleFloat);
                ++output.at<int>(b + shift, a + shift);
            }
        }
    }
}


////////////////////////////////////////////////////////////////////////////////////
// Find circles in an image
// 
// every circle has to be added to the list, e.g.:
//
//      CircleItem newItem; // create a new list item
//      newItem.x = x; // set variables of the list item
//      newItem.y = y;
//      newItem.r = r;
//      list->push_back(newItem); // add the item to the list
//
// accessing the list of circles: see section 'show list of found cirles' in
//                               the following function
////////////////////////////////////////////////////////////////////////////////////

void Segmentation::findCircles(const cv::Mat &input, std::vector<CircleItem> *list, const int radiusMin,
                               const int radiusMax, const float cellStep,
                               const float phiStep)
{
    list->clear();

    ////////////////////////////////////////////////////////////////////////////////////
    // for every radius:
    // - Hough Circle Transformation
    // - find maxima
    // - if the maxima represents a circle center add it to the list
    //
    // note: finding more than one maxima can be done by removing the alrady found
    //       maxima from the Hough transformed image (like it was done in exercise 5b)
    //
    // note: a maxima in a Hough transformed image has a certain value
    //       this value can be used to decide if the the maxima represents a circle
    //       (e.g. you can establish a valueMin and if the value is below valueMin
    //        stop searching for further circle centers in the image)
    ////////////////////////////////////////////////////////////////////////////////////


    // a correct circle with radius r + 1 has a higher value then a correct circle with radius r
    int addValueMax = -1;
    
    cv::Mat hough;
    for (int r = radiusMin; r <= radiusMax; ++r)
    {
        std::cout << "r=" << r << '\r';
        houghCircle(input, hough, r, cellStep, phiStep);
        int maxValue = -1;
        int value = -1;
        while(true)
        {
            cv::Point center = findAndRemoveMaximum(hough, &value, r, cellStep);
            if (maxValue == -1)
                maxValue = value;
            if (value < addValueMax || value < maxValue || value < 1)
                break;

            // point may be a center of a circle
            addValueMax = value;
            addFoundCenter(list, center.x, center.y, r, value);
        }
    }

    // show list of found cirles
    std::cout << "       " << list->size() << " cirles found:\n";
    for (int i = 0; i < list->size(); ++i)
    {
        CircleItem item = list->at(i);
        std::cout << "\t#" << (i + 1) << "\tcenter: (" << item.x << "," << item.y << "), \tradius=" << item.r << "\n";
    }

}

cv::Point Segmentation::findAndRemoveMaximum(cv::Mat &image, int *value, const int radius, const float cellStep)
{
    cv::Point point;
    double min = 0.0;
    double max = 0.0;
    cv::minMaxLoc(image, &min, &max, nullptr, &point);
    *value = int(max);

    // remove Area around maximum
    cv::circle(image, point, 5, cv::Scalar(0, 0, 0), -1);

    // convert accumulator space coordinates to pixel coordinates of the image which was hough transformed
    point.x = round(float(point.x) * cellStep - radius);
    point.y = round(float(point.y) * cellStep - radius);

    return point;
}


void Segmentation::addFoundCenter(std::vector<CircleItem> *list, const int x, const int y, const int r, const int value)
{
    // search for circle with similar center and radius in list
    int dist = 4;
    bool found = false;
    for (int i = 0; i < list->size(); ++i)
    {
        CircleItem *item = &list->at(i);
        if (item->r >= r - 1 && item->r <= r && item->x >= x - dist
            && item->x <= x + dist && item->y >= y - dist && item->y <= y + dist)
        {
            //there is a similar cirle
            found = true;
            if (item->v <= value)
            {
                //std::cout << "update\n";
                // current values are more likely a circle then values in list, therefore update list
                item->x = x;
                item->y = y;
                item->r = r;
                item->v = value;
            }
            break;
        }
    }

    if (found)
        return;

    // nothing similar found in list, threefore, add current data to list
    CircleItem newItem;
    newItem.x = x;
    newItem.y = y;
    newItem.r = r;
    newItem.v = value;
    list->push_back(newItem);
}
