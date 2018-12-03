#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <thread>

#include "interpreter.hpp"
#include "semantic_error.hpp"
#include "threadsafequeue.hpp"

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
void repl(){
  ThreadSafeQueue<std::string> program_queue;
  ThreadSafeQueue<Expression> expression_queue;
  int running = 1;

  Interpreter interp(&program_queue, &expression_queue, &running);

  std::thread interp_th(interp);

  while(!std::cin.eof()){
   
    prompt();
    std::string line = readline();
    
    if(line.empty()) continue;

    if (running) {
      if (line.front() == '%') {
        if(line == "%start") {
          continue;
        }
        else if(line == "%stop") {
          program_queue.push(line);
          continue;
        }
        else if (line == "%reset") {
          program_queue.push("%stop");
          running = 1;
          interp_th.join();
          interp = Interpreter(&program_queue, &expression_queue, &running);
          interp_th = std::thread(interp);
          continue;
        }
        else if(line == "%exit") {
          program_queue.push("%stop");
          interp_th.join();
          return;
        }
        else {
          error("invalid interpreter kernel directive");
          continue;
        }
      }
      else {
        program_queue.push(line);

        Expression exp;
        expression_queue.wait_and_pop(exp);

        if(exp.head().asSymbol() == "Error") std::cout << exp.tailConstBegin()->head().asSymbol() << std::endl;
        else std::cout << exp << std::endl;
      }
    }
    else {
      if (line.front() == '%') {
        if(line == "%start") {
          running = 1;
          interp_th.join();
          interp_th = std::thread(interp);
          continue;
        }
        else if(line == "%stop") continue;
        else if (line == "%reset") {
          running = 1;
          interp_th.join();
          interp = Interpreter(&program_queue, &expression_queue, &running);
          interp_th = std::thread(interp);
          continue;
        }
        else if(line == "%exit") {
          interp_th.join();
          return;
        }
        error("invalid interpreter kernel directive");
      }
      else {
        error("interpreter kernel not running");
        continue;
      }
    }
  }

}

int main(int argc, char *argv[])
{  
  Interpreter interp;

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
  else repl();
    
  return EXIT_SUCCESS;
}
