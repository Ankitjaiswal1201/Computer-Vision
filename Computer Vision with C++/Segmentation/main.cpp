#include <iostream>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "Threshold.h"
#include "Histogram.h"
#include "PointOperations.h"
#include "Filter.h"
#include "Segmentation.h"
#include "Timer.h"
#include "imshow_multiple.h"

int main(int argc, char *argv[])
{
    //read images
    cv::Mat img = cv::imread(INPUTIMAGEDIR "/lena.tiff");
    cv::Mat imgTemplate = cv::imread(INPUTIMAGEDIR "/template_1.tiff");
    cv::Mat imgLines = cv::imread(INPUTIMAGEDIR "/lines.tiff");

    //convert to grayscale
    cv::cvtColor(img, img, CV_BGR2GRAY);
    cv::cvtColor(imgTemplate, imgTemplate, CV_BGR2GRAY);
    cv::cvtColor(imgLines, imgLines, CV_BGR2GRAY);

    // convert to a float image
    img.convertTo(img, CV_32F, 1.0/255.0);
    imgTemplate.convertTo(imgTemplate, CV_32F, 1.0/255.0);
    imgLines.convertTo(imgLines, CV_32F, 1.0/255.0);

    //declare output variables
    cv::Mat imgCorrelation;
    cv::Mat imgMatched;
    cv::Mat houghTransform;
    cv::Mat imgHough;
    cv::Mat imgDetectedLines;

    //create class instances
    Threshold *threshold = new Threshold();
    Histogram *histogram = new Histogram();
    PointOperations *pointOperations = new PointOperations();
    Filter *filter = new Filter();
    Segmentation *segmentation = new Segmentation();

    // begin processing ///////////////////////////////////////////////////////////

    // create a template image
    //segmentation->cutAndSave(img, cv::Point(316, 252), cv::Size(30, 30), "../data/template.tiff");

    // Template Matching
    // compute and show normalized cross correlation
    segmentation->crossCorrelate(img, imgTemplate, imgCorrelation);
    cv::imshow("Cross Correlation", imgCorrelation);
    // find the best match between image and template
    cv::Point matchPoint = segmentation->findMaximum(imgCorrelation);
    std::cout << "Found best match at " << matchPoint;
    // draw a rectangle at the match position
    segmentation->drawRect(img, matchPoint, imgTemplate.size(), imgMatched);
    cv::imshow("Matched Template", imgMatched);

    // Hough Transformation
    cv::imshow("Stright Lines", imgLines);
    // compute Hough Transformation
    // define step size of the angle in degrees
    float phiStep = 1.0f;
    segmentation->houghTransform(imgLines, phiStep, houghTransform);
    // find the lines
    cv::Mat lines = segmentation->findMaxima(houghTransform, 4);
    // scale and display the Hough transformation
    segmentation->scaleHoughImage(houghTransform, imgHough);
    cv::imshow("Hough Transformation", imgHough);
    // draw detected lines on the image and show
    segmentation->drawLines(imgLines, lines, phiStep, imgDetectedLines);
    cv::imshow("Detected Lines", imgDetectedLines);

    // end processing /////////////////////////////////////////////////////////////

    //wait for key pressed
    cv::waitKey();

    return 0;
}
