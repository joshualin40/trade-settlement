A short multi-threading exercise where I learned about the implementation of a Blocking Queue. 

Blocking queues are needed to due the difference in speeds of producer and consumer threads. The producer may produce data faster than the consumer can use it, or vice-versa. 

In our program, we are protecting the resource of each Account, each with its own mutex. 

Bug Record: 
    Issue: Program hangs due to using two lock_guards in settle()
    Why: Two threads can grab one lock and then wait on the other's lock forever set
    Solution: use a std::scoped_lock
    Takeaway: 

    Issue: Program hangs due to scoped_lock holding two of the same mutex (gives UB).
    Why: Causes self deadlock
    Solution: Do the buyer.
    Takeaway: To ensure scoped_lock safety, ensure that no two arguments referring to the same mutex. 


