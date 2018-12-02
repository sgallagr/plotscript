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

  Consumer(ThreadSafeQueue<std::string> *programQueuePtr, ThreadSafeQueue<Expression> *expressionQueuePtr,
           Interpreter * interpreter, int * run_flag)
  {
    pq = programQueuePtr;
    expq = expressionQueuePtr;
    interp = interpreter;
    running = run_flag;
  }
  
  void operator()() const
  {
    while(!std::cin.eof()){
      std::string program;
      pq->wait_and_pop(program);

      if (program == "%stop") {
        *running = 0;
        return;
      }

      std::istringstream expression(program);

      if(!interp->parseStream(expression)){
        Expression exp = Expression(Atom("Error"));
        exp.append(Expression(Atom("Error: Invalid Expression. Could not parse.")));
        expq->push(exp);
      }
      else{
        try{
				  Expression exp = interp->evaluate();
				  expq->push(exp);
        }
        catch(const SemanticError & ex){
          Expression exp = Expression(Atom("Error"));
          exp.append(Expression(Atom(ex.what())));
          expq->push(exp);
        }
      }
    }
  }

private:
  ThreadSafeQueue<std::string> * pq;
  ThreadSafeQueue<Expression> * expq;
  Interpreter * interp;
  int * running;
};

#endif
