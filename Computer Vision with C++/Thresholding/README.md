Thresholding


In main.cpp you will find 3 functions to implement:
a. void threshold_loop(const cv::Mat &input, cv::Mat &output, int threshold)
b. void threshold_loop_ptr(const cv::Mat &input, cv::Mat &output, int threshold)
c. void threshold_loop_ptr2(const cv::Mat &input, cv::Mat &output, int threshold)
5. Function Parameters:
a. const cv::Mat &input: the input image
b. cv::Mat &output: here you have to store the result of your threshold algorithm
c. int threshold: the threshold value
6. In each of these functions you will find a comment like this:
a. // insert your code here ...
b. this is the place where you have to insert your code
c. hint: have a look at the slides from the lecture, this can help you
7. Implement the threshold functionality in “threshold_loop” by using the Mat::at(int i, int j) method to access the image data
8. Implement the threshold functionality in “threshold_loop_ptr” by using a pointer with index to access the image data
9. Implement the threshold functionality in “threshold_loop_ptr2” by using a pointer without index (pointer arithmetic) to access the image data
