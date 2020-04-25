# Histogram


Relevant source files:
1. main.cpp: includes the main function and the program structure
2. Histogram.h: declaration of the histogram class
3. Histogram.cpp: implementation of the histogram class
4. PointOperations.h: declaration of the point operations class
5. PointOperations.cpp: implementation of the point operations class
6. Threshold.h: declaration of the threshold class
7. Threshold.cpp: implementation of the threshold class

### Histogram.cpp:
1. Implement the histogram computation in the function “void Histogram::calcHist(const cv::Mat &input, cv::Mat &hist)” :
- “Input” is the input image
- “hist” is for the result of your implementation

2. Implement the “void Histogram::calcStats(const cv::Mat &hist, uchar &min, uchar &max, uchar &mean)” function (calculate the minimum, maximum and mean value of the image):
- hist: input histogram (read the values from this variable)
- min: result for the minimum
- max: result for the maximum
- mean: result for the mean value

3. PointOperations.cpp:
1. Implement the function ”void PointOperations::adjustContrast(cv::Mat &input, cv::Mat &output, float alpha, uchar center)” for contrast adjustment:
- input: input image
- output: output image for the result
- alpha: value for the contrast adjustment
- center: center point for the contrast adjustment

2. Implement the function “void PointOperations::adjustBrightness(cv::Mat &input, cv::Mat &output, int alpha)“ for the brightness adjustment:
- input: input image
- output: output image for the result
- alpha: value for the brightness adjustment

3. Implement the function “void PointOperations::invert(cv::Mat &input, cv::Mat &output)” for the image inversion:
- input: input image
- output: output image for the result

4. Implement the function “void PointOperations::quantize(cv::Mat &input, cv::Mat &output, uchar n)” for the image quantization:
- input: input image
- output: output image for the result
- n: number of quantization bits for the output image
- note: the output image also has 8 bits, think about the bit positions in your “uchar” for a correct image representation.
