#include "hlcanopen/executor/unique_thread_executor.hpp"

#include <queue>
#include <memory>
#include <iostream>

namespace hlcanopen {


UniqueThreadExecutor::UniqueThreadExecutor() : done(false)
{
  std::cout << " create unique_thread_executor" << std::endl;
    thread = std::thread(
      [&]() {
          while(!done) {
              sem.wait();
              while(!workQueue.empty()) {
                  workQueue.front()();
                  workQueue.pop();
              }
          }
      }
    );
  std::cout << " created unique_thread_executor" << std::endl;
}

UniqueThreadExecutor::~UniqueThreadExecutor()
{
  std::cout << " destroy unique_thread_executor" << std::endl;
    done = true;
    sem.post();
    thread.join();
}



void UniqueThreadExecutor::add(folly::Func f)
{
    std::lock_guard<std::mutex> lock(mutex);
    workQueue.push(f);
    sem.post();
}


}
