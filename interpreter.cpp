#include "interpreter.hpp"

// system includes
#include <stdexcept>
#include <fstream>

// module includes
#include "token.hpp"
#include "parse.hpp"
#include "expression.hpp"
#include "environment.hpp"
#include "startup_config.hpp"

Interpreter::Interpreter(){

  std::ifstream ifs(STARTUP_FILE);

  if(!ifs) error("Could not open startup file for reading.");

  else if(!parseStream(ifs)) error("Invalid startup program. Could not parse.");

  else{
    try{
      evaluate();
    }
    catch(const SemanticError & ex){
      error("Startup program failed during evaluation");
    }	
  }
}

Interpreter::Interpreter(ThreadSafeQueue<std::string> *programQueuePtr, ThreadSafeQueue<Expression> *expressionQueuePtr){

  std::ifstream ifs(STARTUP_FILE);

  if(!ifs) error("Could not open startup file for reading.");

  else if(!parseStream(ifs)) error("Invalid startup program. Could not parse.");

  else{
    try{
      evaluate();
    }
    catch(const SemanticError & ex){
      error("Startup program failed during evaluation");
    }	
  }  

  pq = programQueuePtr;
  expq = expressionQueuePtr;

}

void Interpreter::error(const std::string & err_str){
  std::cerr << "Error: " << err_str << std::endl;
}

bool Interpreter::parseStream(std::istream & expression) noexcept{

  TokenSequenceType tokens = tokenize(expression);

  ast = parse(tokens);

  return (ast != Expression());
};
             

Expression Interpreter::evaluate(){

  return ast.eval(env);
}
