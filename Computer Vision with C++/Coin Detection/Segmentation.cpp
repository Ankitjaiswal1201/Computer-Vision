#include <iostream>
#include <math.h>
#include <thread>

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "Segmentation.h"
#include "Filter.h"


////////////////////////////////////////////////////////////////////////////////////
// constructor and destructor
////////////////////////////////////////////////////////////////////////////////////
Segmentation::Segmentation() {}

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
// Compute normalized cross correlation function
////////////////////////////////////////////////////////////////////////////////////
void Segmentation::crossCorrelate(const cv::Mat &input, const cv::Mat &templ, cv::Mat &output)
{
    int rows = input.rows;
    int cols = input.cols;

    int tRows = templ.rows;
    int tCols = templ.cols;

    output.release();

    // create a float image
    output = cv::Mat(rows - tRows + 1, cols - tCols + 1, CV_32F);

    // calculate template part of the normalization factor
    float normFactor_templ = 0.0f;

    for (int tr = 0; tr < tRows; ++tr)
    {
        const float *pTempl = templ.ptr<float>(tr);

        for (int tc = 0; tc < tCols; ++tc)
        {
            normFactor_templ += pow(*pTempl, 2);

            ++pTempl;
        }
    }

    normFactor_templ = sqrt(normFactor_templ);

    // calculate and normalize cross correlation
    for (int r = 0; r < (rows - tRows + 1); ++r)
    {
        float *pOutput = output.ptr<float>(r);

        for (int c = 0; c < (cols - tCols + 1); ++c)
        {
            float result = 0.0f;
            float normFactor_input = 0.0f;

            for (int tr = 0; tr < tRows; ++tr)
            {
                const float *pInput = input.ptr<float>(r + tr) + c;
                const float *pTempl = templ.ptr<float>(tr);

                for (int tc = 0; tc < tCols; ++tc)
                {
                    result += ((*pInput) * (*pTempl));
                    normFactor_input += pow(*pInput, 2);

                    ++pTempl;
                    ++pInput;
                }
            }

            normFactor_input = sqrt(normFactor_input);

            // normalize the result
            float normFactor = normFactor_input * normFactor_templ;
            result /= (normFactor);

            *pOutput = result;

            ++pOutput;
        }
    }
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

////////////////////////////////////////////////////////////////////////////////////
// Compute Hough Transformation
////////////////////////////////////////////////////////////////////////////////////
void Segmentation::houghTransform(const cv::Mat &input, float phiStep, cv::Mat &output)
{
    int rows = input.rows;
    int cols = input.cols;

    int dMax = (int) sqrt(pow(rows, 2) + pow(cols, 2));
    int phiMax = (int) (180.0f / phiStep);
    float phiStepRad = phiStep * CV_PI / 180.0f;

    output.release();

    // create a 32bit signed integer image initialized with zeros
    output = cv::Mat::zeros(2 * dMax, phiMax, CV_32S);

    // Hough Transformation
    for (int r = 0; r < rows; ++r)
    {
        const float *pInput = input.ptr<float>(r);

        for (int c = 0; c < cols; ++c)
        {
            if (*pInput != 0)
            {
                for (int phi = 0; phi < phiMax; ++phi)
                {
                    // compute phi and d as floating point values
                    float phiRad = (float) phi * phiStepRad;
                    float dFloat = ((float) c * cos(phiRad) + (float) r * sin(phiRad));

                    // round and convert to integer
                    int dInt = (int) (dFloat > 0.0f) ? (dFloat + 0.5f) : (dFloat - 0.5f);

                    // increment in output image
                    ++output.at<int>(dMax + dInt, phi);
                }
            }

            ++pInput;
        }
    }
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
// Find the n local maxima and return the coordinates in a cv::Mat
////////////////////////////////////////////////////////////////////////////////////
cv::Mat Segmentation::findMaxima(const cv::Mat &input, int n)
{
    int yMax = input.rows - 1;
    int xMax = input.cols - 1;

    int rOffset = input.rows / 2;

    // create a copy of the input image
    cv::Mat inputCopy = input.clone();

    // create a 32bit signed integer image initialized with zeros
    cv::Mat maxima = cv::Mat::zeros(n, 2, CV_32S);

    // find n local maxima
    for (int i = 0; i < n; ++i)
    {
        int *pMaxima = maxima.ptr<int>(i);

        // find the maximum
        cv::Point maxPoint = findMaximum(inputCopy);

        // store the maximum in output matrix
        *pMaxima = rOffset - maxPoint.y;
        *(pMaxima + 1) = maxPoint.x;

        // clear the found maximum and all pixels around it
        const int dist = 10;

        // find the indices of the region around the found maximum
        int xStart = maxPoint.x - dist;
        if (xStart < 0)
            xStart = 0;

        int xEnd = maxPoint.x + dist;
        if (xEnd > xMax)
            xEnd = xMax;

        int yStart = maxPoint.y - dist;
        if (yStart < 0)
            yStart = 0;

        int yEnd = maxPoint.y + dist;
        if (yEnd > yMax)
            yEnd = yMax;

        // set the pixel's values to zero
        for (int y = yStart; y <= yEnd; ++y)
        {
            int *pInput = inputCopy.ptr<int>(y) + xStart;

            for (int x = xStart; x <= xEnd; ++x)
            {
                *pInput = 0;
                ++pInput;
            }
        }
    }

    // return the maxima as cv::Mat
    return maxima;
}

////////////////////////////////////////////////////////////////////////////////////
// Draw straight lines
////////////////////////////////////////////////////////////////////////////////////
void Segmentation::drawLines(const cv::Mat &input, cv::Mat lines, float phiStep, cv::Mat &output)
{
    int rows = input.rows;
    int cols = input.cols;

    int nLines = lines.rows;
    float phiStepRad = phiStep * CV_PI / 180.0f;

    // copy input image to output
    output = input.clone();

    // convert to RGB colour image
    cv::cvtColor(output, output, CV_GRAY2RGB);

    // draw the lines
    for (int l = 0; l < nLines; ++l)
    {
        int *pLines = lines.ptr<int>(l);

        // get d and phi
        int d = *pLines;
        float phi = (float) *(pLines + 1) * phiStep;
        float phiRad = (float) *(pLines + 1) * phiStepRad;

        // find start and end point of line
        cv::Point p1;
        cv::Point p2;

        if (phi == 0.0f)
        {
            // vertical line
            p1 = cv::Point(d, 0);
            p2 = cv::Point(d, rows - 1);
        }
        else if (phi == 90.0f)
        {
            // horizontal line
            p1 = cv::Point(0, d);
            p2 = cv::Point(cols - 1, d);
        }
        else if (phi < 90.0f)
        {
            // monotonically decreasing
            int x = d / cos(phiRad);
            int y = d / sin(phiRad);

            p1 = cv::Point(0, y);
            p2 = cv::Point(x, 0);
        }
        else
        {
            // monotonically increasing
            int y = d / sin(phiRad);
            int x = -1.0f * tan(phiRad) * (rows - y);

            p1 = cv::Point(0, y);
            p2 = cv::Point(x, rows - 1);
        }

        // draw a red line
        cv::line(output, p1, p2, cv::Scalar(0.0f, 0.0f, 1.0f), 1);
    }
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
// find maximum in cv::Mat and remove it (so the next local maximum can be found)
////////////////////////////////////////////////////////////////////////////////////
cv::Point Segmentation::findAndRemoveMaximum(cv::Mat &image, int *value, const int radius, const float cellStep)
{
    cv::Point point;
    double min = 0.0;
    double max = 0.0;
    cv::minMaxLoc(image, &min, &max, nullptr, &point);
    *value = int(max);

    if (*value < 1)
    {
        point.x = -1;
        point.y = -1;
        return point;
    }

    int neighborSum =
        image.at<int>(point.y - 1, point.x) + image.at<int>(point.y + 1, point.x) +
        image.at<int>(point.y, point.x - 1) + image.at<int>(point.y, point.x + 1);

    // remove area around maximum
    cv::circle(image, point, 5, cv::Scalar(0, 0, 0), -1);

    // convert accumulator space coordinates to pixel coordinates of the image which was hough transformed
    point.x = round(float(point.x) * cellStep - radius);
    point.y = round(float(point.y) * cellStep - radius);

    return point;
}

////////////////////////////////////////////////////////////////////////////////////
// a cicle (center) was found add it to the list of cirles
////////////////////////////////////////////////////////////////////////////////////
void Segmentation::addFoundCenter(std::vector<CircleItem> *list, const int x, const int y, const int r, const int value)
{
    // search for circle with similar center and radius in list
    int dist = 10;
    bool found = false;
    for (int i = 0; i < list->size(); ++i)
    {
        CircleItem *item = &list->at(i);
        if (item->r >= r - 5 && item->r <= r && item->x >= x - dist
            && item->x <= x + dist && item->y >= y - dist && item->y <= y + dist)
        {
            //there is a similar cirle
            found = true;
            if (item->v <= value)
            {
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
        return; // something similar was found in list, therefore, no need to add it to the list again

    // nothing similar found in list, therefore, add current data to list
    CircleItem newItem;
    newItem.x = x;
    newItem.y = y;
    newItem.r = r;
    newItem.v = value;
    list->push_back(newItem);
}

////////////////////////////////////////////////////////////////////////////////////
// find all circles (whose radius is in the given range) in the image
// and add the circle to the list
////////////////////////////////////////////////////////////////////////////////////
void Segmentation::findCircles(const cv::Mat &input, std::vector<CircleItem> *list, const int radiusMin,
    const int radiusMax, const float cellStep,
    const float phiStep, const int maxCountPerRadius)
{
    list->clear();
    cv::Mat hough;
    for (int r = radiusMin; r <= radiusMax; ++r)
    {
        houghCircle(input, hough, r, cellStep, phiStep);

        int maxValue = -1;
        int value = -1;
        int count = 0;
        while (true) // endless loop, left with 'break'
        {
            if (++count > maxCountPerRadius)
                break;
            cv::Point center = findAndRemoveMaximum(hough, &value, r, cellStep);

            if (maxValue == -1)
                maxValue = value;

            if (value < maxValue || value < 1)
                break;
            // point may be a center of a circle
            addFoundCenter(list, center.x, center.y, r, value);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////
// like function 'findCircles', but uses 4 threads
////////////////////////////////////////////////////////////////////////////////////
void Segmentation::findCirclesThread(const cv::Mat &input, std::vector<CircleItem> *list, const int radiusMin,
    const int radiusMax, const float cellStep,
    const float phiStep, const int maxCountPerRadius)
{
    list->clear();

    // note: number calculations increase with radius and, therefore, not 25% for each thread
    int rSize = radiusMax - radiusMin;
    int r2 = radiusMin + rSize * 0.30f;
    int r3 = radiusMin + rSize * 0.57f;
    int r4 = radiusMin + rSize * 0.80f;

    // start thread 1
    std::thread th1(findCirclesThreadSub, input, list, radiusMin, r2 - 1, cellStep, phiStep, maxCountPerRadius);

    // create new circle lists because different threads can not write to the same list
    std::vector<CircleItem> list2, list3, list4;

    //start thread 2, 3 and 4
    std::thread th2(findCirclesThreadSub, input, &list2, r2, r3 - 1, cellStep, phiStep, maxCountPerRadius);
    std::thread th3(findCirclesThreadSub, input, &list3, r3, r4 - 1, cellStep, phiStep, maxCountPerRadius);
    std::thread th4(findCirclesThreadSub, input, &list4, r4, radiusMax, cellStep, phiStep, maxCountPerRadius);


    // wait for thread 1 to finish
    th1.join();

    // wait for thread 2 to finish
    th2.join();
    // add circles of thread 2 to the list of thread 1 (the original circle list)
    for (auto circle : list2)
    {
        list->push_back(circle);
    }

    // do the same things for thead 3 and 4
    th3.join();
    for (auto circle : list3)
    {
        list->push_back(circle);
    }
    th4.join();
    for (auto circle : list4)
    {
        list->push_back(circle);
    }

    // now: all circles are in the 'list'
}

////////////////////////////////////////////////////////////////////////////////////
// for the thread version of function 'findCircles'
////////////////////////////////////////////////////////////////////////////////////
void Segmentation::findCirclesThreadSub(const cv::Mat &input, std::vector<CircleItem> *list, const int radiusMin,
                                         const int radiusMax, const float cellStep,
                                         const float phiStep, const int maxCountPerRadius)
{
    cv::Mat hough;
    for (int r = radiusMin; r <= radiusMax; ++r)
    {
        houghCircle(input, hough, r, cellStep, phiStep);

        int maxValue = -1;
        int value = -1;
        int count = 0;
        while (true) // endless loop, left with 'break'
        {
            if (++count > maxCountPerRadius)
                break;
            cv::Point center = findAndRemoveMaximum(hough, &value, r, cellStep);
            if (maxValue == -1)
                maxValue = value;

            if (value < maxValue || value < 1)
                break;

            // point may be a center of a circle
            addFoundCenter(list, center.x, center.y, r, value);
        }
    }
}
