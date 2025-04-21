#include "MeanReversionStrategy.h"
#include "Utils.h"
#include <cmath>
#include <iostream>

MeanReversionStrategy::MeanReversionStrategy()
:Strategy(), window(0), threshold(){
}

MeanReversionStrategy::MeanReversionStrategy(const string &name, int window, int threshold)
:Strategy(name), window(window), threshold(threshold){
}

Action MeanReversionStrategy::decideAction(Market *market, int index, double currentHolding) const
{
    double movingAvg = calculateMovingAverage(market, index, window);
    double currentPrice = market->getPrice(index);
    
    double thresholdPercent = threshold / 100.0;
    
    if (currentHolding == 0.0) {
        if (currentPrice < movingAvg * (1.0 - thresholdPercent)) {
            return BUY;
        }
    } else if (currentHolding == 1.0) {
        if (currentPrice > movingAvg * (1.0 + thresholdPercent)) {
            return SELL;
        }
    }
    
    return HOLD;
}

MeanReversionStrategy **MeanReversionStrategy::generateStrategySet(const string &baseName, int minWindow, int maxWindow, int windowStep, int minThreshold, int maxThreshold, int thresholdStep)
{
    int numWindows = ((maxWindow - minWindow) / windowStep) + 1;
    int numThresholds = ((maxThreshold - minThreshold) / thresholdStep) + 1;
    int arraySize = numWindows * numThresholds;
    
    MeanReversionStrategy **newMRSArray = new MeanReversionStrategy*[arraySize];
    
    int index = 0;
    for(int i = minWindow; i <= maxWindow; i += windowStep){
        for(int j = minThreshold; j <= maxThreshold; j += thresholdStep){
            string nameFormatting = baseName + "_" + to_string(i) + '_' + to_string(j);
            newMRSArray[index++] = new MeanReversionStrategy(nameFormatting, i, j);
        }
    }
    
    return newMRSArray;
}