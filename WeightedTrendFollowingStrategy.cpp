#include "WeightedTrendFollowingStrategy.h"
#include "Utils.h"
#include <cmath>

WeightedTrendFollowingStrategy::WeightedTrendFollowingStrategy()
:TrendFollowingStrategy(){
}

WeightedTrendFollowingStrategy::WeightedTrendFollowingStrategy(const string &name, int shortWindow, int longWindow)
: TrendFollowingStrategy(name,shortWindow,longWindow){
}

double WeightedTrendFollowingStrategy::calculateExponentialWeight(int index) const
{

    double weight = 1.0;
    double growthFactor = 1.1;
    
    for (int i = 0; i < index; i++) {
        weight *= growthFactor;
    }
    
    return weight;
}

double WeightedTrendFollowingStrategy::calculateMovingAverage(Market *market, int index, int window) const
{
    if (index < 0 || window <= 0) {
        return market->getPrice(max(0, index));
    }

    int startIdx = max(index - window + 1, 0);
    double totalWeight = 0.0;
    double weightedSum = 0.0;

    for (int i = startIdx; i <= index; i++) {
        int position = i - startIdx; 
        
        
        double weight = calculateExponentialWeight(position);
        
        weightedSum += market->getPrice(i) * weight;
        totalWeight += weight;
    }

    if (totalWeight <= 0.0) {
        return market->getPrice(index);
    }

    return weightedSum / totalWeight;
}

WeightedTrendFollowingStrategy **WeightedTrendFollowingStrategy::generateStrategySet(const string &baseName, int minShortWindow, int maxShortWindow, int stepShortWindow, int minLongWindow, int maxLongWindow, int stepLongWindow)
{
    int numShortWindows = ((maxShortWindow - minShortWindow) / stepShortWindow) + 1;
    int numLongWindows = ((maxLongWindow - minLongWindow) / stepLongWindow) + 1;
    int arraySize = numShortWindows * numLongWindows;
    
    WeightedTrendFollowingStrategy **newWTFSArray = new WeightedTrendFollowingStrategy*[arraySize];
    
    int index = 0;
    for(int i = minShortWindow; i <= maxShortWindow; i += stepShortWindow){
        for(int j = minLongWindow; j <= maxLongWindow; j += stepLongWindow){
            string nameFormatting = baseName + "_" + to_string(i) + '_' + to_string(j);
            newWTFSArray[index++] = new WeightedTrendFollowingStrategy(nameFormatting, i, j);
        }
    }
    
    return newWTFSArray;
}
