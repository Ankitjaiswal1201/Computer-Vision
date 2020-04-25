### Thresholding ###

## Exercise ##

# In main.cpp you will find 3 functions to implement:
1. void threshold_loop(const cv::Mat &input, cv::Mat &output, int threshold)
2. void threshold_loop_ptr(const cv::Mat &input, cv::Mat &output, int threshold)
3. void threshold_loop_ptr2(const cv::Mat &input, cv::Mat &output, int threshold)

# Function Parameters:
1. const cv::Mat &input: the input image
2. cv::Mat &output: here you have to store the result of your threshold algorithm
3. int threshold: the threshold value

# In each of these functions you will find a comment like this:
1. // insert your code here ...
2. this is the place where you have to insert your code
3. hint: have a look at the slides from the lecture, this can help you

# Implement the threshold functionality in “threshold_loop” by using the Mat::at(int i, int j) method to access the image data

# Implement the threshold functionality in “threshold_loop_ptr” by using a pointer with index to access the image data

# Implement the threshold functionality in “threshold_loop_ptr2” by using a pointer without index (pointer arithmetic) to access the image data
