#pragma once

#include <condition_variable>
#include <cstdint>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <unordered_map>

namespace settlement {

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
            if (t.buyer == t.seller) return false; 
            std::scoped_lock lock(lookup[t.buyer]->mu,lookup[t.seller]->mu); // must used scoped lock to prevent deadlock. 
            // it can safetly lock two mutexes with a deadlock avoidance alogirhtm
            if (lookup[t.buyer]->balance < t.amount) return false; 
            lookup[t.buyer]->balance -= t.amount;
            lookup[t.seller]->balance += t.amount;
            return true;
        }
        std::int64_t total() const {
            std::int64_t res = 0;
            for (const auto& [key, value]: lookup)
                res += value->balance;
            return res;
        }
    private:
        std::unordered_map<int, std::unique_ptr<Account>> lookup; // id, Account
};

struct Counter {
    Counter(int settled_, int rejected_) : settled(settled_), rejected(rejected_) {}
    size_t settled;
    size_t rejected;
};

template<typename T>
class BlockingQueue {
    public:
        BlockingQueue() {
            closed = false;
        }
        
        // producer thread
        void push(T item) {
            std::unique_lock<std::mutex> locker(mu); // use unique_lock as it allows for manual locking/unlocking. we want to unlock before notifying worker
            q.push(item);
            locker.unlock();
            // now wake on waiting worker
            cv.notify_one();
        }

        // worker thread 
        std::optional<T> pop() { // wait for an item; std::nullopt once closed AND empty 
            // wait until the q is not empty, releasing lock while waiting
            std::unique_lock<std::mutex> lock(mu);
            if (closed && q.empty()) return std::nullopt; // data race! (if it was on line 80). 
            cv.wait(lock, [this]{ return !q.empty() || closed; }); // access class members by capturing this. // makes the current thread wait until predicate condition is true
            if (q.empty()) return std::nullopt; // closed
            T item = q.front(); 
            q.pop(); 
            lock.unlock(); 
            return item;
        }
        void close() {
            std::unique_lock<std::mutex> lock(mu); // must lock this
            closed = true;
            cv.notify_all(); // wake up all waiting workers
        }
    private:
        std::queue<T> q;
        std::mutex mu;
        std::condition_variable cv;
        bool closed;
};

} // namespace settlement
