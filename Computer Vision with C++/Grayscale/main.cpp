#include <iostream>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "Timer.h"

#include <arm_neon.h>

// convert BGR image to grayscale (manually with C++ - code)
void reference_convert (uint8_t * __restrict dest, uint8_t * __restrict src, int n) {
  int i;
  for (i=0; i<n; i++) {
    int b = *src++; // load blue
    int g = *src++; // load green
    int r = *src++; // load red

    // build weighted average
    // the used weights (77, 150 and 29) are the same ones used by standard OpenCV grayscaling
    // it is also possible to use equal weights: y=(r+g+b)/3
    int y = (r*77)+(g*150)+(b*29);

    // undo the scale by 256 and write to memory:
    *dest++ = (y>>8);
  }
}

// convert BGR image to grayscale using hardware acceleration of the Raspberry Pi (NEON)
// this NEON intrinistics code is only used to demonstrate the abilities of the hardware
// in this seminar you do NOT have to understand or write any NEON code
void neon_convert (uint8_t * __restrict dest, uint8_t * __restrict src, int n) {
  int i;
  uint8x8_t bfac = vdup_n_u8 (77);
  uint8x8_t gfac = vdup_n_u8 (150);
  uint8x8_t rfac = vdup_n_u8 (29);
  n/=8;

  for (i=0; i<n; i++) {
    uint16x8_t temp;
    uint8x8x3_t rgb  = vld3_u8 (src);
    uint8x8_t result;

    temp = vmull_u8 (rgb.val[0],      rfac);
    temp = vmlal_u8 (temp,rgb.val[1], gfac);
    temp = vmlal_u8 (temp,rgb.val[2], bfac);

    result = vshrn_n_u16 (temp, 8);
    vst1_u8 (dest, result);
    src  += 8*3;
    dest += 8;
  }
}


int main(int argc, char *argv[]) {
    // read image
    cv::Mat img = cv::imread("../data/lena.tiff");

    // get image size
    int cols = img.cols;
    int rows = img.rows;

    // check for continuous data in memory
    if (img.isContinuous()) {
      cols = rows*cols;
      rows = 1;
    }

    // convert BGR image to C-readable format for C and NEON code
    uint8_t imageArray[rows*cols*3];
    cv::Vec3b tmp; // Vec3b used to access the 3 channels (BGR) of a pixel
    for (int x = 0; x < img.cols; x++) {
      for (int y = 0; y < img.rows; y++) {
        // accessing input and output without pointer
        tmp = img.at<cv::Vec3b>(cv::Point(x,y));
        imageArray[y*img.cols*3+x*3] = tmp.val[0];
        imageArray[y*img.cols*3+x*3+1] = tmp.val[1];
        imageArray[y*img.cols*3+x*3+2] = tmp.val[2];
      }
    }
    INIT_TIMER

    // convert to grayscale with OpenCV
    cv::Mat imgGray;
    START_TIMER
    cv::cvtColor(img, imgGray, CV_BGR2GRAY);
    STOP_TIMER("Grayscale_OpenCV")

    // convert to grayscale with C-Code
    // create output matrix
    cv::Mat imgGray_c = cv::Mat::zeros(img.rows, img.cols, CV_8U);
    uint8_t imageArray_c[rows*cols];

    START_TIMER
    reference_convert(imageArray_c, imageArray, rows*cols);
    STOP_TIMER("Grayscale_C     ")

    // convert image back to OpenCV format for displaying
    // pointer to input data
    uint8_t *pInput = imageArray_c; // pointer without index
    for (int r = 0; r < rows; r++) {
      // pointer to output data
      uint8_t *pOutput = imgGray_c.ptr<uint8_t>(r); // pointer without index
      for (int c = 0; c < cols; c++) {
        // copy Input to Output
        *pOutput = *pInput;

        // increment data address
        pInput++;
        pOutput++;
      }
    }

    // convert to grayscale with NEON Intrinsics
    // create output matrix
    cv::Mat imgGray_neon = cv::Mat::zeros(img.rows, img.cols, CV_8U);
    uint8_t imageArray_neon[rows*cols];

    START_TIMER
    neon_convert(imageArray_neon, imageArray, rows*cols);
    STOP_TIMER("Grayscale_NEON  ")

    // convert image back to OpenCV format for displaying
    // pointer to input data
    pInput = imageArray_neon;   // pointer without index
    for (int r = 0; r < rows; r++) {
      // pointer to output data
      uint8_t *pOutput = imgGray_neon.ptr<uint8_t>(r);
      for (int c = 0; c < cols; c++) {
        // copy Input to Output
        *pOutput = *pInput; // pointer without index

        // increment data address
        pInput++;
        pOutput++;
      }
    }

    // display images
    cv::imshow("Original Image", img);
    cv::imshow("Grayscale OpenCV", imgGray);
    cv::imshow("Grayscale C", imgGray_c);
    cv::imshow("Grayscale NEON", imgGray_neon);

    //wait for key pressed
    cv::waitKey();

    return 0;
}
