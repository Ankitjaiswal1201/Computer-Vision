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
        }}
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
                }}
            normFactor_input = sqrt(normFactor_input);

            // normalize the result
            float normFactor = normFactor_input * normFactor_templ;
            result /= (normFactor);

            *pOutput = result;

            ++pOutput;
        }}}
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
    float phiStepRad = phiStep * M_PI / 180.0f;

    output.release();

    // create a 32bit signed integer image initialized with zeros
    output = cv::Mat::zeros(2 * dMax, phiMax, CV_32S);

    // Hough Transformation
    for (int r = 0; r < rows; ++r)
    {
        const float *pInput = input.ptr<float>(rows - r - 1);

        for (int c = 0; c < cols; ++c)
        {
            if (*pInput != 0)
            {
                for (int phi = 0; phi < phiMax; ++phi)
                {
                    // origin is the lower left corner!
                    // compute phi and d as floating point values
                    float phiRad = (float) phi * phiStepRad;
                    float dFloat = ((float) c * cos(phiRad) + (float) r * sin(phiRad));

                    // round and convert to integer
                    int dInt = (int) (dFloat > 0.0f) ? (dFloat + 0.5f) : (dFloat - 0.5f);

                    // increment in output image
                    ++output.at<int>(dMax - dInt, phi);
                }          }

            ++pInput;
        } }}
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
            }}}
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
    float phiStepRad = phiStep * M_PI / 180.0f;

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
            p1 = cv::Point(d, rows - 1);
            p2 = cv::Point(d, 0);
        }
        else if (phi == 90.0f)
        {
            // horizontal line
            p1 = cv::Point(0, rows - d - 1);
            p2 = cv::Point(cols - 1, rows - d - 1);
        }
        else if (phi < 90.0f)
        {
            // monotonically decreasing
            int x = d / cos(phiRad);
            int y = d / sin(phiRad);

            p1 = cv::Point(0, rows - y - 1);
            p2 = cv::Point(x, rows - 1);
        }
        else
        {
            // monotonically increasing
            int y = d / sin(phiRad);
            int x = -1.0f * tan(phiRad) * (rows - y);

            p1 = cv::Point(0, rows - y - 1);
            p2 = cv::Point(x, 0);
        }

        // draw a red line
        cv::line(output, p1, p2, cv::Scalar(0.0f, 0.0f, 1.0f), 1);
    }
}
