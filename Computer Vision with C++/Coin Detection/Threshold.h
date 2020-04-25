#ifndef THRESHOLD_H
#define THRESHOLD_H

#include <opencv2/core/core.hpp>

class Threshold
{
public:
    Threshold();

    ~Threshold();

    void cv(const cv::Mat &input, cv::Mat &output, uchar threshold);
    void loop(const cv::Mat &input, cv::Mat &output, uchar threshold);
    void loop_ptr(const cv::Mat &input, cv::Mat &output, uchar threshold);
    void loop_ptr2(const cv::Mat &input, cv::Mat &output, uchar threshold);
private:
};

#endif /* THRESHOLD_H */
