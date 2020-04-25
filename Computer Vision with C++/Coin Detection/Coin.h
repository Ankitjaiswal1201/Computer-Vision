#ifndef COIN_H
#define COIN_H

#include <opencv2/core/core.hpp>
#include "Segmentation.h"

enum class CoinColor
{
    Bronze,
    Silver,
    Gold,
    Unknown
};

struct CoinPrototype
{
    // definition of Euro coins
    double value; // Euro
    double diameter; // mm
    CoinColor colorRing;
    CoinColor colorCore;

    // calculated values (depend on the calibration)
    int radius; // pixel
};

class Coin
{
public:
    Coin();
    ~Coin();

    // fuctions that may help for debugging
    std::string colorNameToString(const CoinColor color);
    void showCirleList(std::vector<CircleItem> *circles);

    // functions for coin detections
    double getCoinValue(cv::Mat image, const int circleX, const int circleY, const int circleR);
    void removeOverlappingCircles(std::vector<CircleItem> *circles);
    void setImageNo(int no);
    void setReferenceCoin(cv::Mat image, double value, const int radiusInPixel, const int centerX, const int centerY);
    void setReferenceCoinRadii(double value, const int radiusInPixel);

    // values
    int radiusMinPixel = 0;
    int radiusMaxPixel = 0;

private:
    void getCoinColorBGR(cv::Mat image, const int circleX, const int circleY, const int circleR, float *ringF, float *coreF);
    CoinColor getCoinColorName(const float b, const float g, const float r);
    double getValue(const int radius, const CoinColor colorRing, const CoinColor colorCore);

    std::vector<CoinPrototype> coinList; // list of all Euro coins, sorted by radius

    // BGR color values picked from the reference coin
    float referenceGold[3];
    float referenceSilver[3];

    // radii are rough rounded -> we need a tolerance of some pixel
    int radiusTolerance = 2; // px
    
    int imageNo = 0;
};

#endif /* COIN_H */
