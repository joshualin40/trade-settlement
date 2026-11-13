#include "settlement.hpp"
#include <iostream> 
#include <mutex> 
#include <unordered_map> 
#include <memory> 
#include <random> 
#include <thread> 
#include <vector> 

using namespace settlement;

const int ACCOUNT_NUMBERS = 20; 
const int STARTING_BALANCE = 1'000'000; 
const int TRADES_PER_PRODUCER = 100'000; 


int main() {
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
    std::cout << "Total Settled: " << total_settled << '\n' << "Total Rejected: " << total_rejected << '\n'; 
    std::cout << "Total: " << l.total();  
}

// std::atomic are faster than wrappiong regular variables in a mutex because they don't use a lock 
// 