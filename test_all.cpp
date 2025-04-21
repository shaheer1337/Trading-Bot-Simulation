#include <iostream>
#include <cassert>
#include <cmath>
#include <vector>
#include <string>
#include <stdexcept>
#include <limits>
#include <chrono>
#include <climits>  // For INT_MAX

#include "Market.h"
#include "Strategy.h"
#include "TradingBot.h"
#include "MeanReversionStrategy.h"
#include "TrendFollowingStrategy.h"
#include "WeightedTrendFollowingStrategy.h"
#include "Utils.h"

using namespace std;

// Helper function to check if two doubles are approximately equal
bool areEqual(double a, double b, double epsilon = 1e-6) {
    return fabs(a - b) < epsilon;
}

// Helper function to check memory leaks by ensuring proper deletion
void testMemoryManagement() {
    cout << "\n=== TESTING MEMORY MANAGEMENT ===\n";
    
    // Test Market memory management
    {
        Market* market = new Market(100.0, 0.2, 0.5, 10, 42);
        market->simulate();
        delete market; // Should clean up all dynamically allocated memory
        cout << "- Market object deletion successful\n";
    }
    
    // Test Strategy hierarchy memory management
    {
        Strategy* mrs = new MeanReversionStrategy("MR_Test", 10, 5);
        delete mrs; // Should call virtual destructor correctly
        cout << "- MeanReversionStrategy deletion through base pointer successful\n";
        
        Strategy* tfs = new TrendFollowingStrategy("TF_Test", 5, 10);
        delete tfs; // Should call virtual destructor correctly
        cout << "- TrendFollowingStrategy deletion through base pointer successful\n";
        
        Strategy* wtfs = new WeightedTrendFollowingStrategy("WTF_Test", 5, 10);
        delete wtfs; // Should call virtual destructor correctly
        cout << "- WeightedTrendFollowingStrategy deletion through base pointer successful\n";
    }
    
    // Test TradingBot memory management
    {
        Market* market = new Market(100.0, 0.2, 0.5, 10, 42);
        TradingBot* bot = new TradingBot(market);
        
        // Add strategies that will be owned and deleted by the bot
        bot->addStrategy(new MeanReversionStrategy("MR_1", 10, 5));
        bot->addStrategy(new TrendFollowingStrategy("TF_1", 5, 10));
        bot->addStrategy(new WeightedTrendFollowingStrategy("WTF_1", 5, 10));
        
        delete bot; // Should delete all strategies
        delete market; // Delete market separately
        cout << "- TradingBot with multiple strategies deletion successful\n";
    }
    
    // Test array memory management
    {
        MeanReversionStrategy** mrArray = MeanReversionStrategy::generateStrategySet("MR", 5, 10, 5, 1, 5, 1);
        // Calculate array size
        int numWindows = ((10 - 5) / 5) + 1;
        int numThresholds = ((5 - 1) / 1) + 1;
        int arraySize = numWindows * numThresholds;
        
        // Delete the array properly
        for (int i = 0; i < arraySize; i++) {
            delete mrArray[i];
        }
        delete[] mrArray;
        cout << "- MeanReversionStrategy array deletion successful\n";
        
        TrendFollowingStrategy** tfArray = TrendFollowingStrategy::generateStrategySet("TF", 5, 10, 5, 10, 20, 10);
        // Calculate array size
        numWindows = ((10 - 5) / 5) + 1;
        int numLongWindows = ((20 - 10) / 10) + 1;
        arraySize = numWindows * numLongWindows;
        
        // Delete the array properly
        for (int i = 0; i < arraySize; i++) {
            delete tfArray[i];
        }
        delete[] tfArray;
        cout << "- TrendFollowingStrategy array deletion successful\n";
    }
}

// Test Market class functionality and edge cases
void testMarket() {
    cout << "\n=== TESTING MARKET CLASS ===\n";
    
    // Test constructor with valid parameters
    Market market1(100.0, 0.2, 0.5, 252, 42);
    assert(areEqual(market1.getVolatility(), 0.2));
    assert(areEqual(market1.getExpectedYearlyReturn(), 0.5));
    assert(market1.getNumTradingDays() == 252);
    cout << "- Market constructor with valid parameters works\n";
    
    // Test constructor with edge case parameters
    Market market2(0.0, 0.0, 0.0, 1, 0);
    assert(areEqual(market2.getVolatility(), 0.0));
    assert(areEqual(market2.getExpectedYearlyReturn(), 0.0));
    assert(market2.getNumTradingDays() == 1);
    cout << "- Market constructor with zero parameters works\n";
    
    // Test simulation
    market1.simulate();
    assert(*market1.getPrices()[0] > 0); // First price should be positive
    cout << "- Market simulation works\n";
    
    // Test getPrice with valid index
    double price = market1.getPrice(0);
    assert(price > 0);
    cout << "- getPrice with valid index works\n";
    
    // Test getLastPrice
    double lastPrice = market1.getLastPrice();
    assert(lastPrice == market1.getPrice(market1.getNumTradingDays() - 1));
    cout << "- getLastPrice works\n";
    
    // Test file operations
    market1.writeToFile("test_market.txt");
    Market loadedMarket("test_market.txt");
    assert(loadedMarket.getNumTradingDays() == market1.getNumTradingDays());
    assert(areEqual(loadedMarket.getVolatility(), market1.getVolatility()));
    assert(areEqual(loadedMarket.getExpectedYearlyReturn(), market1.getExpectedYearlyReturn()));
    cout << "- File operations work\n";
}

// Test Strategy class functionality and edge cases
void testStrategy() {
    cout << "\n=== TESTING STRATEGY CLASSES ===\n";
    
    // Create a market for testing
    Market market(100.0, 0.2, 0.5, 100, 42);
    market.simulate();
    
    // Test MeanReversionStrategy
    {
        // Test with normal parameters
        MeanReversionStrategy mrs1("MR_Test", 10, 5);
        assert(mrs1.getName() == "MR_Test");
        
        // Test decideAction with various scenarios
        double avgPrice = 0;
        for (int i = 0; i < 10; i++) {
            avgPrice += market.getPrice(i);
        }
        avgPrice /= 10;
        
        // Test buy condition (price below average by threshold)
        double buyThreshold = avgPrice * (1.0 - 5/100.0);
        double sellThreshold = avgPrice * (1.0 + 5/100.0);
        
        // Log the strategy behavior
        cout << "- MeanReversionStrategy behavior check:\n";
        cout << "  Average price: " << avgPrice << "\n";
        cout << "  Buy threshold: " << buyThreshold << "\n";
        cout << "  Sell threshold: " << sellThreshold << "\n";
        
        // Test edge case with zero window
        MeanReversionStrategy mrs2("MR_Zero", 0, 5);
        Action action = mrs2.decideAction(&market, 0, 0.0);
        cout << "  Action with zero window: " << action << "\n";
        
        // Test edge case with zero threshold
        MeanReversionStrategy mrs3("MR_ZeroThreshold", 10, 0);
        action = mrs3.decideAction(&market, 10, 0.0);
        cout << "  Action with zero threshold: " << action << "\n";
        
        // Test generateStrategySet
        MeanReversionStrategy** strategies = MeanReversionStrategy::generateStrategySet("MR", 5, 10, 5, 1, 5, 1);
        
        // Calculate expected array size
        int numWindows = ((10 - 5) / 5) + 1;
        int numThresholds = ((5 - 1) / 1) + 1;
        int arraySize = numWindows * numThresholds;
        
        for (int i = 0; i < arraySize; i++) {
            assert(strategies[i] != nullptr);
            assert(strategies[i]->getName().find("MeanReversion_") != string::npos);
            delete strategies[i]; // Clean up
        }
        delete[] strategies;
        cout << "- MeanReversionStrategy generateStrategySet works\n";
    }
    
    // Test TrendFollowingStrategy
    {
        // Test with normal parameters
        TrendFollowingStrategy tfs1("TF_Test", 5, 10);
        assert(tfs1.getName() == "TF_Test");
        
        // Test decideAction with various scenarios
        cout << "- TrendFollowingStrategy behavior check:\n";
        double shortAvg = 0, longAvg = 0;
        for (int i = 0; i < 5; i++) {
            shortAvg += market.getPrice(i);
        }
        shortAvg /= 5;
        
        for (int i = 0; i < 10; i++) {
            longAvg += market.getPrice(i);
        }
        longAvg /= 10;
        
        cout << "  Short average (5 days): " << shortAvg << "\n";
        cout << "  Long average (10 days): " << longAvg << "\n";
        
        // Test edge case with equal window sizes
        TrendFollowingStrategy tfs2("TF_Equal", 10, 10);
        Action action = tfs2.decideAction(&market, 10, 0.0);
        cout << "  Action with equal window sizes: " << action << "\n";
        
        // Test edge case with zero window
        TrendFollowingStrategy tfs3("TF_Zero", 0, 10);
        action = tfs3.decideAction(&market, 10, 0.0);
        cout << "  Action with zero short window: " << action << "\n";
        
        // Test generateStrategySet
        TrendFollowingStrategy** strategies = TrendFollowingStrategy::generateStrategySet("TF", 5, 10, 5, 10, 20, 10);
        
        // Calculate expected array size
        int numShortWindows = ((10 - 5) / 5) + 1;
        int numLongWindows = ((20 - 10) / 10) + 1;
        int arraySize = numShortWindows * numLongWindows;
        
        for (int i = 0; i < arraySize; i++) {
            assert(strategies[i] != nullptr);
            assert(strategies[i]->getName().find("Trend_") != string::npos);
            delete strategies[i]; // Clean up
        }
        delete[] strategies;
        cout << "- TrendFollowingStrategy generateStrategySet works\n";
    }
    
    // Test WeightedTrendFollowingStrategy
    {
        // Test with normal parameters
        WeightedTrendFollowingStrategy wtfs1("WTF_Test", 5, 10);
        assert(wtfs1.getName() == "WTF_Test");
        
        // Test calculateMovingAverage
        double weightedAvg = wtfs1.calculateMovingAverage(&market, 10, 5);
        assert(weightedAvg > 0);
        
        // Test edge cases
        double edgeCaseAvg = wtfs1.calculateMovingAverage(&market, 0, 5); // Start of data
        assert(edgeCaseAvg > 0);
        
        edgeCaseAvg = wtfs1.calculateMovingAverage(&market, 10, 0); // Zero window
        assert(edgeCaseAvg > 0);
        
        cout << "- WeightedTrendFollowingStrategy calculateMovingAverage works\n";
        
        // Test generateStrategySet
        WeightedTrendFollowingStrategy** strategies = WeightedTrendFollowingStrategy::generateStrategySet("WTF", 5, 10, 5, 10, 20, 10);
        
        // Calculate expected array size
        int numShortWindows = ((10 - 5) / 5) + 1;
        int numLongWindows = ((20 - 10) / 10) + 1;
        int arraySize = numShortWindows * numLongWindows;
        
        for (int i = 0; i < arraySize; i++) {
            assert(strategies[i] != nullptr);
            assert(strategies[i]->getName().find("WeightedTrendFollowing_") != string::npos);
            delete strategies[i]; // Clean up
        }
        delete[] strategies;
        cout << "- WeightedTrendFollowingStrategy generateStrategySet works\n";
    }
}

// Test TradingBot functionality and edge cases
void testTradingBot() {
    cout << "\n=== TESTING TRADING BOT ===\n";
    
    // Create a market for testing
    Market* market = new Market(100.0, 0.2, 0.5, 100, 42);
    market->simulate();
    
    // Test constructor
    TradingBot bot(market);
    cout << "- TradingBot constructor works\n";
    
    // Test adding strategies
    bot.addStrategy(new MeanReversionStrategy("MR_1", 10, 5));
    bot.addStrategy(new TrendFollowingStrategy("TF_1", 5, 10));
    bot.addStrategy(new WeightedTrendFollowingStrategy("WTF_1", 5, 10));
    cout << "- addStrategy works\n";
    
    // Test running simulation
    SimulationResult result = bot.runSimulation();
    assert(result.bestStrategy != nullptr);
    assert(result.totalReturn > -std::numeric_limits<double>::max());
    cout << "- runSimulation works\n";
    cout << "  Best strategy: " << result.bestStrategy->getName() << "\n";
    cout << "  Total return: " << result.totalReturn << "\n";
    
    // Test edge case: no strategies
    TradingBot emptyBot(market);
    result = emptyBot.runSimulation();
    assert(result.bestStrategy == nullptr);
    assert(result.totalReturn == -std::numeric_limits<double>::max());
    cout << "- runSimulation with no strategies works\n";
    
    // Test adding many strategies (capacity expansion)
    for (int i = 0; i < 20; i++) {
        emptyBot.addStrategy(new MeanReversionStrategy("MR_" + to_string(i), 10, 5));
    }
    result = emptyBot.runSimulation();
    assert(result.bestStrategy != nullptr);
    cout << "- Adding many strategies (capacity expansion) works\n";
    
    // Clean up
    delete market;
}

// Test performance and timing
void testPerformance() {
    cout << "\n=== TESTING PERFORMANCE ===\n";
    
    // Create a large market for testing
    const int LARGE_SIZE = 1000;
    Market* market = new Market(100.0, 0.2, 0.5, LARGE_SIZE, 42);
    
    // Time market simulation
    auto start = chrono::high_resolution_clock::now();
    market->simulate();
    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> duration = end - start;
    cout << "- Market simulation for " << LARGE_SIZE << " days took " << duration.count() << " seconds\n";
    
    // Time strategy evaluation
    TradingBot bot(market);
    
    // Add multiple strategies
    for (int i = 0; i < 5; i++) {
        bot.addStrategy(new MeanReversionStrategy("MR_" + to_string(i), 10 + i*5, 5));
        bot.addStrategy(new TrendFollowingStrategy("TF_" + to_string(i), 5 + i*2, 10 + i*5));
        bot.addStrategy(new WeightedTrendFollowingStrategy("WTF_" + to_string(i), 5 + i*2, 10 + i*5));
    }
    
    start = chrono::high_resolution_clock::now();
    SimulationResult result = bot.runSimulation();
    end = chrono::high_resolution_clock::now();
    duration = end - start;
    cout << "- Trading simulation with " << 15 << " strategies took " << duration.count() << " seconds\n";
    cout << "  Best strategy: " << result.bestStrategy->getName() << "\n";
    
    // Clean up
    delete market;
}

// Comprehensive test to check for potential issues
void testComprehensive() {
    cout << "\n=== COMPREHENSIVE TESTING ===\n";
    
    // Test all market files
    vector<string> marketFiles = {"bullish_low_vol.txt", "bullish_high_vol.txt", "bearish_low_vol.txt", "bearish_high_vol.txt"};
    
    for (const auto& file : marketFiles) {
        try {
            cout << "- Testing with market file: " << file << "\n";
            Market* market = new Market(0, 0, 0, TRADING_DAYS_PER_YEAR, 999);
            market->loadFromFile(file);
            
            TradingBot bot(market);
            
            // Add various strategies
            bot.addStrategy(new MeanReversionStrategy("MR_1", 10, 5));
            bot.addStrategy(new MeanReversionStrategy("MR_2", 15, 10));
            bot.addStrategy(new MeanReversionStrategy("MR_3", 5, 50));
            
            bot.addStrategy(new TrendFollowingStrategy("TF_1", 10, 15));
            bot.addStrategy(new TrendFollowingStrategy("TF_2", 20, 25));
            bot.addStrategy(new TrendFollowingStrategy("TF_3", 15, 25));
            
            bot.addStrategy(new WeightedTrendFollowingStrategy("WTF_1", 10, 15));
            bot.addStrategy(new WeightedTrendFollowingStrategy("WTF_2", 20, 25));
            bot.addStrategy(new WeightedTrendFollowingStrategy("WTF_3", 15, 25));
            
            // Run simulation
            SimulationResult result = bot.runSimulation();
            cout << "  Best strategy: " << result.bestStrategy->getName() << "\n";
            cout << "  Total return: " << result.totalReturn << "\n";
            
            delete market;
        } catch (const exception& e) {
            cout << "  ERROR: " << e.what() << "\n";
        }
    }
}

// Test automatic memory management through scope exit
void testAutomaticMemoryManagement() {
    cout << "\n=== TESTING AUTOMATIC MEMORY MANAGEMENT ===\n";
    
    // Test automatic cleanup of Market objects on scope exit
    {
        cout << "- Creating Market object in nested scope\n";
        {
            Market market(100.0, 0.2, 0.5, 252, 42);
            market.simulate();
            cout << "  Market object created and will go out of scope\n";
            // Market destructor should be called automatically when this scope ends
        }
        cout << "- Market object should have been destroyed\n";
    }
    
    // Test automatic cleanup of Strategy objects on scope exit
    {
        cout << "- Creating Strategy objects in nested scope\n";
        {
            MeanReversionStrategy mrs("MR_AutoTest", 10, 5);
            TrendFollowingStrategy tfs("TF_AutoTest", 5, 10);
            WeightedTrendFollowingStrategy wtfs("WTF_AutoTest", 5, 10);
            cout << "  Strategy objects created and will go out of scope\n";
            // Strategy destructors should be called automatically when this scope ends
        }
        cout << "- Strategy objects should have been destroyed\n";
    }
    
    // Test automatic cleanup of TradingBot with strategies on scope exit
    {
        cout << "- Creating TradingBot with strategies in nested scope\n";
        {
            Market market(100.0, 0.2, 0.5, 100, 42);
            market.simulate();
            
            TradingBot bot(&market);
            bot.addStrategy(new MeanReversionStrategy("MR_Auto", 10, 5));
            bot.addStrategy(new TrendFollowingStrategy("TF_Auto", 5, 10));
            bot.addStrategy(new WeightedTrendFollowingStrategy("WTF_Auto", 5, 10));
            
            cout << "  TradingBot with strategies created and will go out of scope\n";
            // TradingBot destructor should clean up all strategies when this scope ends
        }
        cout << "- TradingBot and strategies should have been destroyed\n";
    }
    
    // Test automatic cleanup of complex object hierarchies
    {
        cout << "- Creating complex strategy array hierarchy in nested scope\n";
        {
            // Create arrays of strategies
            MeanReversionStrategy** mrArray = MeanReversionStrategy::generateStrategySet("MR", 5, 10, 5, 1, 5, 1);
            TrendFollowingStrategy** tfArray = TrendFollowingStrategy::generateStrategySet("TF", 5, 10, 5, 10, 20, 10);
            
            // Calculate array sizes
            int mrSize = ((10 - 5) / 5 + 1) * ((5 - 1) / 1 + 1);
            int tfSize = ((10 - 5) / 5 + 1) * ((20 - 10) / 10 + 1);
            
            // Create a market and trading bot
            Market market(100.0, 0.2, 0.5, 100, 42);
            market.simulate();
            
            TradingBot bot(&market);
            
            // Add some strategies from the arrays to the bot
            bot.addStrategy(mrArray[0]);
            bot.addStrategy(tfArray[0]);
            
            // Clean up remaining strategies that weren't added to the bot
            for (int i = 1; i < mrSize; i++) {
                delete mrArray[i];
            }
            for (int i = 1; i < tfSize; i++) {
                delete tfArray[i];
            }
            
            // Clean up the arrays themselves
            delete[] mrArray;
            delete[] tfArray;
            
            cout << "  Complex object hierarchy created and will go out of scope\n";
            // When this scope ends, the bot should clean up the strategies it owns,
            // and the market should clean up its prices
        }
        cout << "- Complex object hierarchy should have been destroyed\n";
    }
}

// Test for undefined behavior scenarios
void testUndefinedBehavior() {
    cout << "\n=== TESTING PROTECTION AGAINST UNDEFINED BEHAVIOR ===\n";
    
    // Create test market
    Market market(100.0, 0.2, 0.5, 10, 42);
    market.simulate();
    
    // Test proper handling of edge cases
    try {
        // Test bounds checking in the Market class
        cout << "- Testing bounds checking in Market class\n";
        
        // This should be safe - valid index
        double price = market.getPrice(0);
        cout << "  Successfully retrieved price at index 0: " << price << "\n";
        
        // This should be safe - last valid index
        price = market.getPrice(market.getNumTradingDays() - 1);
        cout << "  Successfully retrieved price at last index: " << price << "\n";
        
        /* Uncomment to test if your implementation handles invalid indices
        // This would be out of bounds in an unsafe implementation
        price = market.getPrice(market.getNumTradingDays());
        cout << "  Warning: Successfully retrieved price beyond array bounds\n";
        */
    } catch (const exception& e) {
        cout << "  Exception caught (expected for bounds checking): " << e.what() << "\n";
    }
    
    // Test division by zero protection
    try {
        cout << "- Testing division by zero protection\n";
        
        // Test window of zero in moving average calculations
        Strategy* strategy = new MeanReversionStrategy("DivTest", 0, 5);
        double avg = strategy->calculateMovingAverage(&market, 5, 0);
        cout << "  Successfully handled zero window in moving average: " << avg << "\n";
        delete strategy;
    } catch (const exception& e) {
        cout << "  Exception caught (might be expected): " << e.what() << "\n";
    }
    
    // Test potential integer overflow cases
    cout << "- Testing potential integer overflow scenarios\n";
    try {
        // Very large window size
        int largeWindow = INT_MAX;
        Strategy* strategy = new MeanReversionStrategy("OverflowTest", 10, 5);
        double avg = strategy->calculateMovingAverage(&market, 5, largeWindow);
        assert(avg >= 0.0); // Add assertion to use the variable
        cout << "  Successfully handled very large window size\n";
        delete strategy;
    } catch (const exception& e) {
        cout << "  Exception caught (might be expected): " << e.what() << "\n";
    }
    
    // Test null pointer handling
    cout << "- Testing null pointer handling\n";
    try {
        Strategy* nullStrategy = nullptr;
        // Don't actually call methods on null - just test your code's null checks
        if (nullStrategy == nullptr) {
            cout << "  Null pointer detected properly\n";
        }
        
        // Test TradingBot with null strategy
        TradingBot bot(&market);
        // Uncomment to test if your implementation handles null strategies
        // bot.addStrategy(nullptr);
        
        cout << "  Successfully handled null pointer check\n";
    } catch (const exception& e) {
        cout << "  Exception caught (might be expected): " << e.what() << "\n";
    }
}


// This function specifically tests for use-after-free scenarios
// void testUseAfterFree() {
//     cout << "\n=== TESTING FOR USE-AFTER-FREE VULNERABILITIES ===\n";
    
//     // Test 1: Strategy use after deletion
//     cout << "Test 1: Strategy use after deletion\n";
//     {
//         MeanReversionStrategy* strategy = new MeanReversionStrategy("Test", 10, 5);
//         Market* market = new Market(100.0, 0.2, 0.1, 10, 42);
//         market->simulate();
        
//         // Store the strategy pointer, then delete it
//         Strategy* storedStrategy = strategy;
//         delete strategy;
        
//         // Use a try-catch to detect the use-after-free
//         try {
//             // This should crash if use-after-free detection is working
//             // We put this in a try-catch to handle the crash gracefully in testing
//             double dummy = storedStrategy->calculateMovingAverage(market, 5, 3);
//             cout << "  FAIL: Successfully used strategy after deletion: " << dummy << endl;
//         } catch (...) {
//             cout << "  PASS: Detected attempt to use deleted strategy\n";
//         }
        
//         delete market;
//     }
    
//     // Test 2: Market use after deletion
//     cout << "Test 2: Market use after deletion\n";
//     {
//         Market* market = new Market(100.0, 0.2, 0.1, 10, 42);
//         market->simulate();
//         TrendFollowingStrategy* strategy = new TrendFollowingStrategy("Test", 5, 10);
        
//         // Store prices pointer before deletion
//         double** pricesBeforeDeletion = market->getPrices();
//         delete market;
        
//         // Use a try-catch to detect the use-after-free
//         try {
//             // This should crash if use-after-free detection is working
//             strategy->decideAction(nullptr, 0, 0); // Just to avoid unused variable warning
//             double dummy = *pricesBeforeDeletion[0]; // This should trigger use-after-free
//             cout << "  FAIL: Successfully accessed market data after deletion: " << dummy << endl;
//         } catch (...) {
//             cout << "  PASS: Detected attempt to use deleted market data\n";
//         }
        
//         delete strategy;
//     }
    
//     // Test 3: TradingBot double deletion of strategies
//     cout << "Test 3: TradingBot double deletion of strategies\n";
//     {
//         Market* market = new Market(100.0, 0.2, 0.1, 10, 42);
//         market->simulate();
//         TradingBot* bot = new TradingBot(market); // Fixed: Pass market to constructor
        
//         // Create strategy and add to bot
//         TrendFollowingStrategy* strategy = new TrendFollowingStrategy("Test", 5, 10);
//         bot->addStrategy(strategy);
        
//         // Now try to delete the strategy directly (double deletion scenario)
//         // This is dangerous because the bot will try to delete it later
//         try {
//             delete strategy; // This creates a double-deletion scenario
//             cout << "  First deletion succeeded\n";
            
//             // Now let the bot try to delete it too
//             delete bot; // This should crash on double-free
//             cout << "  FAIL: Double deletion didn't cause an error\n";
//         } catch (...) {
//             cout << "  PASS: Detected double deletion attempt\n";
//         }
        
//         // Clean up the market if we get here
//         delete market;
//     }
    
//     // Test 4: Check for dangling references when removing strategies
//     cout << "Test 4: Check for dangling references when removing strategies\n";
//     {
//         Market* market = new Market(100.0, 0.2, 0.1, 10, 42);
//         market->simulate();
//         TradingBot* bot = new TradingBot(market); // Fixed: Pass market to constructor
        
//         // Add multiple strategies
//         Strategy* strategy1 = new MeanReversionStrategy("Test1", 10, 5);
//         Strategy* strategy2 = new TrendFollowingStrategy("Test2", 5, 10);
//         Strategy* strategy3 = new WeightedTrendFollowingStrategy("Test3", 3, 7);
        
//         bot->addStrategy(strategy1);
//         bot->addStrategy(strategy2);
//         bot->addStrategy(strategy3);
        
//         // Remove the middle strategy (if removeStrategy exists)
//         try {
//             // This assumes you have a removeStrategy method
//             // If you don't, this test will be skipped
//             // bot->removeStrategy(strategy2);
            
//             // Run a simulation to ensure all strategies are accessed
//             bot->runSimulation();
//             cout << "  PASS: No crashes after strategy removal\n";
//         } catch (...) {
//             cout << "  INCONCLUSIVE: Could not test strategy removal (method may not exist)\n";
//         }
        
//         delete bot;
//         delete market;
//     }
    
//     cout << "Use-after-free testing completed.\n";
// }

int main() {
    cout << "=================================================================\n";
    cout << "COMPREHENSIVE TESTING SUITE FOR TRADING STRATEGY IMPLEMENTATION\n";
    cout << "=================================================================\n";
    
    try {
        // Run all tests
        testMemoryManagement();
        testAutomaticMemoryManagement();
        testMarket();
        testStrategy();
        testTradingBot();
        testPerformance();
        testUndefinedBehavior();
        // testUseAfterFree();
        testComprehensive();
        
        // Uncomment the next line ONLY to verify leak detection tools
        // testDeliberateLeak();
        
        cout << "\n=== ALL TESTS COMPLETED SUCCESSFULLY ===\n";
    } catch (const exception& e) {
        cout << "\nTEST FAILED: " << e.what() << "\n";
        return 1;
    } catch (...) {
        cout << "\nTEST FAILED: Unknown exception\n";
        return 1;
    }
    
    return 0;
} 