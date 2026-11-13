#include "settlement.hpp"
#include <gtest/gtest.h>
#include <thread> 
#include <random> 

using namespace settlement; 

TEST(settlement_test, reject_amount) {
    Ledger l(20, 1'000'000);
    Trade t{1, 2, 1'000'001};
    ASSERT_FALSE(l.settle(t)); 
}

TEST(settlement_test, reject_same_id) {
    Ledger l(20, 1'000'000);
    Trade t{1, 1, 4}; 
    ASSERT_FALSE(l.settle(t));
}

const int ACCOUNT_NUMBERS = 20; 
const int STARTING_BALANCE = 1'000'000; 
const int TRADES_PER_PRODUCER = 100'000; 
TEST(settlement_test, multithreaded_total_unchanged) {
    std::random_device rd; 
    unsigned int seed1 = rd(); 
    unsigned int seed2 = rd(); 

    BlockingQueue<Trade> q; 
    std::thread p1(producer, seed1, std::ref(q), TRADES_PER_PRODUCER); 
    std::thread p2(producer, seed2, std::ref(q), TRADES_PER_PRODUCER); 

    Ledger l(ACCOUNT_NUMBERS, STARTING_BALANCE); 
    std::vector<Counter> counters(4, Counter{0,0});
    std::thread c1(consumer, std::ref(counters[0]), std::ref(q), std::ref(l)); 
    std::thread c3(consumer, std::ref(counters[2]), std::ref(q), std::ref(l)); 
    std::thread c4(consumer, std::ref(counters[3]), std::ref(q), std::ref(l)); 
    std::thread c2(consumer, std::ref(counters[1]), std::ref(q), std::ref(l));  

    p1.join(); // join producers
    p2.join();  

    q.close(); 

    c1.join(); c2.join(); c3.join(); c4.join(); 

    size_t total_rejected{}; size_t total_settled{}; 
    for (int i = 0; i < 4; i++) {
        total_settled += counters[i].settled; 
        total_rejected += counters[i].rejected; 
    }

    ASSERT_EQ(total_settled + total_rejected, 200'000); 
}