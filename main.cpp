#include "settlement.hpp"
#include <iostream> 
#include <mutex> 
#include <unordered_map> 
#include <memory> 
#include <random> 
#include <thread> 
#include <queue> 
#include <optional> 

using namespace settlement;

void producer(unsigned int seed, BlockingQueue<Trade>& q) {
    std::mt19937 engine(seed); 
    std::uniform_int_distribution<int> account_distrib(0,19);
    std::uniform_int_distribution<int> amount_distrib(1,1'000);

    for (int i = 0; i < 100'000; ++i) { // 100,000 random trades for each producer
        int buyer = account_distrib(engine);
        int seller = account_distrib(engine);
        int amount = amount_distrib(engine);
        Trade t{buyer, seller, amount}; 
        q.push(t); 
    }
}

void consumer(Counter& counter, BlockingQueue<Trade>& q, Ledger& l) {
    auto trade = q.pop(); 
    while (trade.has_value()) {
        if (l.settle(trade.value())) counter.settled += 1;
        else counter.rejected += 1; 
        trade = q.pop();
    }
}
int main() {
    std::cout << "Hello"; 
    std::random_device rd; 
    unsigned int seed1 = rd(); 
    unsigned int seed2 = rd(); 

    BlockingQueue<Trade> q; 
    std::thread p1(producer, seed1, std::ref(q)); 
    std::thread p2(producer, seed2, std::ref(q)); 

    Ledger l(20, 1'000'000); 
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