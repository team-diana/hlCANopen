#include "hlcanopen/executor/unique_thread_executor.hpp"

#include <queue>
#include <memory>

namespace hlcanopen {


UniqueThreadExecutor::UniqueThreadExecutor()
{
  thread = std::thread(
    [&](){
      while(!done) {
        sem.wait();
        while(!workQueue.empty()) {
          workQueue.front()();
          workQueue.pop();
        }
      }
    }
  );
}

UniqueThreadExecutor::~UniqueThreadExecutor()
{
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
