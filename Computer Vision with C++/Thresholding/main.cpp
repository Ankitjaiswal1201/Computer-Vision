#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "Timer.h"
///////////////////////////////////////////////////////////////////////////////
// compute threshold image using the OpenCV function
///////////////////////////////////////////////////////////////////////////////
void threshold_cv(const cv::Mat &input, cv::Mat &output, uchar threshold)
{
    cv::threshold(input, output, 128, 255, cv::THRESH_BINARY);
}
///////////////////////////////////////////////////////////////////////////////
// compute threshold image by looping over the elements
///////////////////////////////////////////////////////////////////////////////
void threshold_loop(const cv::Mat &input, cv::Mat &output, uchar threshold)
{
    int rows = input.rows;
    int cols = input.cols;
    
    output.release();
    output.create(rows, cols, CV_8U);
    
    for (int r = 0; r < rows; ++r)
    {
        for (int c = 0; c < cols; ++c)
        {
            if (input.at<uchar>(r, c) >= threshold)
                output.at<uchar>(r, c) = 255;
            else
                output.at<uchar>(r, c) = 0;
        }}}
///////////////////////////////////////////////////////////////////////////////
// compute threshold image by looping over the elements (pointer access)
///////////////////////////////////////////////////////////////////////////////
void threshold_loop_ptr(const cv::Mat &input, cv::Mat &output, uchar threshold)
{
    int rows = input.rows;
    int cols = input.cols;
    
    output.release();
    output.create(rows, cols, CV_8U);
    
    if (input.isContinuous())
    {
        cols = rows*cols;
        rows = 1;
    }
    
    for (int r = 0; r < rows; ++r)
    {
        const uchar *pInput = input.ptr<uchar>(r);
        uchar *pOutput = output.ptr<uchar>(r);
        
        for (int c = 0; c < cols; ++c)
        {
            
            if (pInput[c] >= threshold)
                pOutput[c] = 255;
            else
                pOutput[c] = 0;
        } }}
///////////////////////////////////////////////////////////////////////////////
// compute threshold image by looping over the elements (pointer access)
///////////////////////////////////////////////////////////////////////////////
void threshold_loop_ptr2(const cv::Mat &input, cv::Mat &output, uchar threshold)
{
    int rows = input.rows;
    int cols = input.cols;
    
    output.release();
    output.create(rows, cols, CV_8U);
    
    if (input.isContinuous())
    {
        cols = rows*cols;
        rows = 1;
    }
    
    for (int r = 0; r < rows; ++r)
    {
        const uchar *pInput = input.ptr<uchar>(r);
        uchar *pOutput = output.ptr<uchar>(r);
        
        for (int c = 0; c < cols; ++c)
        {
            
            if (*pInput >= threshold)
                *pOutput = 255;
            else
                *pOutput = 0;
            
            ++pInput;
            ++pOutput;
        }}}
int main(int argc, char *argv[])
{
    //read image
    cv::Mat img = cv::imread("../data/lena.tiff");
    //display image
    cv::imshow("Image", img);
    
    //convert to grayscale
    cv::Mat imgGray;
    cv::cvtColor(img, imgGray, CV_BGR2GRAY);
    //display grayscale
    cv::imshow("Grayscale", imgGray);

    //declare output variables
    cv::Mat output;
    
    // begin processing ///////////////////////////////////////////////////////////

    INIT_TIMER
    
    START_TIMER
    threshold_cv(imgGray, output, 128);
    STOP_TIMER("threshold_cv")

    cv::imshow("threshold_cv", output);

    
    START_TIMER
    threshold_loop(imgGray, output, 200);
    STOP_TIMER("threshold_loop")

    cv::imshow("threshold_loop", output);

    
    START_TIMER
    threshold_loop_ptr(imgGray, output, 50);
    STOP_TIMER("threshold_loop_ptr")

    cv::imshow("threshold_loop_ptr", output);
    
    
    START_TIMER
    threshold_loop_ptr2(imgGray, output, 160);
    STOP_TIMER("threshold_loop_ptr2")

    cv::imshow("threshold_loop_ptr2", output);
    // end processing /////////////////////////////////////////////////////////////
    //wait for key pressed
    cv::waitKey();
    return 0;
}
