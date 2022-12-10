#ifndef SEMAPHORE_H7598278632
#define SEMAPHORE_H7598278632

#include <mutex>
#include <condition_variable>

class semaphore {
    std::mutex mutex_;
    std::condition_variable condition_;
    unsigned long count_ = 0; // Initialized as locked.

public:
    void release();

    void acquire();

    bool try_acquire();
};
#endif //SEMAPHORE_H7598278632