#include <iostream>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "imshow_multiple.h"
#include "Morphology.h"

int main(int argc, char *argv[])
{
    //create class instance
    Morphology *morphology = new Morphology();

    ///////////////////////////////////////////////////////////////////////////////
    // Closing: dilate then erode
    ///////////////////////////////////////////////////////////////////////////////

    // Closing #1
    cv::Mat imgText1 = cv::imread(INPUTIMAGEDIR "/text1.tiff", CV_LOAD_IMAGE_GRAYSCALE);
    cv::threshold(imgText1, imgText1, 0, 255, CV_THRESH_BINARY);
    cv::Mat imgtext1Dilated, imgText1Eroded;
    morphology->dilate(imgText1, imgtext1Dilated, morphology->getKernelPlus());
    morphology->erode(imgtext1Dilated, imgText1Eroded, morphology->getKernelPlus());
    imshow_multiple("Closing #1, kernel 3x3 'plus'", 3, &imgText1, &imgtext1Dilated, &imgText1Eroded);

    // Closing #2
    cv::Mat imgText2 = cv::imread(INPUTIMAGEDIR "/text2.tiff", CV_LOAD_IMAGE_GRAYSCALE);
    cv::threshold(imgText2, imgText2, 0, 255, CV_THRESH_BINARY);
    cv::Mat imgtext2Dilated, imgText2Eroded;
    morphology->dilate(imgText2, imgtext2Dilated, morphology->getKernelLine());
    morphology->erode(imgtext2Dilated, imgText2Eroded, morphology->getKernelLine());
    imshow_multiple("Closing #2, kernel 1x5 'line'", 3, &imgText2, &imgtext2Dilated, &imgText2Eroded);

    // Closing #3
    cv::Mat imgClosing3 = cv::imread(INPUTIMAGEDIR "/dots.tiff", CV_LOAD_IMAGE_GRAYSCALE);
    cv::threshold(imgClosing3, imgClosing3, 0, 255, CV_THRESH_BINARY);
    cv::Mat imgKernelDot = cv::imread(INPUTIMAGEDIR "/kernel_dot.tiff", CV_LOAD_IMAGE_GRAYSCALE);
    cv::threshold(imgKernelDot, imgKernelDot, 0, 255, CV_THRESH_BINARY);
    cv::Mat imgClosing3Dilated, imgClosing3Eroded;
    morphology->dilate(imgClosing3, imgClosing3Dilated, imgKernelDot);
    morphology->erode(imgClosing3Dilated, imgClosing3Eroded, imgKernelDot);
    imshow_multiple("Closing #3", 3, &imgClosing3, &imgClosing3Dilated, &imgClosing3Eroded);

    ///////////////////////////////////////////////////////////////////////////////
    // Opening: erode then dilate
    ///////////////////////////////////////////////////////////////////////////////

    // Opening #1
    cv::Mat imgFingerprint = cv::imread(INPUTIMAGEDIR "/fingerprint.tiff", CV_LOAD_IMAGE_GRAYSCALE);
    cv::threshold(imgFingerprint, imgFingerprint, 0, 255, CV_THRESH_BINARY);
    cv::Mat imgFingerprintEroded, imgFingerprintDilated;
    morphology->erode(imgFingerprint, imgFingerprintEroded, morphology->getKernelFull(3));
    morphology->dilate(imgFingerprintEroded, imgFingerprintDilated, morphology->getKernelFull(3));
    imshow_multiple("Opening #1, kernel 3x3", 3, &imgFingerprint, &imgFingerprintEroded, &imgFingerprintDilated);

    // Opening #2
    cv::Mat imgText3 = cv::imread(INPUTIMAGEDIR "/text3.tiff", CV_LOAD_IMAGE_GRAYSCALE);
    cv::threshold(imgText3, imgText3, 0, 255, CV_THRESH_BINARY);
    cv::Mat imgText3Eroded, imgText3Dilated;
    morphology->erode(imgText3, imgText3Eroded, morphology->getKernelFull(4));
    morphology->dilate(imgText3Eroded, imgText3Dilated, morphology->getKernelFull(4));
    imshow_multiple("Opening #2, kernel 4x4", 3, &imgText3, &imgText3Eroded, &imgText3Dilated);

    ///////////////////////////////////////////////////////////////////////////////
    // Boundary extraction: subtract eroded image from original image
    ///////////////////////////////////////////////////////////////////////////////

    cv::Mat img = cv::imread(INPUTIMAGEDIR "/segments.tiff", CV_LOAD_IMAGE_GRAYSCALE);
    cv::threshold(img, img, 0, 255, CV_THRESH_BINARY);
    cv::Mat imgEroded, imgsubtracted;
    morphology->erode(img, imgEroded, morphology->getKernelFull(3));
    morphology->subtract(img, imgsubtracted, imgEroded);
    imshow_multiple("Boundary extraction", 3, &img, &imgEroded, &imgsubtracted);

    //wait for key pressed
    cv::waitKey();

    return 0;
}
