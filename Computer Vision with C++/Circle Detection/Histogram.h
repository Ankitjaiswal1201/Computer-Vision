#ifndef HISTOGRAM_H
#define HISTOGRAM_H

#include <opencv2/core/core.hpp>

class Histogram
{
public:
    Histogram();

    ~Histogram();

    void calcHist_cv(const cv::Mat &input, cv::Mat &hist);
    void calcHist(const cv::Mat &input, cv::Mat &hist);
    void calcStats(const cv::Mat &hist, uchar &min, uchar &max, uchar &mean);

    void show(const cv::string &winname, const cv::Mat &hist);

private:
    int histSize; // number of bins
};

#endif /* HISTOGRAM_H */
