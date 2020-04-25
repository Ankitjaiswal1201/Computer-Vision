#include <iostream>

#include <opencv2/imgproc/imgproc.hpp>

#include "PointOperations.h"


PointOperations::PointOperations()
{}

PointOperations::~PointOperations()
{}
////////////////////////////////////////////////////////////////////////////////////
// adjust the contrast of an image by alpha around center
////////////////////////////////////////////////////////////////////////////////////
void PointOperations::adjustContrast(cv::Mat &input, cv::Mat &output, float alpha, uchar center)
{
    int rows = input.rows;
    int cols = input.cols;

    output.release();
    output.create(rows, cols, CV_8U);

    if (input.isContinuous())
    {
        cols = rows * cols;
        rows = 1;
    }

    for (int r = 0; r < rows; ++r)
    {
        const uchar *pInput = input.ptr<uchar>(r);
        uchar *pOutput = output.ptr<uchar>(r);

        for (int c = 0; c < cols; ++c)
        {
            // calculate new brightness level for the pixel
            float adjusted = alpha*(*pInput-center)+center;

            // limit the values (saturation point and zero point)
            if (adjusted > 255)
                adjusted = 255;
            else if (adjusted < 0)
                adjusted = 0;

            // set the new brightness in output image
            *pOutput = (uchar) adjusted;

            ++pInput;
            ++pOutput;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////
// adjust the brightness of an image by alpha
////////////////////////////////////////////////////////////////////////////////////
void PointOperations::adjustBrightness(cv::Mat &input, cv::Mat &output, int alpha)
{
    int rows = input.rows;
    int cols = input.cols;

    output.release();
    output.create(rows, cols, CV_8U);

    if (input.isContinuous())
    {
        cols = rows * cols;
        rows = 1;
    }

    for (int r = 0; r < rows; ++r)
    {
        const uchar *pInput = input.ptr<uchar>(r);
        uchar *pOutput = output.ptr<uchar>(r);

        for (int c = 0; c < cols; ++c)
        {
            // calculate new brightness level for the pixel
            int adjusted = *pInput+alpha;

            // limit the values (saturation point and zero point)
            if (adjusted > 255)
                adjusted = 255;
            else if (adjusted < 0)
                adjusted = 0;

            // set the new brightness in output image; note the implicit cast to uchar
            *pOutput = adjusted;

            ++pInput;
            ++pOutput;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////
// inversion of an image
////////////////////////////////////////////////////////////////////////////////////
void PointOperations::invert(cv::Mat &input, cv::Mat &output)
{
    int rows = input.rows;
    int cols = input.cols;

    output.release();
    output.create(rows, cols, CV_8U);

    if (input.isContinuous())
    {
        cols = rows * cols;
        rows = 1;
    }

    for (int r = 0; r < rows; ++r)
    {
        const uchar *pInput = input.ptr<uchar>(r);
        uchar *pOutput = output.ptr<uchar>(r);

        for (int c = 0; c < cols; ++c)
        {
            // invert input and set the new brightness in output image
            *pOutput = 255 - *pInput;

            ++pInput;
            ++pOutput;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////
// quantization of an image with n bits
////////////////////////////////////////////////////////////////////////////////////
void PointOperations::quantize(cv::Mat &input, cv::Mat &output, uchar n)
{
    int rows = input.rows;
    int cols = input.cols;

    output.release();
    output.create(rows, cols, CV_8U);

    if (input.isContinuous())
    {
        cols = rows * cols;
        rows = 1;
    }

    for (int r = 0; r < rows; ++r)
    {
        const uchar *pInput = input.ptr<uchar>(r);
        uchar *pOutput = output.ptr<uchar>(r);

        for (int c = 0; c < cols; ++c)
        {
            // calculate new brightness level for the pixel
            // using shift operations
            uchar shift = (8 - n);
            // integer division and multiplication
            uchar adjusted = (*pInput >> shift) << shift;
            // obtain central position by adding half of the interval size
            adjusted += (128 >> n);

            // set the new brightness in output image
            *pOutput = adjusted;

            ++pInput;
            ++pOutput;
        }
    }
}
