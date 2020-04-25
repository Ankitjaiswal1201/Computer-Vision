#include <stdarg.h>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

///////////////////////////////////////////////////////////////////////////////
// show multiple images in one single window
///////////////////////////////////////////////////////////////////////////////
void imshow_multiple(const cv::string &winname, int nArgs, ...)
{
    // check number of images to show
    if(nArgs <= 0) {
        printf("No images specified\n");
        return;
    }
    else if(nArgs > 5) {
        printf("Too many images spacified\n");
        return;
    }

    // get the first image and its dimensions
    va_list args;
    va_start(args, nArgs);
    cv::Mat *pImg = va_arg(args, cv::Mat*);
    int rows = pImg->rows;
    int cols = pImg->cols;
    va_end(args);

    // create new big image to display all the others in a row
    cv::Mat imgMultiple(rows, cols * nArgs, pImg->type());

    // loop through all images and copy to the big one
    va_start(args, nArgs);

    for (int n = 0; n < nArgs; ++n)
    {
        pImg = va_arg(args, cv::Mat*);
        cv::Rect ROI(cols * n, 0, cols, rows);
        pImg->copyTo(imgMultiple(ROI));
    }

    // show the newly created image
    cv::imshow(winname, imgMultiple);

    va_end(args);
}
