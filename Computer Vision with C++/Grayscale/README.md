### Exercise Question ###

Write the function reference_convert (uint8_t * dest, uint8_t * src, int n) to convert the BGR colored image src 
into a grayscale image dest. dest is a continuous matrix with n pixels where each pixel represents a grayscale 
value (0…255). src is a continuous matrix where each pixel has 3 consecutive values (0…255) for the BGR colors 
(i.e. pixel ordering is (b0, g0, r0, b1, g1, r1 … bn-1, gn-1, rn-1)). When you are finished, the program will show 
the original image, 3 identical grayscale images and the computation time for each of the grayscale conversions.
