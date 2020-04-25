#include <iostream>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "Histogram.h"
#include "PointOperations.h"
#include "Threshold.h"
#include "Timer.h"

bool trackbarChanged = true;
int trackbarMode = 0;
int trackbarInvert = 0;
int trackbarThreshold = 128;
int trackbarContrast = 5;
int trackbarBrightness = 50 + 255;
int trackbarQuantize = 3;

int trackbarBrightness2 = trackbarBrightness - 255;
float trackbarContrastFloat = float(trackbarContrast) / 10.0f;

void trackbarCallback(int, void *)
{
    trackbarChanged = true;

    // convert to float because trackbar allowes only integer
    trackbarContrastFloat = float(trackbarContrast) / 10.0f;

    // trackbar allows only integer starting with 0, but we need negative values
    trackbarBrightness2 = trackbarBrightness - 255;
}

int main(int argc, char *argv[]) {
    // read image
    cv::Mat img = cv::imread(INPUTIMAGE);

    // display image
    // cv::imshow("Image", img);

    // convert to grayscale
    cv::Mat imgGray;
    cv::cvtColor(img, imgGray, CV_BGR2GRAY);

    // display grayscale
    // cv::imshow("Grayscale", imgGray);

    // main image
    cv::Mat imgMain;
    cv::cvtColor(imgGray, imgMain, CV_GRAY2BGR);
    cv::imshow("Input grayscaled", imgMain);

    // add trackbars to main image
    cv::createTrackbar("Mode", "Input grayscaled", &trackbarMode, 4, trackbarCallback);
    cv::createTrackbar("Invert", "Input grayscaled", &trackbarInvert, 1, trackbarCallback);
    cv::createTrackbar("1:Thresh.", "Input grayscaled", &trackbarThreshold, 400, trackbarCallback);
    cv::createTrackbar("2:Contrast", "Input grayscaled", &trackbarContrast, 50, trackbarCallback);
    cv::createTrackbar("3:Brigthn.", "Input grayscaled", &trackbarBrightness, 510, trackbarCallback);
    cv::createTrackbar("4:Quantize", "Input grayscaled", &trackbarQuantize, 8, trackbarCallback);

    // create class instances
    Threshold *threshold = new Threshold();
    Histogram *histogram = new Histogram();
    PointOperations *pointOperations = new PointOperations();

    // endless loop
    while (true) {
        int key = cv::waitKey(10);
        if (key >= 0) { // any key
            break; // leave endless loop
        }

        if (!trackbarChanged) {
            continue;
        }
        trackbarChanged = false;

        // main image (grayscale) with text
        cv::cvtColor(imgGray, imgMain, CV_GRAY2BGR);
        std::string mainText;
        switch (trackbarMode) {
            case 1:
                mainText = "Mode 1: Threshold  =" + std::to_string(trackbarThreshold);
                break;
            case 2:
                mainText = "Mode 2: Contrast  alpha=" + std::to_string(trackbarContrastFloat);
                break;
            case 3:
                mainText = "Mode 3: Brightness  alpha=" + std::to_string(trackbarBrightness2);
                break;
            case 4:
                mainText = "Mode 4: Quantize  n=" + std::to_string(trackbarQuantize);
                break;
            default:
                mainText = "Mode 0: Grayscale";
        }
        cv::putText(imgMain, mainText, cv::Point(10, 15), CV_FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 255));


        /////////////////////////////////////////////////////////////
        // begin processing
        // ///////////////////////////////////////////////////////////

        // input image
        cv::Mat imgInput = cv::Mat::zeros(cv::Size(imgGray.cols, imgGray.rows), CV_8U);

        // inversion?
        if (trackbarInvert) {
            // invert the image
            pointOperations->invert(imgGray, imgInput);
        }
        else
        {
            // do not invert (just copy the grayscale image)
            imgGray.copyTo(imgInput);
        }

        // output image
        cv::Mat imgOutput;

        // processing depends on mode
        switch (trackbarMode) {
            case 1:
                // mode 1: threashold
                threshold->loop_ptr2(imgInput, imgOutput, trackbarThreshold);
                break;
            case 2:
                // mode 2: contrast
                pointOperations->adjustContrast(imgInput, imgOutput, trackbarContrastFloat, 127);
                break;
            case 3:
                // mode 3: brightness
                pointOperations->adjustBrightness(imgInput, imgOutput, trackbarBrightness2);
                break;
            case 4:
                // mode 4: quantize the image
                pointOperations->quantize(imgInput, imgOutput, trackbarQuantize);
                break;
            default:
                // mode 0: grayscale
                imgInput.copyTo(imgOutput);
        }

        // display output image
        cv::imshow("Output", imgOutput);

        // calculate and display histogram
        cv::Mat imgHist;
        histogram->calcHist(imgOutput, imgHist);
        histogram->show("Histogram", imgHist);

        // calculate and display histogram with OpenCV function
        // histogram->calcHist_cv(imgOutput, imgHist);
        // histogram->show("histogram_grayscale_cv", imgHist);

        // calculate minimum, maximum and mean
        uchar min, max, mean;
        histogram->calcStats(imgHist, min, max, mean);

        /////////////////////////////////////////////////////////////
        // end processing
        /////////////////////////////////////////////////////////////


        // print results
        mainText = "Histogram Min:  " + std::to_string(min);
        cv::putText(imgMain, mainText, cv::Point(10, 50), CV_FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 255));
        mainText = "Histogram Max:  " + std::to_string(max);
        cv::putText(imgMain, mainText, cv::Point(10, 70), CV_FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 255));
        mainText = "Histogram Mean:  " + std::to_string(mean);
        cv::putText(imgMain, mainText, cv::Point(10, 90), CV_FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 255));

        // display main image with trackbars and text
        cv::imshow("Input grayscaled", imgMain);
    }

    return 0;
}
