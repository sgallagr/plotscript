#include "interpreter.hpp"

// system includes
#include <fstream>

// module includes
#include "token.hpp"
#include "parse.hpp"
#include "expression.hpp"
#include "environment.hpp"
#include "startup_config.hpp"

Interpreter::Interpreter(){

  std::ifstream ifs(STARTUP_FILE);

  parseStream(ifs);
  evaluate();
}

Interpreter::Interpreter(ThreadSafeQueue<std::string> *programQueuePtr, ThreadSafeQueue<Expression> *expressionQueuePtr){

  std::ifstream ifs(STARTUP_FILE);

  parseStream(ifs);
  evaluate();

  pq = programQueuePtr;
  expq = expressionQueuePtr;
}

bool Interpreter::parseStream(std::istream & expression) noexcept{

  TokenSequenceType tokens = tokenize(expression);

  ast = parse(tokens);

  return (ast != Expression());
};
             

Expression Interpreter::evaluate(){

  return ast.eval(env);
}
