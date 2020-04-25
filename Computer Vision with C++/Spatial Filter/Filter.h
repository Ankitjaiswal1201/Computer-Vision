#ifndef FILTER_H
#define FILTER_H

#include <opencv2/core/core.hpp>

class Filter
{
public:
    Filter();

    ~Filter();

    void convolve_cv(const cv::Mat &input, cv::Mat &output, const cv::Mat &kernel);
    void convolve_3x3(const cv::Mat &input, cv::Mat &output, const cv::Mat &kernel);
    void convolve_generic(const cv::Mat &input, cv::Mat &output, const cv::Mat &kernel);
    void convolve_extrapolate(const cv::Mat &input, cv::Mat &output, const cv::Mat &kernel);
    void getAbsOfSobel(const cv::Mat &input_1, const cv::Mat &input_2, cv::Mat &output);
    void scaleSobelImage(const cv::Mat &input, cv::Mat &output);
    
    cv::Mat calcBinomial(uchar size);
    cv::Mat calcSobel(uchar size, bool transpose);

    cv::Mat getBinomial(uchar size);
    cv::Mat getBinomialSeparated(uchar size, bool transpose);
    cv::Mat getSobelX(uchar size);
    cv::Mat getSobelY(uchar size);

private:
    cv::Mat Binomial3, Binomial5;
    cv::Mat Binomial5x1, Binomial1x5;
    cv::Mat Binomial3x1, Binomial1x3;
    cv::Mat Sobel3_X, Sobel3_Y;
    cv::Mat Sobel5_X, Sobel5_Y;

    int calcBinomialCoefficient(int n, int k);
};

#endif /* FILTER_H */
