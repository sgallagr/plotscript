#include "threadsafequeue.hpp"

template<typename T>
bool ThreadSafeQueue<T>::empty() const {
  std::lock_guard<std::mutex> lock(the_mutex);
  return the_queue.empty();
}

template<typename T>
void ThreadSafeQueue<T>::push(const T& value) {
  std::unique_lock<std::mutex> lock(the_mutex);
  the_queue.push(value);
  lock.unlock();
  the_condition_variable.notify_one();
}

template<typename T>
bool ThreadSafeQueue<T>::try_pop(T &popped_value) {
  std::lock_guard<std::mutex> lock(the_mutex);
  if (the_queue.empty()) return false;

  popped_value = the_queue.front();
  the_queue.pop();
  return true;
}

template<typename T>
void ThreadSafeQueue<T>::wait_and_pop(T &popped_value) {
  std::unique_lock<std::mutex> lock(the_mutex);
  while (the_queue.empty()) the_condition_variable.wait(lock);

  popped_value = the_queue.front();
  the_queue.pop();
}
