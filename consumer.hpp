#ifndef CONSUMER_HPP
#define CONSUMER_HPP

#include <iostream>

#include "threadsafequeue.hpp"
#include "expression.hpp"
#include "interpreter.hpp"
#include "semantic_error.hpp"

class Consumer
{
public:

  Consumer(ThreadSafeQueue<std::string> *stringQueuePtr, ThreadSafeQueue<Expression> *expressionQueuePtr, Interpreter * interpreter)
  {
    sq = stringQueuePtr;
    eq = expressionQueuePtr;
    interp = interpreter;
  }
  
  void operator()() const
  {
    std::string program;
    sq->wait_and_pop(program);

    std::istringstream expression(program);

    if(!interp->parseStream(expression)){
      Expression exp(Atom("Error: Invalid Expression. Could not parse."));
      eq->push(exp);
    }
    else{
      try{
				Expression exp = interp->evaluate();
				eq->push(exp);
      }
      catch(const SemanticError & ex){
        eq->push(Expression(Atom(ex.what())));
      }
    }
  }

private:
  ThreadSafeQueue<std::string> * sq;
  ThreadSafeQueue<Expression> * eq;
  Interpreter * interp;
};

#endif
