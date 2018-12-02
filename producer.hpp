#ifndef PRODUCER_HPP
#define PRODUCER_HPP

#include <iostream>

#include "threadsafequeue.hpp"
#include "expression.hpp"

class Producer
{
public:

  Producer(ThreadSafeQueue<std::string> *stringQueuePtr, ThreadSafeQueue<Expression> *expressionQueuePtr)
  {
    sq = stringQueuePtr;
    eq = expressionQueuePtr;
  }
  
  void operator()() const
  {
    while(!std::cin.eof()){
    
      std::cout << "\nplotscript> ";

      std::string line;
      std::getline(std::cin, line);
    
      if(line.empty()) continue;

      sq->push(line);

      Expression exp;
      eq->wait_and_pop(exp);
      std::cout << exp << std::endl;
    }
  }

private:
  ThreadSafeQueue<std::string> * sq;
  ThreadSafeQueue<Expression> * eq;
};

#endif
