#include <iostream>
#include <sstream>
#include <iomanip>
#include <stdio.h>
#include <vector>
#include <chrono>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "Threshold.h"
#include "Histogram.h"
#include "PointOperations.h"
#include "Filter.h"
#include "Morphology.h"
#include "Segmentation.h"
#include "Timer.h"
#include "imshow_multiple.h"
#include "Coin.h"

//
// function
//
bool calibrate(Segmentation *segmentation, Coin *coinClass, const cv::Mat imgEdges, const float cellStep,
               const float phiStep, int *rMin, int *rMax, cv::Mat imgInput);

//
// for trackbar
//
// (trackbar values: integer)
//

//
// coin calibration (range of radius)
//
int globalRadiusCalibMin = 15;
int globalRadiusCalibMax = 40;
void trackbarCallbackCalibration(int, void*)
{
    if (globalRadiusCalibMax < globalRadiusCalibMin)
    {
        std::cout << "ERROR: calibration radius:  min > max ==> reset max\n";
        globalRadiusCalibMax = globalRadiusCalibMin;
    }
    std::cout << "CALIBRATION changed: search radius = " << globalRadiusCalibMin << "..." << globalRadiusCalibMax << "\n";
}

//
// blur kernel (global variables, callback functions)
//
cv::Mat globalBlurKernelHorizontal, globalBlurKernelVertical; // 2D kernel separated into 2 1D kernels
int trackbarBlurSigma = 0;
int trackbarBlurKernelSize = 2;
double globalBlurSigma = -1.0;
int globalBlurKernelSize = 0;

void calculateBlurKernels()
{
    Filter *filter = new Filter();
    filter->setGaussianKernels1D(globalBlurKernelHorizontal, globalBlurKernelVertical, globalBlurKernelSize, globalBlurSigma);
}

void trackbarCallbackKernelSize(int, void*)
{
    globalBlurKernelSize = (trackbarBlurKernelSize + 1) * 2 + 1;
    std::cout << "BLUR changed: new kernel size = " << globalBlurKernelSize << "\n";

    if (globalBlurSigma < 0.0)
        return; // do not calculate kernels if sigma is not initialized yet

    calculateBlurKernels();
}

void trackbarCallbackBlurSigma(int, void*)
{
    globalBlurSigma = double(trackbarBlurSigma) / 10.0;
    std::cout << "BLUR changed: new sigma = " << globalBlurSigma << "\n";

    calculateBlurKernels();
}

//
// other trackbars
//
int valueBrightnessInt = 123;
int valueBrightness = 0;
int valueContrastInt = 636;
float valueContrast = 0.0f;
int valueEdgeThInt = 90;

void updateTrackbarValues(int, void*)
{
    valueContrast = float(valueContrastInt) / 100.0;
    valueBrightness = valueBrightnessInt - 255;
}

//
// camera/input image
//
int cameraWidth, cameraHeight;
int viewX1, viewX2, viewY1, viewY2, viewXtmp, viewYtmp;
bool viewOnePointSet = false;

void setViewToFullCameraSize()
{
    viewX1 = 0;
    viewY1 = 0;
    viewX2 = cameraWidth - 1;
    viewY2 = cameraHeight - 1;
    std::cout << "View: set to full camera size\n";
}

void textToImage(cv::Mat image, const std::string text, const int x, const int y, const cv::Scalar color, const cv::Scalar background)
{
    cv::putText(image, text, cv::Point(x + 1, y), CV_FONT_HERSHEY_SIMPLEX, 0.5, background);
    cv::putText(image, text, cv::Point(x - 1, y), CV_FONT_HERSHEY_SIMPLEX, 0.5, background);
    cv::putText(image, text, cv::Point(x, y + 1), CV_FONT_HERSHEY_SIMPLEX, 0.5, background);
    cv::putText(image, text, cv::Point(x, y - 1), CV_FONT_HERSHEY_SIMPLEX, 0.5, background);
    cv::putText(image, text, cv::Point(x, y), CV_FONT_HERSHEY_SIMPLEX, 0.5, color);
}

//
// mouse
//
void mouseCallback(int event, int x, int y, int flags, void* userdata)
{
    if (event == cv::EVENT_RBUTTONDOWN)
    {
        setViewToFullCameraSize();
        viewOnePointSet = false;
        return;
    }

    if (event != cv::EVENT_LBUTTONDOWN)
        return;

    // left mouse key was pressed

    if (!viewOnePointSet)
    {
        // first point
        viewXtmp = x;
        viewYtmp = y;
        viewOnePointSet = true;
        return;
    }

    // second point
    viewOnePointSet = false;
    if (viewXtmp < x)
    {
        viewX1 = viewXtmp;
        viewX2 = x;
    }
    else
    {
        viewX1 = x;
        viewX2 = viewXtmp;
    }
    if (viewYtmp < y)
    {
        viewY1 = viewYtmp;
        viewY2 = y;
    }
    else
    {
        viewY1 = y;
        viewY2 = viewYtmp;
    }

    // minimum view size
    int min = 50;
    if (viewX2 - viewX1 < 50 || viewY2 - viewY1 < 50) {
        std::cout << "Error: view to small\n";
        setViewToFullCameraSize();
    }
}


// image type: 12 = photo

int main(int argc, char *argv[])
{
    // initialize: trackbar values
    trackbarCallbackCalibration(0, nullptr);
    trackbarCallbackKernelSize(0, nullptr);
    trackbarCallbackBlurSigma(0, nullptr);
    updateTrackbarValues(0, nullptr);
    
    // initiate class instances
    Threshold *threshold = new Threshold();
    PointOperations *pointOperations = new PointOperations();
    Filter *filter = new Filter();
    Morphology *morphology = new Morphology();
    Segmentation *segmentation = new Segmentation();
    Coin *coinClass = new Coin();

    // define names for BGR colors
    cv::Scalar colorGreen = cv::Scalar(0, 255, 0);
    cv::Scalar colorBlue = cv::Scalar(255, 0, 0);
    cv::Scalar colorRed = cv::Scalar(0, 0, 255);
    cv::Scalar colorWhite = cv::Scalar(255, 255, 255);
    cv::Scalar colorBlack = cv::Scalar(0, 0, 0);
    cv::Scalar colorYellow = cv::Scalar(0, 255, 255);
    cv::Scalar colorOrange = cv::Scalar(0, 165, 255);

    // default step sizes
    float cellStep = 1.0f;  // size of accumulator cell
                            // (smaller cells: better results, but increased time and memory consumption)
    float phiStep = 1.0f; // step size of radius in accumulator



    // note: circle detection takes some time
    //       if the imput image is not well prepared, a lot of false circles will be detected
    //       e.g. 100 circles can take minutes (on the Raspberry Pi)
    //       during that: program will not respond to inputs (change of trackbars, keys)
    //       to prevent that:
    //
    // limit number of dedectable circles per radius
    int maxCoinCountCalibrated = 20;
    int maxCoinCountUncalibrated = 5;

    // coin detection starts with calibration to get the size of a 1 Euro coin
    // (because coin size depend on the distance between a camera and the coin)
    bool calibrated = false;

    // Hough Transformation for cirles searches for a limited range of radii to save computation time)
    int rMin = globalRadiusCalibMin;
    int rMax = globalRadiusCalibMax;

    // FPS
    bool showFps = true;

    // main window with all trackbars
    cv::Mat imgMain;
    imgMain = cv::Mat::zeros(cv::Size(350, 1), CV_8U);
    imgMain = cv::Mat::zeros(cv::Size(350, 200), CV_8U); // bugfix MS Windows (part 1/2)
    cv::imshow("Main", imgMain);
    // input image
    cv::Mat imgInput;
    
    // detect camera and use it if possible
    bool cameraActive = true;
    cv::VideoCapture capture;
    if (!capture.open(0)) // open camera
    {
        std::cout << "Could not open camera.\n";
        cameraActive = false;
    }
    if (cameraActive && !capture.read(imgInput))
    {
        std::cout << "Unable to read frame from camera.\n";
        cameraActive = false;
    }

    if (cameraActive)
    {
        // use camera image
        cameraWidth = capture.get(CV_CAP_PROP_FRAME_WIDTH);
        cameraHeight = capture.get(CV_CAP_PROP_FRAME_HEIGHT);
        std::cout << "Camera resolution: " << cameraWidth << " x " << cameraHeight << "\n";
    }
    else
    {
        // use image files
        cameraWidth = 640;
        cameraHeight = 480;
        imgInput = cv::Mat::zeros(cv::Size(cameraWidth, cameraHeight), CV_8UC3);
        std::cout << "read images from file, resolution: " << cameraWidth << " x " << cameraHeight << "\n";

    }
    
    //
    // set view (area in which coins are detected)
    //
    
    // use whole input image -> slow
    //setViewToFullCameraSize(); 
    
    // seach only in a part of the image -> faster
    viewX1 = cameraWidth/3;
    viewY1 = cameraHeight/3;
    viewX2 = cameraWidth/3 * 2;
    viewY2 = cameraHeight/3 * 2;
    
    
    
    std::cout << "Press ENTER to calibrate for 1 Euro coin ... or ANY other key to end program.\n";

    
    //
    // create main window and add trackbars to it
    //
    cv::imshow("Input", imgInput);
    
    int enableHough = 0;
    cv::createTrackbar("Enable Hough", "Main", &enableHough, 1, nullptr);
    
    cv::createTrackbar("Calibration radius min", "Main", &globalRadiusCalibMin, 200, trackbarCallbackCalibration);
    cv::createTrackbar("Calibration radius max", "Main", &globalRadiusCalibMax, 200, trackbarCallbackCalibration);

    cv::createTrackbar("Brightness", "Main", &valueBrightnessInt, 511, updateTrackbarValues);
    cv::createTrackbar("Contrast", "Main", &valueContrastInt, 1000, updateTrackbarValues);

    cv::createTrackbar("Blur kernel size", "Main", &trackbarBlurKernelSize, 10, trackbarCallbackKernelSize);
    cv::createTrackbar("Blur sigma", "Main", &trackbarBlurSigma, 100, trackbarCallbackBlurSigma);

    cv::createTrackbar("Threshold 1", "Main", &valueEdgeThInt, 255, nullptr);

    // add trackbar for image selection (only if no camera was found)
    int imageNo = 0;
    if (!cameraActive)
        cv::createTrackbar("Image selection", "Main", &imageNo, 12, nullptr);

    // add trackbar for alternative versions of functions (e.g usage of threads to improve speed)
    // note: you do not have to use this
    int alternative = 1;
    cv::createTrackbar("Use threads", "Main", &alternative, 1, nullptr);

    // add mouse callback to input window
    cv::setMouseCallback("Input", mouseCallback, NULL);

    // create other output windows
    cv::Mat imgGray, imgBrightness, imgContrast, imgBlur, imgThresh, imgEroded, imgEdges, imgHough, imgResult;
    cv::Mat imgSubtracted;
    imgBlur = cv::Mat::zeros(cv::Size(cameraWidth, cameraHeight), CV_8U);
    imgEdges = cv::Mat::zeros(cv::Size(cameraWidth, cameraHeight), CV_8U);
    cv::imshow("Prepared grayscale", imgBlur);
    cv::imshow("Edges", imgEdges);
    imgMain = cv::Mat::zeros(cv::Size(350, 1), CV_8U); // bugfix for MS Windows (part 2/2)
    cv::imshow("Main", imgMain);


    while (true) // endless loop
    {
        // meassure time (for calculation of fps)
        auto timerStart = std::chrono::high_resolution_clock::now();

        // get image
        if (cameraActive)
        {
            // capture picture
            if (!capture.read(imgInput))
            {
                std::cout << "Camera: Unable to read frame from camera.\n";
                return 0;
            }
        }
        else
        {
            // load image from file
            std::string fileName(INPUTIMAGEDIR);
            fileName += "/coin" + std::to_string(imageNo) + ".tiff";
            imgInput = cv::imread(fileName, CV_LOAD_IMAGE_COLOR);
        }


        //
        // apply view and convert to grayscale
        //
        cv::cvtColor(imgInput(cv::Rect(viewX1, viewY1, viewX2 - viewX1, viewY2 - viewY1)), imgGray, cv::COLOR_BGR2GRAY);


        //
        // adjust brightness
        //
        pointOperations->adjustBrightness(imgGray, imgBrightness, valueBrightness);


        //
        // adjust contrast
        //
        pointOperations->adjustContrast(imgBrightness, imgContrast, valueContrast);


        //
        // blur
        //

        // convloution uses float -> convert to float
        cv::Mat imgContrastFloat;
        imgContrast.convertTo(imgContrastFloat, CV_32F);
        
        // 2x convolution with 1D kernel
        cv::Mat blurHorizontal, blurVertical;
        filter->convolve_generic_normalized_float_kernel(imgContrastFloat, blurHorizontal, globalBlurKernelHorizontal);
        filter->convolve_generic_normalized_float_kernel(blurHorizontal, blurVertical, globalBlurKernelVertical);

        // convert back to uchar and remove cropped edges
        int borderSize = globalBlurKernelSize / 2 + 1;
        blurVertical(cv::Rect(borderSize, borderSize, blurVertical.cols - borderSize - borderSize,
                              blurVertical.rows - borderSize - borderSize)).convertTo(imgBlur, CV_8U);

        // show image
        cv::imshow("Prepared grayscale", imgBlur);


        //
        // edge detection (threshold -> erode -> substract)
        //
        threshold->loop_ptr2(imgBlur, imgThresh, valueEdgeThInt);
        morphology->erode(imgThresh, imgEroded, morphology->getKernelFull(3));
        morphology->subtract(imgThresh, imgSubtracted, imgEroded);
        
        // subtraction can creeate a (white) border -> use part of image without border
        imgEdges = imgSubtracted(cv::Rect(2, 2, imgSubtracted.cols - 4, imgSubtracted.rows - 4));
        
        // show image
        cv::imshow("Edges", imgEdges);

        // create empty list of circles
        std::vector<CircleItem> circles;

        // add view indicator to camera window
        cv::rectangle(imgInput, cv::Point(viewX1, viewY1), cv::Point(viewX2, viewY2), colorGreen);

        //
        // program is operated by keyboard
        //  ENTER:           start coin calibration
        //  f or s:          show/hide show FPS
        //  (any other key): end program
        //
        // as long as no key is pressed input images will be read (from file or camera)

        int key = cv::waitKey(20); // catch pressed key and wait for some time to draw the images
        if (key == 10 || key == 13 || key == 1048586 || key == 1113997 || key == 65421) // key 'ENTER' (code depends on the system)
        {
            // calibrate: detect reference coin (1 Euro)
            calibrated = calibrate(segmentation, coinClass, imgEdges, cellStep, phiStep, &rMin, &rMax, imgInput);
            continue;
        }
        else if ((key == 102 || key == 1048678) || (key == 115 || key == 1048691)) // key 'f' or key 's'
        {
            showFps = !showFps;
        }
        else if (key >= 0) // any other key -> end program
        {
            std::cout<<"unknown key pressed (" << key << ") -> end program.\n";
            return 0;
        }

        std::stringstream mainWindowText;
        if (enableHough)
        {
            coinClass->setImageNo(imageNo); // image number is used as a (very simple) color calibration
            
            if (calibrated)
            {
                //
                // detect coins, count sum and mark coins in camera image
                //

                // find cirlces
                if (!alternative)
                    segmentation->findCircles(imgEdges, &circles, rMin, rMax, cellStep, phiStep, maxCoinCountCalibrated);
                else;
                    segmentation->findCirclesThread(imgEdges, &circles, rMin, rMax, cellStep, phiStep, maxCoinCountCalibrated);

                coinClass->removeOverlappingCircles(&circles);

                // show list of circles
                // coinClass->showCirleList(&circles);

                double sum = 0.0;
                for (auto circle : circles)
                {
                    int x = circle.x + viewX1 + borderSize;
                    int y = circle.y + viewY1 + borderSize;

                    double value = coinClass->getCoinValue(imgInput, x, y, circle.r);

                    // color depends on coin
                    cv::Scalar color = value >= 1.0
                        ? colorWhite
                        : value >= 0.1
                        ? colorYellow
                        : value >= 0.01 ? colorRed : colorBlue;

                    // draw circle
                    cv::circle(imgInput, cv::Point(x, y), circle.r, color);

                    
                    if (value >= 0.01) {
                        // coin detected
                        sum += value;

                        // draw value as number
                        std::stringstream text;
                        text << std::fixed << std::setprecision(2) << value;
                        textToImage(imgInput, text.str().c_str(), x - 18, y + 5, color, colorBlack);
                    }
                }
                mainWindowText << "SUM=" << std::fixed << std::setprecision(2) << sum;
            }
            else
            {
                //
                // no calibration yet -> mark cirles
                //

                // find cirlces
                if (!alternative)
                    segmentation->findCircles(imgEdges, &circles, rMin, rMax, cellStep, phiStep, maxCoinCountUncalibrated);
                else
                    segmentation->findCirclesThread(imgEdges, &circles, rMin, rMax, cellStep, phiStep, maxCoinCountUncalibrated);

                mainWindowText << "Not calibrated yet. Use 1 Euro coin and press ENTER for calibration.";
                for (auto circle : circles)
                {
                    cv::circle(imgInput,
                            cv::Point(viewX1 + borderSize  + circle.x, viewY1 + borderSize + circle.y), circle.r, colorBlue);
                }
            }
        }
        
        if (showFps)
        {
            auto timerEnd = std::chrono::high_resolution_clock::now();
            auto time = std::chrono::duration_cast<std::chrono::milliseconds>(timerEnd - timerStart).count();

            std::stringstream text;
            text << "FPS: " << std::fixed << std::setprecision(2) << (1000.0f / float(time));
            textToImage(imgInput, text.str().c_str(), 10, cameraHeight - 20, colorBlack, colorWhite);

        }

        // show main window with text
        textToImage(imgInput, mainWindowText.str().c_str(), 10, 15, colorBlack, colorWhite);
        cv::imshow("Input", imgInput);
    } // endless loop
}

bool calibrate(Segmentation *segmentation, Coin *coinClass, const cv::Mat imgEdges, const float cellStep,
               const float phiStep, int *rMin, int *rMax, cv::Mat imgInput)
{
    //
    // calibrate: detect reference coin (1 Euro)
    //
    int referenceRadius = -1;
    int valueMax = 0;
    cv::Point referenceCenter;

    std::cout << "Start calibration ...\n";
    for (int r = globalRadiusCalibMin; r <= globalRadiusCalibMax; ++r) {
        // Hough Transformation
        cv::Mat imgHough;
        segmentation->houghCircle(imgEdges, imgHough, r, cellStep, phiStep);

        // show hough space
        cv::Mat imgHoughScaled;
        segmentation->scaleHoughImage(imgHough, imgHoughScaled);

        // find one circle
        int value;
        cv::Point center = segmentation->findAndRemoveMaximum(imgHough, &value, r, cellStep);

        // evaluate radius
        if (value > valueMax && referenceCenter.x < cameraWidth - r - 10 &&
            referenceCenter.y < cameraWidth - r - 10) {
            valueMax = value;
            referenceRadius = r;
            referenceCenter = center;
        }
    }
    std::cout << "Calibration coin: radius=" << referenceRadius
              << "\tx=" << referenceCenter.x << "\ty=" << referenceCenter.y << '\n';

    if (referenceRadius < 1 || referenceCenter.x < referenceRadius + 10 ||
    referenceCenter.x > cameraWidth - referenceRadius - 10 ||
    referenceCenter.y < referenceRadius + 10 ||
    referenceCenter.y > cameraWidth - referenceRadius - 10)
    {
        std::cout << "Coin not found or coin (partially) outside of camera image.\n";
    }
    else
    {
        // update coin radii (calculated in relation to reference coin)
        coinClass->setReferenceCoin(imgInput, 1.00, referenceRadius, referenceCenter.x, referenceCenter.y);

        *rMin = coinClass->radiusMinPixel;
        *rMax = coinClass->radiusMaxPixel;

        std::cout << "Reference coin found and accepted. New search radius for coins: "
                  << *rMin << " ... " << *rMax << " px\n";
        return true; // calibration successful
    }
    return false; // calibration failed
}

