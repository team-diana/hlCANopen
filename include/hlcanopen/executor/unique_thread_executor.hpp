#pragma once

#include <list>
#include <mutex>
#include <thread>
#include <folly/Executor.h>
#include <folly/LifoSem.h>
#include <queue>

namespace hlcanopen {

/**
 *  Runs functions each in a unique thread.
 */
class UniqueThreadExecutor : public folly::Executor {
public:
    UniqueThreadExecutor();
    virtual ~UniqueThreadExecutor();

    UniqueThreadExecutor(const UniqueThreadExecutor& t) = delete;

    void add(folly::Func f) override;

private:
    std::queue<folly::Func> workQueue;
    std::mutex mutex;
    std::thread thread;
    bool done = false;
    folly::LifoSem sem;
};

}
