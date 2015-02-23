// Copyright (C) 2015 team-diana MIT license

#ifndef PIPE_HPP
#define PIPE_HPP

#include <queue>
#include <mutex>
#include <memory>
#include <utility>
#include <condition_variable>

template<class T> class Pipe {
  template<class U> friend class BiPipe;

public:
  Pipe() :
  mutex(std::make_unique<std::mutex>()),
  cond(std::make_unique<std::condition_variable>())
  {
  }

  Pipe(const Pipe<T>& oth) = delete;
//   Pipe(Pipe<T>&& oth) = default;
  Pipe(Pipe<T>&& oth) :
    mutex(std::move(oth.mutex)),
    cond(std::move(oth.cond)),
    queue(oth.queue)
  {

  }

  ~Pipe() = default;

  template<class M> void write(M&& m) {
    std::lock_guard<std::mutex> lguard(*mutex);
    queue.push(std::move(m));
    cond->notify_one();
  }

  T read() {
    std::unique_lock<std::mutex> ulock(*mutex);
    cond->wait(ulock, [this] {return queue.size();});
    T m = queue.front();
    queue.pop();
    return m;
  }

private:
  std::unique_ptr<std::mutex> mutex;
  std::unique_ptr<std::condition_variable> cond;
  std::queue<T> queue;
};

#endif // PIPE_HPP
