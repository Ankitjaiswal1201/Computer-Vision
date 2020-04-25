#ifndef SEGMENTATION_H
#define SEGMENTATION_H

#include <opencv2/core/core.hpp>

struct CircleItem
{
    // center
    int x;
    int y;

    // radius
    int r;

    // you can add other variables to the item here:
    int v;
};

class Segmentation
{
public:
    Segmentation();
    ~Segmentation();

    // Template Matching
    void cutAndSave(const cv::Mat &input, cv::Point origin, cv::Size size, const cv::string &filename);
    void crossCorrelate(const cv::Mat &input, const cv::Mat &templ, cv::Mat &output);
    cv::Point findMaximum(const cv::Mat &input);
    void drawRect(const cv::Mat &input, cv::Point origin, cv::Size size, cv::Mat &output);

    // Hough Transformation
    void houghTransform(const cv::Mat &input, float phiStep, cv::Mat &output);
    void scaleHoughImage(const cv::Mat &input, cv::Mat &output);
    cv::Mat findMaxima(const cv::Mat &input, int n);
    void drawLines(const cv::Mat &input, cv::Mat lines, float phiStep, cv::Mat &output);

    // Hough Circle Transformation
    void houghCircle(const cv::Mat &input, cv::Mat &output, const int radius, const float cellStep, const float phiStep);
    cv::Point findAndRemoveMaximum(cv::Mat &image, int *value, const int radius, const float cellStep);
    void findCircles(const cv::Mat &input, std::vector<CircleItem> *list, const int radiusMin,
                     const int radiusMax, const float cellStep,
                     const float phiStep);

  private:
      void addFoundCenter(std::vector<CircleItem> *list, const int x, const int y, const int r, const int value);
};

#endif /* SEGMENTATION_H */
