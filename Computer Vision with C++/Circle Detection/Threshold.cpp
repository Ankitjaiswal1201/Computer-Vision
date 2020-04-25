#include <opencv2/imgproc/imgproc.hpp>

#include "Threshold.h"

Threshold::Threshold()
{}

Threshold::~Threshold()
{}

////////////////////////////////////////////////////////////////////////////////////
// compute threshold image using the OpenCV function - only for reference
////////////////////////////////////////////////////////////////////////////////////
void Threshold::cv(const cv::Mat &input, cv::Mat &output, uchar threshold)
{
    cv::threshold(input, output, 128, 255, cv::THRESH_BINARY);
}

////////////////////////////////////////////////////////////////////////////////////
// compute threshold image by looping over the elements
////////////////////////////////////////////////////////////////////////////////////
void Threshold::loop(const cv::Mat &input, cv::Mat &output, uchar threshold)
{
    int rows = input.rows;
    int cols = input.cols;

    output.release();
    output.create(rows, cols, CV_8U);

    for (int r = 0; r < rows; ++r)
    {
        for (int c = 0; c < cols; ++c)
        {
            if (input.at<uchar>(r, c) >= threshold)
                output.at<uchar>(r, c) = 255;
            else
                output.at<uchar>(r, c) = 0;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
// compute threshold image by looping over the elements (pointer access)
///////////////////////////////////////////////////////////////////////////////
void Threshold::loop_ptr(const cv::Mat &input, cv::Mat &output, uchar threshold)
{
    int rows = input.rows;
    int cols = input.cols;

    output.release();
    output.create(rows, cols, CV_8U);

    if (input.isContinuous())
    {

    }

    for (int r = 0; r < rows; ++r)
    {
        const uchar *pInput = input.ptr<uchar>(r);
        uchar *pOutput = output.ptr<uchar>(r);

        for (int c = 0; c < cols; ++c)
        {

            if (pInput[c] >= threshold)
                pOutput[c] = 255;
            else
                pOutput[c] = 0;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
// compute threshold image by looping over the elements (pointer access)
///////////////////////////////////////////////////////////////////////////////
void Threshold::loop_ptr2(const cv::Mat &input, cv::Mat &output, uchar threshold)
{
    int rows = input.rows;
    int cols = input.cols;

    output.release();
    output.create(rows, cols, CV_8U);

    if (input.isContinuous())
    {
        cols = rows*cols;
        rows = 1;
    }

    for (int r = 0; r < rows; ++r)
    {
        const uchar *pInput = input.ptr<uchar>(r);
        uchar *pOutput = output.ptr<uchar>(r);

        for (int c = 0; c < cols; ++c)
        {

            if (*pInput >= threshold)
                *pOutput = 255;
            else
                *pOutput = 0;

            ++pInput;
            ++pOutput;
        }
    }
}
