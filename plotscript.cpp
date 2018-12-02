#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <thread>

#include "interpreter.hpp"
#include "semantic_error.hpp"
#include "startup_config.hpp"
#include "threadsafequeue.hpp"
#include "producer.hpp"
#include "consumer.hpp"

void prompt(){
  std::cout << "\nplotscript> ";
}

std::string readline(){
  std::string line;
  std::getline(std::cin, line);

  return line;
}

void error(const std::string & err_str){
  std::cerr << "Error: " << err_str << std::endl;
}

void info(const std::string & err_str){
  std::cout << "Info: " << err_str << std::endl;
}

int eval_from_stream(std::istream & stream, Interpreter & interp){
  
  if(!interp.parseStream(stream)){
    error("Invalid Program. Could not parse.");
    return EXIT_FAILURE;
  }
  else{
    try{
      Expression exp = interp.evaluate();
      std::cout << exp << std::endl;
    }
    catch(const SemanticError & ex){
      std::cerr << ex.what() << std::endl;
      return EXIT_FAILURE;
    }	
  }

  return EXIT_SUCCESS;
}

int eval_from_file(std::string filename, Interpreter & interp){
      
  std::ifstream ifs(filename);
  
  if(!ifs){
    error("Could not open file for reading.");
    return EXIT_FAILURE;
  }
  
  return eval_from_stream(ifs, interp);
}

int eval_from_command(std::string argexp, Interpreter & interp){

  std::istringstream expression(argexp);

  return eval_from_stream(expression, interp);
}

// A REPL is a repeated read-eval-print loop
void repl(Interpreter & interp){
    
  while(!std::cin.eof()){
    
    prompt();
    std::string line = readline();
    
    if(line.empty()) continue;

    std::istringstream expression(line);
    
    if(!interp.parseStream(expression)){
      error("Invalid Expression. Could not parse.");
    }
    else{
      try{
				Expression exp = interp.evaluate();
				std::cout << exp << std::endl;
      }
      catch(const SemanticError & ex){
				std::cerr << ex.what() << std::endl;
      }
    }
  }
}

int main(int argc, char *argv[])
{  
  Interpreter interp;
  
  std::ifstream ifs(STARTUP_FILE);

  ThreadSafeQueue<std::string> string_queue;
  ThreadSafeQueue<Expression> expression_queue;

  Producer producer(&string_queue, &expression_queue);
  Consumer consumer(&string_queue, &expression_queue, &interp);
  
  if(!ifs){
    error("Could not open startup file for reading.");
  }

  if(!interp.parseStream(ifs)){
    error("Invalid startup program. Could not parse.");
  }
  else{
    try{
      interp.evaluate();
    }
    catch(const SemanticError & ex){
      std::cout << "Startup program failed during evaluation" << std::endl;
    }	
  }

  if(argc == 2){
    return eval_from_file(argv[1], interp);
  }
  else if(argc == 3){
    if(std::string(argv[1]) == "-e"){
      return eval_from_command(argv[2], interp);
    }
    else{
      error("Incorrect number of command line arguments.");
    }
  }
  else{
    //repl(interp);
    std::thread producer_th(producer);
    std::thread consumer_th(consumer);

    producer_th.join();
    consumer_th.join();
  }
    
  return EXIT_SUCCESS;
}
