#include <iostream>
#include <algorithm>
#include "Coin.h"


////////////////////////////////////////////////////////////////////////////////////
// constructor and destructor
////////////////////////////////////////////////////////////////////////////////////
Coin::Coin()
{
    // add Euro coins to list (sorted by diameter)
    CoinPrototype *coin;

    // 1 Euro Cent
    coin = new CoinPrototype();
    coin->value = 0.01;
    coin->diameter = 16.25;
    coin->colorCore = CoinColor::Bronze;
    coin->colorRing = CoinColor::Bronze;
    coinList.push_back(*coin);

    // 2 Euro Cent
    coin = new CoinPrototype();
    coin->value = 0.02;
    coin->diameter = 18.75;
    coin->colorCore = CoinColor::Bronze;
    coin->colorRing = CoinColor::Bronze;
    coinList.push_back(*coin);

    // 10 Euro Cent
    coin = new CoinPrototype();
    coin->value = 0.10;
    coin->diameter = 19.75;
    coin->colorCore = CoinColor::Gold;
    coin->colorRing = CoinColor::Gold;
    coinList.push_back(*coin);

    // 5 Euro Cent
    coin = new CoinPrototype();
    coin->value = 0.05;
    coin->diameter = 21.25;
    coin->colorCore = CoinColor::Bronze;
    coin->colorRing = CoinColor::Bronze;
    coinList.push_back(*coin);

    // 20 Euro Cent
    coin = new CoinPrototype();
    coin->value = 0.20;
    coin->diameter = 22.25;
    coin->colorCore = CoinColor::Gold;
    coin->colorRing = CoinColor::Gold;
    coinList.push_back(*coin);

    // 1 Euro 
    coin = new CoinPrototype();
    coin->value = 1.00;
    coin->diameter = 23.25;
    coin->colorCore = CoinColor::Silver;
    coin->colorRing = CoinColor::Gold;
    coinList.push_back(*coin);

    // 50 Euro Cent
    coin = new CoinPrototype();
    coin->value = 0.50;
    coin->diameter = 24.25;
    coin->colorCore = CoinColor::Gold;
    coin->colorRing = CoinColor::Gold;
    coinList.push_back(*coin);

    // 2 Euro 
    coin = new CoinPrototype();
    coin->value = 2.00;
    coin->diameter = 25.75;
    coin->colorCore = CoinColor::Gold;
    coin->colorRing = CoinColor::Silver;
    coinList.push_back(*coin);
}

Coin::~Coin(){}

////////////////////////////////////////////////////////////////////////////////////
// convert CoinColor to string
////////////////////////////////////////////////////////////////////////////////////
std::string Coin::colorNameToString(const CoinColor color)
{
    switch (color)
    {
    case CoinColor::Bronze:
        return "bronze";
    case CoinColor::Silver:
        return "silver";
    case CoinColor::Gold:
        return "gold";
    }
    return "unknown";
}

////////////////////////////////////////////////////////////////////////////////////
// show the list of circles
////////////////////////////////////////////////////////////////////////////////////
void Coin::showCirleList(std::vector<CircleItem> *circles)
{
    std::cout << "\nlist of circles:\n";
    for (size_t i = 0; i < circles->size(); ++i)
    {
        CircleItem *circle = &circles->at(i);
        std::cout << " circle #" << i << "\tx=" << circle->x << "\ty=" << circle->y << "\tr=" << circle->r << "\tv=" << circle->v << "\n";
    }
}

////////////////////////////////////////////////////////////////////////////////////
// we use a 1 Euro coin as reference to get the size (in pixel) and
// the BGR values of the colors 'gold' and 'silver' from the reference coin
////////////////////////////////////////////////////////////////////////////////////
void Coin::setReferenceCoin(cv::Mat image, double value, const int radiusInPixel, const int centerX, const int centerY)
{
    // set radii (in px) of all euro coins
    setReferenceCoinRadii(value, radiusInPixel);

    // define colors gold and silver based on the colors of the reference coin
    getCoinColorBGR(image, centerX, centerY, radiusInPixel, referenceGold, referenceSilver);
}

////////////////////////////////////////////////////////////////////////////////////
// set the radii of all coins based on the size of the reference coin
////////////////////////////////////////////////////////////////////////////////////
void Coin::setReferenceCoinRadii(double value, const int radiusInPixel)
{
    std::cout << "detectable coins (sorted by radius):\n";
    radiusMinPixel = 2147483647; // (2^31)-1
    radiusMaxPixel = 0;
    
    // get reference coin from coinList
    for (auto coinRef : coinList) 
    {
        if (coinRef.value == value)
        {
            // coin found -> calculate radius in pixel for all coins
            double refRadius = coinRef.diameter / 2.0;
            double rDouble = double(radiusInPixel);
            for (size_t i = 0; i < coinList.size(); ++i)
            {
                CoinPrototype *coin = &coinList[i];
                coin->radius = round(rDouble / refRadius * (coin->diameter / 2.0));
                if (radiusMinPixel > coin->radius)
                    radiusMinPixel = coin->radius;
                if (radiusMaxPixel < coin->radius)
                    radiusMaxPixel = coin->radius;

                std::cout << "\tcoin\tvalue=" << coin->value << " EUR\tradius=" << coin->radius << " px\n";
            }

            // add tolerance
            radiusMinPixel -= radiusTolerance;
            radiusMaxPixel += radiusTolerance;

            if (radiusMinPixel > 0 && radiusMaxPixel > radiusMinPixel)
                return; // no error
        }
    }

    // some error(s) occured
    std::cout << "Error: Reference coin (value=" << value << ") is not in coin list or detected radius is invalid.\n";
    exit(-1);
}

////////////////////////////////////////////////////////////////////////////////////
// get the coin value in Euro (or 0 if it is not a Euro coin)
////////////////////////////////////////////////////////////////////////////////////
double Coin::getCoinValue(cv::Mat image, const int circleX, const int circleY, const int circleR)
{
    // coin (partially) not in image?
    if (circleX < circleR || circleY < circleR || image.cols < circleX + circleR || image.rows < circleY + circleR)
        return 0.0;

    // get colors (ring and core)
    float ring[3];
    float core[3];
    getCoinColorBGR(image, circleX, circleY, circleR, ring, core);

    // compare colors and radius with coins in list and return the value in Euro
    return getValue(circleR, getCoinColorName(ring[0], ring[1], ring[2]), getCoinColorName(core[0], core[1], core[2]));
}

////////////////////////////////////////////////////////////////////////////////////
// get the color values (BGR) of the coin's ring and core
//
// note:
// - a cirlce was found: center (circleX, circleY), radius 'circleR'
// - the radius is not a sufficient criterion to decide which coins was found
// - a Euro coin has eigher 1 or 2 (ring and core) colors
// - 1 and 2 euro coins: max. core radius about 72% of the max. coin radius
// - we create a mask for the ring, overlay it with the color image and every
//   pixel that fits to the mask is used to get the BGR color of the ring or core
////////////////////////////////////////////////////////////////////////////////////
void Coin::getCoinColorBGR(cv::Mat image, const int circleX, const int circleY, const int circleR, float *ringF, float *coreF)
{
    unsigned long int ring[3] = { 0, 0, 0 };
    unsigned long int core[3] = { 0, 0, 0 };
    cv::Scalar colorWhite = cv::Scalar(255, 255, 255);
    
    int thickness = 1 + cvRound(float(circleR) * 0.05f); // thickness >= 1
    int radiusRing = cvRound(float(circleR) * 0.86f) - thickness;
    int radiusCore = cvRound(float(circleR) * 0.36f) - thickness;
    int diameter = circleR * 2;

    // use part of the main image where the reference coin was found
    cv::Mat imgCoin = image(cv::Rect(circleX - circleR, circleY - circleR, diameter, diameter));

    // create masks for ring and core
    cv::Mat maskRing = cv::Mat::zeros(cv::Size(diameter, diameter), CV_8U);
    cv::Mat maskCore = cv::Mat::zeros(cv::Size(diameter, diameter), CV_8U);
    cv::circle(maskRing, cv::Point(circleR, circleR), radiusRing, colorWhite, thickness);
    cv::circle(maskCore, cv::Point(circleR, circleR), radiusCore, colorWhite, thickness);

    for (int row = 0; row < diameter; ++row)
    {
        const uchar *pImage = imgCoin.ptr<uchar>(row);
        const uchar *pMaskRing = maskRing.ptr<uchar>(row);
        const uchar *pMaskCore = maskCore.ptr<uchar>(row);

        for (int col = 0; col < diameter; ++col)
        {
            // check every pixel if it is in the mask
            if (*pMaskCore)
            {
                core[0] += *pImage++;
                core[1] += *pImage++;
                core[2] += *pImage++;
            }
            else if (*pMaskRing)
            {
                ring[0] += *pImage++;
                ring[1] += *pImage++;
                ring[2] += *pImage++;
            }
            else
            {
                pImage += 3;
            }
            pMaskCore++;
            pMaskRing++;
        }
    }

    // scale BGR values to [0, 1]
    float sum = float(ring[0] + ring[1] + ring[2]);
    ringF[0] = float(ring[0]) / sum;
    ringF[1] = float(ring[1]) / sum;
    ringF[2] = float(ring[2]) / sum;

    sum = float(core[0] + core[1] + core[2]);
    coreF[0] = float(core[0]) / sum;
    coreF[1] = float(core[1]) / sum;
    coreF[2] = float(core[2]) / sum;
}

////////////////////////////////////////////////////////////////////////////////////
// estimate pre-defined color based on the BGR values
////////////////////////////////////////////////////////////////////////////////////
CoinColor Coin::getCoinColorName(const float b, const float g, const float r)
{
    float max = std::max({ b, g, r });
    float min = std::min({ b, g, r });
    
    //
    // color calibration (color depend on the image number)
    //
    float refMin = imageNo > 11 ? 0.29 : 0.31;
    float refMax = imageNo > 11 ? 0.37 : 0.35;
    
    if (min > refMin && max < refMax)
        return CoinColor::Silver;

    if (g < b || r < g || b < 0.05)
        return CoinColor::Unknown;

    if (r - g > g - b)
        return CoinColor::Bronze;

    return CoinColor::Gold;
}

////////////////////////////////////////////////////////////////////////////////////
// returns the value in Euro
//
// input: radius, color of the ring, color of the core
////////////////////////////////////////////////////////////////////////////////////
double Coin::getValue(const int radius, const CoinColor colorRing, const CoinColor colorCore)
{
    // radius tolerance
    int rMin = radius - radiusTolerance;
    int rMax = radius + radiusTolerance;

    int bestDiff = rMax; // huge number
    double bestValue = 0.0;

    // find the coin in the list based on the radius (+/- tolerance) and the colors (ring/core)
    for (auto coin : coinList)
    {
        // coinList is sorted by increasing radius
        if (coin.radius > rMax)
            break; // radius in list is already grater then the searched one -> stop searching

        if (coin.radius < rMin || coin.colorRing != colorRing || coin.colorCore != colorCore)
            continue; // radius is to small or colors does not match

        // more than one coin may fulfil the above criteria,
        // therefore, use the coin which fits best
        int diff = radius > coin.radius ? radius - coin.radius : coin.radius - radius;
        if (diff < bestDiff)
        {
            bestDiff = diff;
            bestValue = coin.value;
        }
    }
    return bestValue > 0.0 ? bestValue : -radius;
}

////////////////////////////////////////////////////////////////////////////////////
// removes overlapping circles from the list of cirlces
//
// e.g. the core of the 1 and 2 Euro coin may be detectet as circle
//
// note: criterion is the value from the Hough transformation 
//       (the higher this value the more likely it is a (center of a) circle)
////////////////////////////////////////////////////////////////////////////////////
void Coin::removeOverlappingCircles(std::vector<CircleItem> *circles)
{
    // check circle i against all other circles in the list (j)
    // if circles i and j overlap mark the circle with lesser value (v) from Hough transformation
    for (size_t i = 0; i < circles->size(); ++i)
    {
        CircleItem *circle = &circles->at(i);
        if (circle->v < 0)
            continue;

        for (size_t j = 0; j < circles->size(); ++j)
        {
            if (i == j)
                continue;
            
            CircleItem *check = &circles->at(j);
            if (check->v < 0)
                continue;

            float minDistanceSquared = circle->r + check->r;
            float x2 = circle->x - check->x;
            float y2 = circle->y - check->y;

            // distance (note: there is no need to use sqrt() because we can compare the squares)
            if (x2 * x2 + y2 * y2 < minDistanceSquared * minDistanceSquared)
            {
                // circles overlap -> mark one ot them for deletion
                if (circle->v < check->v)
                    circle->v = -1;
                else
                    check->v = -1;
            }
        }
    }

    // remove marked (value v < 0) circles
    for (size_t i = 0; i < circles->size(); ++i)
    {
        CircleItem *circle = &circles->at(i);
        if (circle->v < 0)
        {
            circles->erase(circles->begin() + i);
            i--; // decrease loop index because we deleted a item
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////
// number of the image
////////////////////////////////////////////////////////////////////////////////////
void Coin::setImageNo(int no)
{
    imageNo = no;
}
