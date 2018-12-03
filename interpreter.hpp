/*! \file interpreter.hpp

The interpreter can parse from a stream into an internal AST and evaluate it.
It maintains an environment during evaluation.
 */

#ifndef INTERPRETER_HPP
#define INTERPRETER_HPP

// system includes
#include <istream>
#include <string>
#include <iostream>

// module includes
#include "environment.hpp"
#include "expression.hpp"
#include "threadsafequeue.hpp"
#include "semantic_error.hpp"

/*! \class Interpreter
\brief Class to parse and evaluate an expression (program)

Interpreter has an Environment, which starts at a default.
The parse method builds an internal AST.
The eval method updates Environment and returns last result.
*/
class Interpreter {
public:

  Interpreter();

  Interpreter(ThreadSafeQueue<std::string> *programQueuePtr, ThreadSafeQueue<Expression> *expressionQueuePtr,
              int * run_flag);

  /*! Parse into an internal Expression from a stream
    \param expression the raw text stream repreenting the candidate expression
    \return true on successful parsing 
   */
  bool parseStream(std::istream &expression) noexcept;

  /*! Evaluate the Expression by walking the tree, returning the result.
    \return the Expression resulting from the evaluation in the current environment
    \throws SemanticError when a semantic error is encountered
   */
  Expression evaluate();

  void error(const std::string & err_str);

  void operator()()
  {
    while(running){
      std::string program;
      pq->wait_and_pop(program);

      if (program == "%stop") {
        *running = 0;
        return;
      }

      std::istringstream expression(program);

      if(!parseStream(expression)){
        Expression exp = Expression(Atom("Error"));
        exp.append(Expression(Atom("Error: Invalid Expression. Could not parse.")));
        expq->push(exp);
      }
      else{
        try{
				  Expression exp = evaluate();
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

  // the environment
  Environment env;

  // the AST
  Expression ast;

  ThreadSafeQueue<std::string> * pq;
  ThreadSafeQueue<Expression> * expq;
  int * running;
};

#endif
