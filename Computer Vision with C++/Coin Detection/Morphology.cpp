#include <iostream>
#include <math.h>

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "Morphology.h"

////////////////////////////////////////////////////////////////////////////////////
// constructor. initialize the kernels
////////////////////////////////////////////////////////////////////////////////////
Morphology::Morphology()
{
    // Initialize kernels
    uchar kernel3x3_plus[3 * 3] = {
          0, 255,   0,
        255, 255, 255,
          0, 255,   0 };
    kernel3x3Plus = cv::Mat(3, 3, CV_8U, kernel3x3_plus).clone();

    uchar kernel1x5_line[5] = { 255, 255, 255, 255, 255 };
    kernel1x5Line = cv::Mat(5, 1, CV_8U, kernel1x5_line).clone();

    uchar kernel3x3_full[3 * 3] = {
        255, 255, 255,
        255, 255, 255,
        255, 255, 255 };
    kernel3x3Full = cv::Mat(3, 3, CV_8U, kernel3x3_full).clone();

    uchar kernel4x4_full[4 * 4] = {
        255, 255, 255, 255,
        255, 255, 255, 255,
        255, 255, 255, 255,
        255, 255, 255, 255 };
    kernel4x4Full = cv::Mat(4, 4, CV_8U, kernel4x4_full).clone();
}

Morphology::~Morphology(){}

cv::Mat Morphology::getKernelPlus()
{
    return kernel3x3Plus;
}

cv::Mat Morphology::getKernelLine()
{
    return kernel1x5Line;
}

cv::Mat Morphology::getKernelFull(int size)
{
    if (size == 4)
        return kernel4x4Full;
    return kernel3x3Full;
}

void Morphology::dilate(const cv::Mat &input, cv::Mat &output, const cv::Mat &kernel)
{
    if (input.empty() || kernel.empty())
    {
        std::cout << "One ore more inputs are empty!" << std::endl;
        return;
    }

    int rows = input.rows;
    int cols = input.cols;

    int kRows = kernel.rows;
    int kCols = kernel.cols;

    int refPointX = (kCols - 1) / 2; 
    int refPointY = (kRows - 1) / 2;

    output.release();
    output = cv::Mat::zeros(rows, cols, CV_8U);

    for (int r = 0; r < rows - kRows; ++r)
    {
        uchar *pOutput = output.ptr<uchar>(r + refPointY) + refPointX;

        for (int c = 0; c < cols - kCols; ++c)
        {
            bool found = false;

            for (int kr = 0; kr < kRows; ++kr)
            {
                const uchar *pInput = input.ptr<uchar>(r + kr) + c;
                const uchar *pKernel = kernel.ptr<uchar>(kr);

                for (int kc = 0; kc < kCols; ++kc)
                {
                    if (*pKernel > 0 && *pInput > 0)
                    {
                        found = true;
                        break;
                    }
                    ++pKernel;
                    ++pInput;
                }
                if (found)
                    break;
            }
            *pOutput = found ? 255 : 0;
            pOutput++;
        }
    }
}

void Morphology::erode(const cv::Mat &input, cv::Mat &output, const cv::Mat &kernel)
{
    if (input.empty() || kernel.empty())
    {
        std::cout << "One ore more inputs are empty!" << std::endl;
        return;
    }

    int rows = input.rows;
    int cols = input.cols;

    int kRows = kernel.rows;
    int kCols = kernel.cols;

    int refPointX = (kCols - 1) / 2; 
    int refPointY = (kRows - 1) / 2;

    output.release();
    output = cv::Mat::zeros(rows, cols, CV_8U);

    for (int r = 0; r < rows - kRows; ++r)
    {
        uchar *pOutput = output.ptr<uchar>(r + refPointY) + refPointX;

        for (int c = 0; c < cols - kCols; ++c)
        {
            bool found = false;

            for (int kr = 0; kr < kRows; ++kr)
            {
                const uchar *pInput = input.ptr<uchar>(r + kr) + c;
                const uchar *pKernel = kernel.ptr<uchar>(kr);

                for (int kc = 0; kc < kCols; ++kc)
                {
                    if (*pKernel > 0 && *pInput == 0)
                    {
                        found = true;
                        break;
                    }
                    ++pKernel;
                    ++pInput;
                }
                if (found)
                    break;
            }
            *pOutput = found ? 0 : 255;
            pOutput++;
        }
    }
}

void Morphology::subtract(const cv::Mat &input, cv::Mat &output, const cv::Mat &subtract)
{
    if (input.empty() || subtract.empty())
    {
        std::cout << "One ore more inputs are empty!" << std::endl;
        return;
    }

    int rows = input.rows;
    int cols = input.cols;

    if (subtract.rows != rows || subtract.cols != cols)
    {
        std::cout << "subtract image does not fit input image!" << std::endl;
        return;
    }

    output.release();
    output = cv::Mat::zeros(rows, cols, CV_8U);

    if (input.isContinuous() && output.isContinuous() && subtract.isContinuous())
    {
        cols = rows * cols;
        rows = 1;
    }

    for (int r = 0; r < rows; ++r)
    {
        uchar *pOutput = output.ptr<uchar>(r);
        const uchar *pInput = input.ptr<uchar>(r);
        const uchar *pSubtract = subtract.ptr<uchar>(r);

        for (int c = 0; c < cols; ++c)
        {
            int value = *pInput++ - *pSubtract++;
            *pOutput++ = value > 0 ? value : 0;
        }
    }
}
