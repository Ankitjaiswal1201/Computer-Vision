#ifndef POINTOPS_H
#define POINTOPS_H

#include <opencv2/core/core.hpp>

class PointOperations
{
public:
    PointOperations();

    ~PointOperations();

    void adjustContrast(cv::Mat &input, cv::Mat &output, float alpha, uchar center=127);
    void adjustBrightness(cv::Mat &input, cv::Mat &output, int alpha);
    void invert(cv::Mat &input, cv::Mat &output);
    void quantize(cv::Mat &input, cv::Mat &output, uchar n);

private:
};

#endif /* POINTOPS_H */
