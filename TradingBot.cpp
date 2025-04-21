#include "TradingBot.h"
#include <limits>

TradingBot::TradingBot(Market *market, int initialCapacity)
: market(market) , availableStrategies(new Strategy*[initialCapacity]),strategyCount(0),strategyCapacity(initialCapacity)
{
    for(int i =0;i< strategyCapacity;i++){
        availableStrategies[i] =nullptr;
    }
}

TradingBot::~TradingBot()
{
    for(int i =0;i< strategyCapacity;i++){
        delete availableStrategies[i];
        availableStrategies[i] =nullptr;
    }
    delete [] availableStrategies;
    availableStrategies = nullptr;
}


void TradingBot::addStrategy(Strategy *strategy)
{
    if (strategy == nullptr) {
        return;
    }

    if(strategyCount==strategyCapacity){
        Strategy **newAvailableStartegies = new Strategy*[strategyCapacity*2];
        int newStartegyCapacity = strategyCapacity*2;
        for(int i =0;i<strategyCount;i++){
            newAvailableStartegies[i] = availableStrategies[i];
        }
        for(int i =strategyCount;i<newStartegyCapacity;i++){
            newAvailableStartegies[i] = nullptr;
        }
        strategyCapacity = newStartegyCapacity;
        delete [] availableStrategies;
        availableStrategies = newAvailableStartegies;
    }
    availableStrategies[strategyCount++] =strategy;
}

SimulationResult TradingBot::runSimulation()
{
    SimulationResult simRes;

    if (market == nullptr || strategyCount == 0 || market->getNumTradingDays() <= 1) {
        return simRes;
    }

    for(int i = 0; i < strategyCount; i++){
        if (availableStrategies[i] == nullptr) {
            continue;
        }

        double profit = 0;
        double currentHolding = 0.0;
        double buyPrice = 0;
        
        int startDay = max(market->getNumTradingDays()-101, 0);
        for(int j = startDay; j < market->getNumTradingDays(); j++){
            if (j < 0 || j >= market->getNumTradingDays()) {
                continue;
            }
            
            Action action = availableStrategies[i]->decideAction(market, j, currentHolding);
            
            if(action == BUY && currentHolding == 0.0){
                buyPrice = market->getPrice(j);
                currentHolding = 1.0;
            } else if(action == SELL && currentHolding == 1.0){
                profit += market->getPrice(j) - buyPrice;
                currentHolding = 0.0;
            }
            
        }
        
        if(currentHolding == 1.0 && market->getNumTradingDays() > 0){
            profit += market->getPrice(market->getNumTradingDays()-1) - buyPrice;
        }
    
        
        if(profit > simRes.totalReturn){
            simRes.bestStrategy = availableStrategies[i];
            simRes.totalReturn = profit; 
        }
    }
    
    return simRes;
}
