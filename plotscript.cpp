#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <thread>

#include "interpreter.hpp"
#include "semantic_error.hpp"
#include "startup_config.hpp"
#include "threadsafequeue.hpp"
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
void repl(ThreadSafeQueue<std::string> *pq, ThreadSafeQueue<Expression> *expq){
   
  while(!std::cin.eof()){
   
    prompt();
    std::string line = readline();
    
    if(line.empty()) continue;

    pq->push(line);

    Expression exp;
    expq->wait_and_pop(exp);

    if(exp.head().asSymbol() == "Error") std::cout << exp.tailConstBegin()->head().asSymbol() << std::endl;
    else std::cout << exp << std::endl;

  }

}

int main(int argc, char *argv[])
{  
  Interpreter interp;
  int running = 1;
  
  std::ifstream ifs(STARTUP_FILE);

  ThreadSafeQueue<std::string> program_queue;
  ThreadSafeQueue<Expression> expression_queue;

  Consumer consumer(&program_queue, &expression_queue, &interp, &running);
  
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
    std::thread consumer_th(consumer);

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
          /*else if (line == "%reset") {
            program_queue.push("%stop");
            consumer_th.join();
            running = 1;
            continue;
          }*/
          else if(line == "%exit") {
            program_queue.push("%stop");
            consumer_th.join();
            return EXIT_SUCCESS;
          }
          else {
            std::cout << "Error: invalid interpreter kernel directive" << std::endl;
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
            consumer_th.join();
            consumer_th = std::thread(consumer);
            continue;
          }
          if(line == "%stop") continue;
          /*else if (line == "%reset") {
            program_queue.push("%stop");
            consumer_th.join();
            running = 1;
            std::thread consumer_th(consumer);
            continue;
          }*/
          else if(line == "%exit") {
            return EXIT_SUCCESS;
          }
          else std::cout << "Error: invalid interpreter kernel directive" << std::endl;
        }
        else {
          std::cout << "Error: interpreter kernel not running" << std::endl;
          continue;
        }
      }


    }
    
    consumer_th.join();
  }
    
  return EXIT_SUCCESS;
}
