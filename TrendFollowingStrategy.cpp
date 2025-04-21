#include "TrendFollowingStrategy.h"
#include "Utils.h"
#include <iostream>

TrendFollowingStrategy::TrendFollowingStrategy()
:Strategy(), shortMovingAverageWindow(0),longMovingAverageWindow(0){
}

TrendFollowingStrategy::TrendFollowingStrategy(const std::string &name, int shortWindow, int longWindow)
:Strategy(name), shortMovingAverageWindow(shortWindow), longMovingAverageWindow(longWindow){
}

Action TrendFollowingStrategy::decideAction(Market *market, int index, double currentHolding) const
{

    double shortAvg = calculateMovingAverage(market, index, shortMovingAverageWindow);
    double longAvg = calculateMovingAverage(market, index, longMovingAverageWindow);
    
    bool isUptrend = shortAvg > longAvg;
    
    if (isUptrend && currentHolding == 0.0) {
        return BUY;
    } 
    else if (!isUptrend && currentHolding == 1.0) {
        return SELL;
    } 
    else {
        return HOLD;
    }
}

TrendFollowingStrategy **TrendFollowingStrategy::generateStrategySet(const string &baseName, int minShortWindow, int maxShortWindow, int stepShortWindow, int minLongWindow, int maxLongWindow, int stepLongWindow)
{
    int numShortWindows = ((maxShortWindow - minShortWindow) / stepShortWindow) + 1;
    int numLongWindows = ((maxLongWindow - minLongWindow) / stepLongWindow) + 1;
    int arraySize = numShortWindows * numLongWindows;
    
    TrendFollowingStrategy **newTFSArray = new TrendFollowingStrategy*[arraySize];
    
    int index = 0;
    for(int i = minShortWindow; i <= maxShortWindow; i += stepShortWindow){
        for(int j = minLongWindow; j <= maxLongWindow; j += stepLongWindow){
            string nameFormatting = baseName + "_" + to_string(i) + '_' + to_string(j);
            newTFSArray[index++] = new TrendFollowingStrategy(nameFormatting, i, j);
        }
    }
    
    return newTFSArray;
}
