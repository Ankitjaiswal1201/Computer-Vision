#include <iostream>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <iostream>
#include <stdio.h>
#include <vector>

#include "Threshold.h"
#include "Histogram.h"
#include "PointOperations.h"
#include "Filter.h"
#include "Morphology.h"
#include "Segmentation.h"
#include "Timer.h"
#include "imshow_multiple.h"


void waitAndCloseWindows()
{
    std::cout << "\npress any key to continue ...\n";
    cv::waitKey();
    cv::destroyAllWindows();
}

int main(int argc, char *argv[])
{
    cv::Scalar colorRed = cv::Scalar(0, 0, 255); // color in BGR image
    Threshold *threshold = new Threshold();
    PointOperations *pointOperations = new PointOperations();
    Morphology *morphology = new Morphology();
    Segmentation *segmentation = new Segmentation();


    // default step sizes
    float cellStep = 1.0f;  // size of accumulator cell
                            // (smaller cells: better results, but increased time and memory consumption)
    float phiStep = 1.0f; // step size of radius in accumulator


    //other reasonable step sizes:
    //cellStep = 0.2f;
    //phiStep = 0.1f;



    ////////////////////////////////////////////////////////////////////////////////////
    // part A: apply the Circle Hough Transformation and have a look at the
    //         accumulator (Hough parameter space)
    ////////////////////////////////////////////////////////////////////////////////////

    std::cout << "##########\n# PART A #\n##########\n";
    {
        // load images as grayscale
        cv::Mat img1 = cv::imread(INPUTIMAGEDIR "/one_circle_r12.tiff", CV_LOAD_IMAGE_GRAYSCALE);
        cv::Mat img2 = cv::imread(INPUTIMAGEDIR "/two_circles_different_size.tiff", CV_LOAD_IMAGE_GRAYSCALE);
        cv::imshow("Original: one circle", img1);
        cv::imshow("Original: two circles", img2);

        // (1) Circle Hough Transformation: image with one circle
        cv::Mat hough1a, hough1b, hough2a, hough2b;
        int radius = 12;
        cellStep = 0.2f;
        phiStep = 0.1f;
        segmentation->houghCircle(img1, hough1a, radius, cellStep, phiStep);

        // (2) ... with a different cellStep and phiStep
        cellStep = 1.0f;
        phiStep = 1.0f;
        segmentation->houghCircle(img1, hough1b, radius, cellStep, phiStep);

        // (3) ... image with different circles
        segmentation->houghCircle(img2, hough2a, radius, cellStep, phiStep);

        // (4) ... same image again, but with different radius
        int radius2 = 30;
        segmentation->houghCircle(img2, hough2b, radius2, cellStep, phiStep);


        // Hough space data type: 32 bit integer
        // to show it we have to convert it (e.g. to a 8 bit grayscale)
        cv::Mat out1a, out1b, out2a, out2b;
        segmentation->scaleHoughImage(hough1a, out1a);
        segmentation->scaleHoughImage(hough1b, out1b);
        segmentation->scaleHoughImage(hough2a, out2a);
        segmentation->scaleHoughImage(hough2b, out2b);

        // show images
        cv::imshow("Hough: one circle, r=" + std::to_string(radius) +" , small step sizes", out1a);
        cv::imshow("Hough: one circle, r=" + std::to_string(radius) + ", default step sizes", out1b);
        cv::imshow("Hough: two circles, r=" + std::to_string(radius), out2a);
        cv::imshow("Hough: two circles, r=" + std::to_string(radius2), out2b);
    }
    waitAndCloseWindows();




    ////////////////////////////////////////////////////////////////////////////////////
    // part B: find cirles with unknown radius
    // 
    // problems:
    // - 3 parameters (x, y, r) -> 3D parameter space (a, b, r)
    // - for every possible radius one have to find local maxima in Hough space and
    //   evaluate if it is a center of a circle or not
    // - every circle will have local maxima in for many different r; one have to find
    //   the correct radius
    //
    // goal:
    // - function 'findCircles' which finds all the circles in the following images
    //   (every circle must be found only once and with the correct radius)
    //
    // note:
    //   the 3D space can be stored in a 3D cv::Mat with
    //        cv::Mat output;
    //        int dims[] = { dimA, dimB, dimR };
    //        output = cv::Mat::zeros(3, dims, CV_32S); 
    //   or alternatively: create a 2D Hough space (a, b) for every radius separately and
    //   store it in a 2D cv::Mat
    ////////////////////////////////////////////////////////////////////////////////////

    std::cout << "##########\n# PART B #\n##########\n";

    // min. and max. radius
    int rMin = 10;
    int rMax = 35;
    
    //
    // image with 1 circle
    //
    {
        std::string name = "one_circle_r12.tiff";
        std::cout << std::endl << name << std::endl;

        // load gray image and create color image
        cv::Mat imgGray = cv::imread(INPUTIMAGEDIR "/" + name, CV_LOAD_IMAGE_GRAYSCALE);
        cv::Mat imgColor, imgResult;
        cv::cvtColor(imgGray, imgColor, cv::COLOR_GRAY2BGR);
        imgColor.copyTo(imgResult);

        // list of circles
        std::vector<CircleItem> circles;

        // find cirlces
        segmentation->findCircles(imgGray, &circles, rMin, rMax, cellStep, phiStep);

        // draw circles in BGR image
        for (auto circle : circles)
        {
            cv::circle(imgResult, cv::Point(circle.x, circle.y), circle.r, colorRed);
        }

        // show
        imshow_multiple(name, 2, &imgColor, &imgResult); // all pictures must have same color type and size
    }

    
    //
    // image with 2 circles (same size)
    //
    {
        std::string name = "two_circles_r12.tiff";
        std::cout << std::endl << name << std::endl;

        // load gray image and create color image
        cv::Mat imgGray = cv::imread(INPUTIMAGEDIR "/" + name, CV_LOAD_IMAGE_GRAYSCALE);
        cv::Mat imgColor, imgResult;
        cv::cvtColor(imgGray, imgColor, cv::COLOR_GRAY2BGR);
        imgColor.copyTo(imgResult);

        // list of circles
        std::vector<CircleItem> circles;

        // find cirlces
        segmentation->findCircles(imgGray, &circles, rMin, rMax, cellStep, phiStep);

        // draw circles in BGR image
        for (auto circle : circles)
        {
            cv::circle(imgResult, cv::Point(circle.x, circle.y), circle.r, colorRed);
        }

        // show
        imshow_multiple(name, 2, &imgColor, &imgResult); // all pictures must have same color type and size
    }


    //
    // image with 2 circles (different size)
    //
    {
        std::string name = "two_circles_different_size.tiff";
        std::cout << std::endl << name << std::endl;

        // load gray image and create color image
        cv::Mat imgGray = cv::imread(INPUTIMAGEDIR "/" + name, CV_LOAD_IMAGE_GRAYSCALE);
        cv::Mat imgColor, imgResult;
        cv::cvtColor(imgGray, imgColor, cv::COLOR_GRAY2BGR);
        imgColor.copyTo(imgResult);

        // list of circles
        std::vector<CircleItem> circles;

        // find cirlces
        segmentation->findCircles(imgGray, &circles, rMin, rMax, cellStep, phiStep);

        // draw circles in BGR image
        for (auto circle : circles)
        {
            cv::circle(imgResult, cv::Point(circle.x, circle.y), circle.r, colorRed);
        }

        // show
        imshow_multiple(name, 2, &imgColor, &imgResult); // all pictures must have same color type and size
    }


    //
    // image with 3 circles
    //
    {
        std::string name = "3_circles.tiff";
        std::cout << std::endl << name << std::endl;

        // load gray image and create color image
        cv::Mat imgGray = cv::imread(INPUTIMAGEDIR "/" + name, CV_LOAD_IMAGE_GRAYSCALE);
        cv::Mat imgColor, imgResult;
        cv::cvtColor(imgGray, imgColor, cv::COLOR_GRAY2BGR);
        imgColor.copyTo(imgResult);

        // list of circles
        std::vector<CircleItem> circles;

        // find cirlces
        segmentation->findCircles(imgGray, &circles, rMin, rMax, cellStep, phiStep);

        // draw circles in BGR image
        for (auto circle : circles)
        {
            cv::circle(imgResult, cv::Point(circle.x, circle.y), circle.r, colorRed);
        }

        // show
        imshow_multiple(name, 2, &imgColor, &imgResult); // all pictures must have same color type and size
    }

    waitAndCloseWindows();
    


    ////////////////////////////////////////////////////////////////////////////////////
    // part C: eye tracing with Circle Hough Transformation
    ////////////////////////////////////////////////////////////////////////////////////

    std::cout << "##########\n# PART C #\n##########\n";
    {
        cellStep = 0.5f;
        phiStep = 0.1f;

        // image filename
        std::string name = "face.tiff";
        
        // load image
        std::cout << std::endl << name << std::endl;
        cv::Mat imgColor = cv::imread(INPUTIMAGEDIR "/" + name, CV_LOAD_IMAGE_COLOR);


        ////////////////////////////////////////////////////////////////////////////////////
        // steps:
        // 1. prepare the image
        // 2. find edges
        // 3. find eyes with Circle Hough Transformation
        // 4. draw the found circles into the original image (and show the image)
        ////////////////////////////////////////////////////////////////////////////////////

        // step 1
        cv::Mat imgGray, imgBrightness, imgContrast, imgThresh, imgEroded, imgSubtracted, imgHough, imgResult;
        cv::cvtColor(imgColor, imgGray, cv::COLOR_BGR2GRAY);

        // adjust brightness and contrast
        pointOperations->adjustBrightness(imgGray, imgBrightness, 100);
        pointOperations->adjustContrast(imgBrightness, imgContrast, 2.5);

        // threshold
        threshold->loop_ptr2(imgContrast, imgThresh, 255);
        
        // step 2: find edges (substract eroded image from original image)
        morphology->erode(imgThresh, imgEroded, morphology->getKernelFull(3));
        morphology->subtract(imgThresh, imgSubtracted, imgEroded);

        // step 3: Hough Transformation
        int r = 12;
        segmentation->houghCircle(imgSubtracted, imgHough, r, cellStep, phiStep);

        // find circles
        imgColor.copyTo(imgResult);
        for (int i = 0; i < 2; ++i)
        {
            int value;
            cv::Point point = segmentation->findAndRemoveMaximum(imgHough, &value, r, cellStep);
            std::cout << "  found circle   center: " << point.x << "," << point.y << std::endl;
            
            // step 4: use found circle center and draw red cirles
            cv::circle(imgResult, cv::Point(point.x, point.y), r, colorRed);
        }

        // imshow_multiple: all pictures must have same color type
        cv::Mat tmpContrast, tmpThresh, tmpSubtracted;
        cv::cvtColor(imgContrast, tmpContrast, cv::COLOR_GRAY2BGR);
        cv::cvtColor(imgThresh, tmpThresh, cv::COLOR_GRAY2BGR);
        cv::cvtColor(imgSubtracted, tmpSubtracted, cv::COLOR_GRAY2BGR);
        imshow_multiple("Face: (1) brightness and contrast adjusted  (2) threshold  (3) edges  (4) found circles", 4,
                        &tmpContrast, &tmpThresh, &tmpSubtracted, &imgResult);

        // show hough space
        cv::Mat imgHoughScaled;
        segmentation->scaleHoughImage(imgHough, imgHoughScaled);
        cv::imshow("Face: Hough Transformation", imgHoughScaled);
    }


    //wait for key pressed
    cv::waitKey();
    return 0;
}
