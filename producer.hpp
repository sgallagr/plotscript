#ifndef PRODUCER_HPP
#define PRODUCER_HPP

#include <iostream>

#include "threadsafequeue.hpp"
#include "expression.hpp"

class Producer
{
public:

  Producer(ThreadSafeQueue<std::string> *programQueuePtr, ThreadSafeQueue<Expression> *expressionQueuePtr)
  {
    pq = programQueuePtr;
    expq = expressionQueuePtr;
  }
  
  void operator()() const
  {
    while(!std::cin.eof()){
    
      std::cout << "\nplotscript> ";

      std::string line;
      std::getline(std::cin, line);
    
      if(line.empty()) continue;

      pq->push(line);

      Expression exp;
      expq->wait_and_pop(exp);

      if(exp.head().asSymbol() == "Error") std::cout << exp.tailConstBegin()->head().asSymbol() << std::endl;
      else std::cout << exp << std::endl;

    }
  }

private:
  ThreadSafeQueue<std::string> * pq;
  ThreadSafeQueue<Expression> * expq;
};

#endif
