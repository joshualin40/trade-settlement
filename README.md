A short multi-threading exercise where I learned about the implementation of a Blocking Queue. 

Blocking queues are needed to due the difference in speeds of producer and consumer threads. The producer may produce data faster than the consumer can use it, or vice-versa. 

In our program, we are protecting the resource of each Account, each with its own mutex. 

Bug Record: 
    Issue: Program hangs due to using two lock_guards in settle()
    Why: Two threads can grab one lock and then wait on the other's lock forever set