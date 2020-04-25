#include <iostream>
#include <math.h>

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "Filter.h"

////////////////////////////////////////////////////////////////////////////////////
// constructor. Initialize the kernels
////////////////////////////////////////////////////////////////////////////////////
Filter::Filter()
{
    // Initialize Binomial kernels
    // 3x3
    signed char kernelB3[3 * 3] =  {1, 2, 1,
                             2, 4, 2,
                             1, 2, 1};

    Binomial3 = cv::Mat(3, 3, CV_8S, kernelB3).clone();

    // 5x5
    signed char kernelB5[5 * 5] =  {1,  4,  6,  4, 1,
                             4, 16, 24, 16, 4,
                             6, 24, 36, 24, 6,
                             4, 16, 24, 16, 4,
                             1,  4,  6,  4, 1};

    Binomial5 = cv::Mat(5, 5, CV_8S, kernelB5).clone();

    // 3x1 and 1x3
    signed char kernelB3x1[3 * 1] =  {1,  2, 1};

    Binomial3x1 = cv::Mat(3, 1, CV_8S, kernelB3x1).clone();  
    Binomial1x3 = cv::Mat(1, 3, CV_8S, kernelB3x1).clone();  

    // 5x1 and 1x5
    signed char kernelB5x1[5 * 1] =  {1,  4,  6,  4, 1};

    Binomial5x1 = cv::Mat(5, 1, CV_8S, kernelB5x1).clone();  
    Binomial1x5 = cv::Mat(1, 5, CV_8S, kernelB5x1).clone();  

    // initialize Sobel kernels
    // 3x3 in X direction
    signed char kernelS1[3 * 3] =  {-1, 0, 1,
                             -2, 0, 2,
                             -1, 0, 1};

    Sobel3_X = cv::Mat(3, 3, CV_8S, kernelS1).clone();

    // 3x3 in Y direction
    signed char kernelS2[3 * 3] =  {-1, -2, -1,
                              0,  0,  0,
                              1,  2,  1};

    Sobel3_Y = cv::Mat(3, 3, CV_8S, kernelS2).clone();

    // 5x5 in X direction
    signed char kernelS3[5 * 5] =  { -2, -1, 0, 1,  2,
                              -8, -4, 0, 4,  8,
                             -12, -6, 0, 6, 12,
                              -8, -4, 0, 4,  8,
                              -2, -1, 0, 1,  2};

    Sobel5_X = cv::Mat(5, 5, CV_8S, kernelS3).clone();

    // 5x5 in Y direction
    signed char kernelS4[5 * 5] =  {-2, -8, -12, -8, -2,
                             -1, -4,  -6, -4, -1,
                              0,  0,   0,  0,  0,
                              1,  4,   6,  4,  1,
                              2,  8,  12,  8,  2};

    Sobel5_Y = cv::Mat(5, 5, CV_8S, kernelS4).clone();
}

Filter::~Filter(){}

////////////////////////////////////////////////////////////////////////////////////
// convolve the image with the kernel using the OpenCV function - only for reference
////////////////////////////////////////////////////////////////////////////////////
void Filter::convolve_cv(const cv::Mat &input, cv::Mat &output, const cv::Mat &kernel)
{

    if (input.empty() || kernel.empty())
    {  
        std::cout << "One ore more inputs are empty!" << std::endl;
        return;
    }

    int normFactor = 0;
    
    int rows = kernel.rows;
    int cols = kernel.cols;
    
    if (kernel.isContinuous())
    {
        cols = rows * cols;
        rows = 1;
    }
    
    // calculate the normalisation factor from the filter kernel
    for (int r = 0; r < rows; ++r)
    {
        const signed char *pKernel = kernel.ptr<signed char>(r);
        
        for (int c = 0; c < cols; ++c)
        {
            normFactor += abs(*pKernel);
            ++pKernel;
        }
    }

    cv::Mat floatInput, floatKernel;
    input.convertTo(floatInput, CV_32F);
    kernel.convertTo(floatKernel, CV_32F);

    cv::filter2D(floatInput, output, floatInput.depth(), floatKernel, cv::Point(-1,-1), 0, cv::BORDER_REPLICATE);

    output /= normFactor;
}

///////////////////////////////////////////////////////////////////////////////
// convolve the image with the square-sized 3x3 kernel using pointer access
///////////////////////////////////////////////////////////////////////////////
void Filter::convolve_3x3(const cv::Mat &input, cv::Mat &output, const cv::Mat &kernel)
{
    if (input.empty() || kernel.empty())
    {
        std::cout << "One ore more inputs are empty!" << std::endl;
        return;
    }

    int rows = input.rows;
    int cols = input.cols;

    output.release();
    // create a float image initialized with zeros
    output = cv::Mat::zeros(rows, cols, CV_32F);

    int kRows = kernel.rows;
    int kCols = kernel.cols;
    
    if (kernel.isContinuous())
    {
        kCols = kRows * kCols;
        kRows = 1;
    }

    // calculate the normalisation factor from the filter kernel
    int normFactor = 0;

    for (int r = 0; r < kRows; ++r)
    {
        const signed char *pKernel = kernel.ptr<signed char>(r);
        
        for (int c = 0; c < kCols; ++c)
        {
            normFactor += abs(*pKernel);
            ++pKernel;
        }
    }

    // perform a 3x3 convolution with cropped edges
    for (int r = 0; r < (rows - 2); ++r)
    {
        float *pOutput = output.ptr<float>(r + 1) + 1;

        for (int c = 0; c < (cols - 2); ++c)
        {
            const float *pInputAbove = input.ptr<float>(r)     + c;
            const float *pInput      = input.ptr<float>(r + 1) + c;
            const float *pInputBelow = input.ptr<float>(r + 2) + c;

            const signed char *pKernel1stLine = kernel.ptr<signed char>(0);
            const signed char *pKernel2ndLine = kernel.ptr<signed char>(1);
            const signed char *pKernel3rdLine = kernel.ptr<signed char>(2);

            float result = 0.0f;

            for (int k = 0; k < 3; ++k)
            {
                result += (*pInputAbove) * (*pKernel1stLine) +
                          (*pInput)      * (*pKernel2ndLine) +
                          (*pInputBelow) * (*pKernel3rdLine);
                
                ++pInputAbove;
                ++pInput;
                ++pInputBelow;  
                ++pKernel1stLine;
                ++pKernel2ndLine;
                ++pKernel3rdLine;      
            }

            result /= normFactor;

            *pOutput = result;

            ++pOutput;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
// convolve the image with any filter kernel using pointer access
///////////////////////////////////////////////////////////////////////////////
void Filter::convolve_generic(const cv::Mat &input, cv::Mat &output, const cv::Mat &kernel)
{
    if (input.empty() || kernel.empty())
    {
        std::cout << "One ore more inputs are empty!" << std::endl;
        return;
    }
    
    int rows = input.rows;
    int cols = input.cols;
    
    output.release();
    // create a float image initialized with zeros
    output = cv::Mat::zeros(rows, cols, CV_32F); 

    int kRows = kernel.rows;
    int kCols = kernel.cols;
    
    if (kernel.isContinuous())
    {
        kCols = kRows * kCols;
        kRows = 1;
    }

    // calculate the normalisation factor from the filter kernel
    int normFactor = 0;

    for (int r = 0; r < kRows; ++r)
    {
        const signed char *pKernel = kernel.ptr<signed char>(r);
        
        for (int c = 0; c < kCols; ++c)
        {
            normFactor += abs(*pKernel);
            ++pKernel;
        }
    }

    // perform a generic convolution with cropped edges
    kRows = kernel.rows;
    kCols = kernel.cols;

    int kHotspotX = kCols / 2;
    int kHotspotY = kRows / 2;

    // perform convolution
    for (int r = 0; r < (rows - kRows + 1); ++r)
    {
        float *pOutput = output.ptr<float>(r + kHotspotY) + kHotspotX;

        for (int c = 0; c < (cols - kCols + 1); ++c)
        {
            float result = 0.0f;

            for (int kr = 0; kr < kRows; ++kr)
            {
                const float *pInput = input.ptr<float>(r + kr) + c;
                const signed char *pKernel = kernel.ptr<signed char>(kr);

                for (int kc = 0; kc < kCols; ++kc)
                {
                    result += ((*pInput) * (*pKernel));

                    ++pKernel;
                    ++pInput;
                }
            }

            result /= normFactor;

            *pOutput = result;

            ++pOutput;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
// extrapolate the image's borders and perform convolution
///////////////////////////////////////////////////////////////////////////////
void Filter::convolve_extrapolate(const cv::Mat &input, cv::Mat &output, const cv::Mat &kernel)
{
    if (input.empty() || kernel.empty())
    {
        std::cout << "One ore more inputs are empty!" << std::endl;
        return;
    }

    int border = kernel.rows/2;
    int rows = input.rows;
    int cols = input.cols;
    int rOffset = rows + border;
    int cOffset = cols + border;

    // create new Mat to hold the extrapolated image
    cv::Mat extrapolated(rows + 2 * border, cols + 2 * border, input.type());

    // define a ROI and copy input image to it
    cv::Rect ROI(border, border, cols, rows);
    input.copyTo(extrapolated(ROI));

    // do the extrapolation
    // begin with the part on the left and right
    for (int r = 0; r < rows; ++r)
    {
        const float *pInput = input.ptr<float>(r);
        float *pExtrapolated = extrapolated.ptr<float>(border + r);

        for (int b = 0; b < border; ++b)
        {
            *pExtrapolated = (*pInput);
            *(pExtrapolated + cOffset) = *(pInput + cols - 1);

            ++pExtrapolated;
        }
    }

    // continue with upper and lower part of the image
    for (int b = 0; b < border; ++b)
    {
        float *pExtrapolatedAbove = extrapolated.ptr<float>(b);
        float *pExtrapolatedAboveExisting = extrapolated.ptr<float>(border);

        float *pExtrapolatedBelow = extrapolated.ptr<float>(b + rOffset);
        float *pExtrapolatedBelowExisting = extrapolated.ptr<float>(rOffset - 1);

        for (int c = 0; c < (cols + 2 * border); ++c)
        {
            *pExtrapolatedAbove = *pExtrapolatedAboveExisting;
            *pExtrapolatedBelow = *pExtrapolatedBelowExisting;

            ++pExtrapolatedAbove;
            ++pExtrapolatedAboveExisting;
            ++pExtrapolatedBelow;
            ++pExtrapolatedBelowExisting;
        }
    }

    // do the convolution with the extrapolated image
    cv::Mat extrapolatedOutput;
    convolve_generic(extrapolated, extrapolatedOutput, kernel);

    // discard the border pixels and copy the result to the output image
    extrapolatedOutput(ROI).copyTo(output);
}

///////////////////////////////////////////////////////////////////////////////
// calculate the abs() of the x-Sobel and y-Sobel image
///////////////////////////////////////////////////////////////////////////////
void Filter::getAbsOfSobel(const cv::Mat &input_1, const cv::Mat &input_2, cv::Mat &output)
{
    if (input_1.empty() || input_2.empty())
    {
        std::cout << "One ore more inputs are empty!" << std::endl;
        return;
    }

    int rows = input_1.rows;
    int cols = input_1.cols;

    output.release();
    // create a float image initialized with zeros
    output = cv::Mat::zeros(rows, cols, CV_32F);
    
    // calculate the abs() of the x-Sobel and y-Sobel results
    for (int r = 0; r < rows; ++r)
    {
        const float *pInput_1 = input_1.ptr<float>(r);
        const float *pInput_2 = input_2.ptr<float>(r);
        float *pOutput = output.ptr<float>(r);
        
        for (int c = 0; c < cols; ++c)
        {
            *pOutput = sqrt( pow(*pInput_1, 2) + pow(*pInput_2, 2) );

            ++pInput_1;
            ++pInput_2;
            ++pOutput;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
// compute a binomial kernel
///////////////////////////////////////////////////////////////////////////////
cv::Mat Filter::calcBinomial(uchar size)
{
    cv::Mat series(size, 1, CV_8S);
    cv::Mat kernel(size, size, CV_8S);

    uchar order = size - 1;

    // calculate a series of binomial coefficients
    signed char *pSeries = series.ptr<signed char>(0);
    for (int x = 0; x < size; ++x)
    {
        *pSeries = (signed char) calcBinomialCoefficient(order, x);
        ++pSeries;
    }

    // form the kernel as outer product
    for (int x = 0; x < size; ++x)
    {
        signed char *pKernel = kernel.ptr<signed char>(x);
        const signed char *pSeriesX = series.ptr<signed char>(0) + x;
        const signed char *pSeriesY = series.ptr<signed char>(0);

        for (int y = 0; y < size; ++y)
        {
            *pKernel = (*pSeriesX * *pSeriesY);
            ++pKernel;
            ++pSeriesY;
        }
    }

    return kernel;
}

///////////////////////////////////////////////////////////////////////////////
// compute a Sobel kernel
///////////////////////////////////////////////////////////////////////////////
cv::Mat Filter::calcSobel(uchar size, bool transpose)
{
    signed char mean = (signed char) size / 2;

    cv::Mat binomial(size, 1, CV_8S);
    cv::Mat grad(size, 1, CV_8S);
    cv::Mat kernel(size, size, CV_8S);
    cv::Mat output;

    // calculate a series of binomial coefficients
    signed char *pBinomial = binomial.ptr<signed char>(0);
    for (int x = 0; x < size; ++x)
    {
        *pBinomial = (signed char) calcBinomialCoefficient(size - 1, x);
        ++pBinomial;
    }

    // Get gradient
    signed char *pGrad = grad.ptr<signed char>(0);
    for (int x = 0; x < size; ++x)
    {
        *pGrad = (signed char) -mean + x;
        ++pGrad;
    }

    // form the kernel as (binomial * grad^T) or (grad * binomial^T) respectively
    signed char *pX, *pY;
    for (int x = 0; x < size; ++x)
    {
        signed char *pKernel = kernel.ptr<signed char>(x);

        if (transpose)
        {
            pX = grad.ptr<signed char>(0) + x;
            pY = binomial.ptr<signed char>(0);
        }
        else
        {
            pX = binomial.ptr<signed char>(0) + x;
            pY = grad.ptr<signed char>(0);
        }

        for (int y = 0; y < size; ++y)
        {
            *pKernel = (*pX * *pY);
            ++pKernel;
            ++pY;
        }
    }
    
    return kernel;
}

///////////////////////////////////////////////////////////////////////////////
// return the Binomial Coefficient (n over k)
///////////////////////////////////////////////////////////////////////////////
int Filter::calcBinomialCoefficient(int n, int k)
{
    if (( k == 0 ) || (n == k))
    {
        return 1;
    }
    else
    {
        // recursion
        int tempN = calcBinomialCoefficient(n - 1, k);
        int tempK = calcBinomialCoefficient(n - 1, k - 1);
        return tempN + tempK;
    }
}

///////////////////////////////////////////////////////////////////////////////
// return the Binomial kernel
///////////////////////////////////////////////////////////////////////////////
cv::Mat Filter::getBinomial(uchar size)
{
    if (size == 3)
        return Binomial3;
    else if (size == 5)
        return Binomial5;
    else
        return cv::Mat();
}

///////////////////////////////////////////////////////////////////////////////
// return the separated Binomial kernels
///////////////////////////////////////////////////////////////////////////////
cv::Mat Filter::getBinomialSeparated(uchar size, bool transpose)
{
    if (size == 3)
    {
        if (transpose)
            return Binomial3x1;
        else
            return Binomial1x3;
    }
    else if (size == 5)
    {
        if (transpose)
            return Binomial5x1;
        else
            return Binomial1x5;
    }
    else
    {
        return cv::Mat();
    }
}


///////////////////////////////////////////////////////////////////////////////
// return the Sobel kernel in X direction
///////////////////////////////////////////////////////////////////////////////
cv::Mat Filter::getSobelX(uchar size)
{
    if (size == 3)
        return Sobel3_X;
    else if (size == 5)
        return Sobel5_X;
    else
        return cv::Mat();
}

///////////////////////////////////////////////////////////////////////////////
// return the Sobel kernel in Y direction
///////////////////////////////////////////////////////////////////////////////
cv::Mat Filter::getSobelY(uchar size)
{
    if (size == 3)
        return Sobel3_Y;
    else if (size == 5)
        return Sobel5_Y;
    else
        return cv::Mat();
}

///////////////////////////////////////////////////////////////////////////////
// scale a Sobel image for better displaying
///////////////////////////////////////////////////////////////////////////////
void Filter::scaleSobelImage(const cv::Mat &input, cv::Mat &output)
{
    // find max value
    double min, max;
    cv::minMaxLoc(input, &min, &max);

    // scale the image
    input.convertTo(output, CV_32F, (0.5f / max), 0.5f);
}
