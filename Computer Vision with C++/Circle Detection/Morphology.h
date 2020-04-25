#ifndef MORPHOLOGICLAL_H
#define MORPHOLOGICLAL_H

#include <opencv2/core/core.hpp>

class Morphology
{
public:
    Morphology();
    ~Morphology();

    void dilate(const cv::Mat &input, cv::Mat &output, const cv::Mat &kernel);
    void erode(const cv::Mat &input, cv::Mat &output, const cv::Mat &kernel);
    void subtract(const cv::Mat &input, cv::Mat &output, const cv::Mat &subtract);

    cv::Mat getKernelPlus();
    cv::Mat getKernelLine();
    cv::Mat getKernelFull(int size);

private:
    cv::Mat kernel3x3Plus;
    cv::Mat kernel1x5Line;
    cv::Mat kernel3x3Full, kernel4x4Full;
};

#endif /* MORPHOLOGICLAL_H */
