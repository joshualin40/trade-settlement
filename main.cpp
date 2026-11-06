#include <iostream> 
#include <mutex> 
#include <unordered_map> 
#include <memory> 
#include <random> 
#include <thread> 

struct Trade {
    int buyer; 
    int seller; 
    std::int64_t amount; 
};

struct Account {
    int id; 
    std::int64_t balance; 
    std::mutex mu; 

    // Constructor: mu is implicitly default-initialized
    Account(int account_id, std::int64_t initial_balance) 
        : id(account_id), balance(initial_balance) {} 
};

// total amount of money must not change

class Ledger {
    public: 
        Ledger(int accounts, std::int64_t initial_balance) {
            for (int i = 0; i < accounts; ++i) {
                lookup[i] = (std::make_unique<Account>(i, initial_balance));
            }
        }
        bool settle(const Trade& t) {
            if (lookup[t.buyer]->balance < t.amount || t.buyer == t.seller) return false; 
            lookup[t.buyer]->balance -= t.amount; 
            lookup[t.seller]->balance += t.amount; 
            return true; 
        }
        std::int64_t total() const {
            std::int64_t res; 
            for (const auto& [key, value]: lookup) 
                res += value->balance; 
            return res; 
        }
    private: 
        std::unordered_map<int, std::unique_ptr<Account>> lookup; // id, Account
};

template<typename T> 
class BlockingQueue {
    public: 
        void push(T item); 
        std::optional<T> pop(); 
        void close(); 
    private: 
        std::deque<T> storage; 
};

void producer(unsigned int seed, BlockingQueue<Trade>& q) {
    std::mt19937 engine(seed); 
    std::uniform_int_distribution<int> account_distrib(0,19);
    std::uniform_int_distribution<int> amount_distrib(1,1'000);

    for (int i = 0; i < 100'000; ++i) { // 100,000 random trades better producer
        int buyer = account_distrib(engine);
        int seller = account_distrib(engine);
        int amount = amount_distrib(engine);
        Trade t{buyer, seller, amount}; 
        q.push(t); 
    }

}
int main() {
    std::random_device rd; 
    unsigned int seed1 = rd(); 
    unsigned int seed2 = rd(); 

    Ledger q(); 
    std::thread p1(producer, seed1, std::ref(q)); 
    std::thread p2(producer, seed2, std::ref(q)); 
}