#ifndef THREADSAFEQUEUE_HPP
#define THREADSAFEQUEUE_HPP

#include <queue>
#include <mutex>
#include <condition_variable>

template<typename T>
class ThreadSafeQueue {
public:
  
  void push(const T & value);

  bool empty() const;

  bool try_pop(T& popped_value);

  void wait_and_pop(T& popped_value);

private:

  std::queue<T> the_queue;

  mutable std::mutex the_mutex;

  std::condition_variable the_condition_variable;
};

#endif
