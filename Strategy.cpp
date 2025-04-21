#include "Strategy.h"
#include <iostream>

Strategy::Strategy()
: name(""){  
}

Strategy::Strategy(const string &name)
: name(name){
}

double Strategy::calculateMovingAverage(Market *market, int index, int window) const
{
    
    if (market == nullptr) {
        return 0.0;
    }
    
    if (index < 0 || window <= 0) {
        return market->getPrice(max(0, index));
    }

    double sum = 0.0;
    int startIdx = max(index - window + 1, 0);
    int count = 0;
    
    for (int i = startIdx; i <= index; i++) {
        sum += market->getPrice(i);
        count++;
    }
    
    return count > 0 ? sum / count : market->getPrice(index);
}

string Strategy::getName() const
{
    return name;
}

Strategy::~Strategy()
{
}