#include <iostream>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "Threshold.h"
#include "Histogram.h"
#include "PointOperations.h"
#include "Filter.h"
#include "Timer.h"
#include "imshow_multiple.h"

int main(int argc, char *argv[])
{
    //read image
    cv::Mat img = cv::imread(INPUTIMAGE);

    //convert to grayscale
    cv::Mat imgGray;
    cv::cvtColor(img, imgGray, CV_BGR2GRAY);

    // convert to a float image
    cv::Mat imgGray_float;
    imgGray.convertTo(imgGray_float, CV_32F, 1.0/255.0);
    
    //declare output variables
    cv::Mat imgSmoothed3x3, imgSmoothed5x5, imgSmoothed3x1, imgSmoothed5x1;
    cv::Mat img_x_Sobel3x3, img_y_Sobel3x3, img_x_Sobel5x5, img_y_Sobel5x5;
    cv::Mat imgAbsSobel3x3, imgAbsSobel5x5;

    //create class instances
    Threshold *threshold = new Threshold();
    Histogram *histogram = new Histogram();
    PointOperations *pointOperations = new PointOperations();
    Filter *filter = new Filter();

    // begin processing ///////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////////////////
    // Binomial filter with pre-defined kernels
    //////////////////////////////////////////////////////////////////////////////
    filter->convolve_3x3(imgGray_float, imgSmoothed3x3, filter->getBinomial(3));
    filter->convolve_generic(imgGray_float, imgSmoothed5x5, filter->getBinomial(5));
    imshow_multiple("Gaussian Filter", 3, &imgGray_float, &imgSmoothed3x3, &imgSmoothed5x5);

    
    //////////////////////////////////////////////////////////////////////////////
    // x-Sobel filter with pre-defined kernels
    //////////////////////////////////////////////////////////////////////////////
    filter->convolve_3x3(imgGray_float, img_x_Sobel3x3, filter->getSobelX(3));
    filter->convolve_generic(imgGray_float, img_x_Sobel5x5, filter->getSobelX(5));
    filter->convolve_cv(imgGray_float, img_x_Sobel5x5, filter->getSobelX(5));
    
    // new image value scale -> so we can see more
    filter->scaleSobelImage(img_x_Sobel3x3, img_x_Sobel3x3);
    filter->scaleSobelImage(img_x_Sobel5x5, img_x_Sobel5x5);
    imshow_multiple("Sobel Filter X", 3, &imgGray_float, &img_x_Sobel3x3, &img_x_Sobel5x5);

    
    //////////////////////////////////////////////////////////////////////////////
    // y - Sobel filter with pre-defined kernels
    //////////////////////////////////////////////////////////////////////////////
    filter->convolve_3x3(imgGray_float, img_y_Sobel3x3, filter->getSobelY(3));
    filter->convolve_generic(imgGray_float, img_y_Sobel5x5, filter->getSobelY(5));
    filter->convolve_cv(imgGray_float, img_y_Sobel5x5, filter->getSobelY(5));
    
    // new image value scale -> so we can see more
    filter->scaleSobelImage(img_y_Sobel3x3, img_y_Sobel3x3);
    filter->scaleSobelImage(img_y_Sobel5x5, img_y_Sobel5x5);
    imshow_multiple("Sobel Filter Y", 3, &imgGray_float, &img_y_Sobel3x3, &img_y_Sobel5x5);

    
    //////////////////////////////////////////////////////////////////////////////
    // calculate the abs() of the Sobel images
    //////////////////////////////////////////////////////////////////////////////
    filter->getAbsOfSobel(img_x_Sobel3x3, img_y_Sobel3x3, imgAbsSobel3x3);
    filter->getAbsOfSobel(img_x_Sobel5x5, img_y_Sobel5x5, imgAbsSobel5x5);
    imshow_multiple("Abs() Sobel", 2, &imgAbsSobel3x3, &imgAbsSobel5x5);


    ///////////////////////////////
    // Comparison normal and separated convolution
    //////////////////////////////
    
    INIT_TIMER

    START_TIMER
    filter->convolve_generic(imgGray_float, imgSmoothed5x5, filter->getBinomial(5));
    STOP_TIMER("Normal 5x5 convolution")

    START_TIMER
    filter->convolve_generic(imgGray_float, imgSmoothed5x1, filter->getBinomialSeparated(5, false));
    filter->convolve_generic(imgSmoothed5x1, imgSmoothed5x5, filter->getBinomialSeparated(5, true));
    STOP_TIMER("Separated 5x5 convolution")

    START_TIMER
    filter->convolve_generic(imgGray_float, imgSmoothed3x3, filter->getBinomial(3));
    STOP_TIMER("Normal 3x3 convolution")

    START_TIMER
    filter->convolve_generic(imgGray_float, imgSmoothed3x1, filter->getBinomialSeparated(3, false));
    filter->convolve_generic(imgSmoothed3x1, imgSmoothed3x3, filter->getBinomialSeparated(3, true));
    STOP_TIMER("Separated 3x3 convolution")

    // end processing /////////////////////////////////////////////////////////////

    //wait for key pressed
    cv::waitKey();

    return 0;
}
