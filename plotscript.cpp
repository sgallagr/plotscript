#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <thread>
#include <cstdlib>
#include <csignal>

#include "interpreter.hpp"
#include "semantic_error.hpp"
#include "threadsafequeue.hpp"

volatile sig_atomic_t global_status_flag = 0;

// *****************************************************************************
// install a signal handler for Cntl-C on Windows
// *****************************************************************************
#if defined(_WIN64) || defined(_WIN32)
#include <windows.h>

// this function is called when a signal is sent to the process
BOOL WINAPI interrupt_handler(DWORD fdwCtrlType) {

  switch (fdwCtrlType) {
  case CTRL_C_EVENT: // handle Cnrtl-C
    // if not reset since last call, exit
    if (global_status_flag > 0) { 
      exit(EXIT_FAILURE);
    }
    ++global_status_flag;
    return TRUE;

  default:
    return FALSE;
  }
}

// install the signal handler
inline void install_handler() { SetConsoleCtrlHandler(interrupt_handler, TRUE); }
// *****************************************************************************

// *****************************************************************************
// install a signal handler for Cntl-C on Unix/Posix
// *****************************************************************************
#elif defined(__APPLE__) || defined(__linux) || defined(__unix) ||             \
    defined(__posix)
#include <unistd.h>

// this function is called when a signal is sent to the process
void interrupt_handler(int signal_num) {

  if(signal_num == SIGINT){ // handle Cnrtl-C
    // if not reset since last call, exit
    if (global_status_flag > 0) {
      exit(EXIT_FAILURE);
    }
    ++global_status_flag;
  }
}

// install the signal handler
inline void install_handler() {

  struct sigaction sigIntHandler;

  sigIntHandler.sa_handler = interrupt_handler;
  sigemptyset(&sigIntHandler.sa_mask);
  sigIntHandler.sa_flags = 0;

  sigaction(SIGINT, &sigIntHandler, NULL);
}
#endif
// *****************************************************************************

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

  Interpreter interp(&program_queue, &expression_queue);

  std::thread interp_th(interp);

  while(1){

    if (global_status_flag > 0) {
      if (std::cin.fail() || std::cin.eof()) {
        global_status_flag = 0;
        std::cin.clear(); // reset cin state
        std::cerr << "\nError: interpreter kernel interrupted" << std::endl;
        continue;
      } 
    }

    prompt();
    std::string line = readline();
    
    if(line.empty()) continue;

    if (running) {
      if (line.front() == '%') {
        if(line == "%start") {}
        else if(line == "%stop") {
          program_queue.push(line);
          running = 0;
        }
        else if (line == "%reset") {
          program_queue.push("%stop");
          interp_th.join();
          interp = Interpreter(&program_queue, &expression_queue);
          interp_th = std::thread(interp);
        }
        else if(line == "%exit") {
          program_queue.push("%stop");
          interp_th.join();
          return;
        }
        else error("invalid kernel directive");
      }
      else {
        program_queue.push(line);

        Expression exp;
        expression_queue.wait_and_pop(exp);
        if (exp.head().asSymbol() == "Interrupt") {
          global_status_flag = 0;
          std::cin.clear(); // reset cin state
          error("interpreter kernel interrupted");
          program_queue.push("%stop");
          interp_th.join();
          interp = Interpreter(&program_queue, &expression_queue);
          interp_th = std::thread(interp);
        }
        else if(exp.head().asSymbol() == "Error") std::cout << exp.tailConstBegin()->head().asSymbol() << std::endl;
        else std::cout << exp << std::endl;
      }
    }
    else {
      if (line.front() == '%') {
        if(line == "%start") {
          running = 1;
          interp_th.join();
          interp_th = std::thread(interp);
        }
        else if(line == "%stop") {}
        else if (line == "%reset") {
          running = 1;
          interp_th.join();
          interp = Interpreter(&program_queue, &expression_queue);
          interp_th = std::thread(interp);
        }
        else if(line == "%exit") {
          interp_th.join();
          return;
        }
        else error("invalid kernel directive");
      }
      else error("interpreter kernel not running");
    }
  }

}

int main(int argc, char *argv[])
{  
  install_handler();

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
