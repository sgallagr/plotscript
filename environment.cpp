#include "environment.hpp"

#include <cassert>
#include <cmath>
#include <complex>
#include <string>

#include "environment.hpp"
#include "semantic_error.hpp"

/*********************************************************************** 
Helper Functions
**********************************************************************/

// predicate, the number of args is nargs
bool nargs_equal(const std::vector<Expression> & args, unsigned nargs){
  return args.size() == nargs;
}

/*********************************************************************** 
Each of the functions below have the signature that corresponds to the
typedef'd Procedure function pointer.
**********************************************************************/

// the default procedure always returns an expresison of type None
Expression default_proc(const std::vector<Expression> & args){
  args.size(); // make compiler happy we used this parameter
  return Expression();
};

Expression add(const std::vector<Expression> & args){

  double result = 0;
  std::complex<double> result_c(0,0);

  // keep track if there is any argument that is complex
  bool complex = false;

  // preconditions
  if (args.size() >= 2) {

    // check all aruments are numbers, while adding
    for (auto & a : args) {
      if (a.isHeadNumber()) {
        result += a.head().asNumber();
      }
      else if (a.isHeadComplex()) {
        if (!complex) complex = true;
        result_c += a.head().asComplex();
      }
      else {
        throw SemanticError("Error in call to add: argument not a number or complex number");
      }
    }
  }
  else {
    throw SemanticError("Error in call to add: invalid number of arguments.");
  }

  if (!complex) return Expression(result);
  else{
    result_c.real(std::real(result_c) + result);
    return Expression(result_c);
  }
};

Expression mul(const std::vector<Expression> & args){
 
  double result = 1;
  std::complex<double> result_c(0,0);

  // keep track if there is any argument that is complex
  bool complex = false;

  // preconditions
  if (args.size() >= 2) {

    // check all aruments are numbers, while multiplying
    for(auto & a : args){
      if(a.isHeadNumber()){
        result *= a.head().asNumber();
      }
      else if (a.isHeadComplex()){
        if (!complex)  {
          complex = true;
          result_c = a.head().asComplex();
        }
        else result_c *= a.head().asComplex();
      }
      else{
        throw SemanticError("Error in call to mul: argument not a number or complex number");
      }
    }
  }
  else {
    throw SemanticError("Error in call to mul: invalid number of arguments.");
  }

  if (!complex) return Expression(result);
  else{
    result_c.real(std::real(result_c) * result);
    result_c.imag(std::imag(result_c) * result);
    return Expression(result_c);
  }
};

Expression subneg(const std::vector<Expression> & args){

  double result = 0;
  std::complex<double> result_c(0,0);

  // keep track if there is any argument that is complex
  bool complex = false;

  // preconditions
  if(nargs_equal(args,1)){
    if(args[0].isHeadNumber()){
      result = -args[0].head().asNumber();
    }
    else if (args[0].isHeadComplex()) {
      result_c = -args[0].head().asComplex();
      complex = true;
    }
    else{
      throw SemanticError("Error in call to negate: invalid argument.");
    }
  }

  // preconditions
  else if(nargs_equal(args,2)){
    if( (args[0].isHeadNumber()) && (args[1].isHeadNumber()) ){
      result = args[0].head().asNumber() - args[1].head().asNumber();
    }
    else if( (args[0].isHeadNumber()) && (args[1].isHeadComplex()) ){
      result_c = args[0].head().asNumber() - args[1].head().asComplex();
      complex = true;
    }
    else if( (args[0].isHeadComplex()) && (args[1].isHeadNumber()) ){
      result_c = args[0].head().asComplex() - args[1].head().asNumber();
      complex = true;
    }
    else if( (args[0].isHeadComplex()) && (args[1].isHeadComplex()) ){
      result_c = args[0].head().asComplex() - args[1].head().asComplex();
      complex = true;
    }
    else{      
      throw SemanticError("Error in call to subtraction: invalid argument.");
    }
  }
  else{
    throw SemanticError("Error in call to subtraction or negation: invalid number of arguments.");
  }

  if (!complex) return Expression(result);
  else return Expression(result_c);
};

Expression div(const std::vector<Expression> & args){

  double result = 0; 
  std::complex<double> result_c = 0;

  // keep track if there is any argument that is complex
  bool complex = false;

  // preconditions
  if(nargs_equal(args,2)){
    if( (args[0].isHeadNumber()) && (args[1].isHeadNumber()) ){
      result = args[0].head().asNumber() / args[1].head().asNumber();
    }
    else if( (args[0].isHeadComplex()) && (args[1].isHeadNumber()) ){
      result_c = args[0].head().asComplex() / args[1].head().asNumber();
      complex = true;
    }
    else if( (args[0].isHeadNumber()) && (args[1].isHeadComplex()) ){
      result_c = args[0].head().asNumber() / args[1].head().asComplex();
      complex = true;
    }
    else if( (args[0].isHeadComplex()) && (args[1].isHeadComplex()) ){
      result_c = args[0].head().asComplex() / args[1].head().asComplex();
      complex = true;
    }
    else{      
      throw SemanticError("Error in call to division: invalid argument.");
    }
  }
  else{
    throw SemanticError("Error in call to division: invalid number of arguments.");
  }

  if (!complex) return Expression(result);
  else return Expression(result_c);
};

Expression sqrt(const std::vector<Expression> & args){

  double result = 0;
  std::complex<double> result_c(0,1);

  // keep track if the argument is complex
  bool complex = false;

  // preconditions
  if (nargs_equal(args, 1)) {
    if (args[0].isHeadNumber()) {
      if (args[0].head().asNumber() < 0) {
        complex = true;
        result_c *= -args[0].head().asNumber();
      }
      result = std::sqrt(args[0].head().asNumber());
    }
    else if (args[0].isHeadComplex()) {
      complex = true;
      result_c = std::sqrt(args[0].head().asComplex());
    }
    else {
      throw SemanticError("Error in call to square root: invalid argument.");
    }
  }
  else {
    throw SemanticError("Error in call to square root: invalid number of arguments.");
  }

  if (!complex) return Expression(result);
  else return Expression(result_c);
};

Expression pow(const std::vector<Expression> & args){

  double result = 0;
  std::complex<double> result_c(0,0);

  // keep track if there is any argument that is complex
  bool complex = false;

  // preconditions
  if (nargs_equal(args, 2)) {
    if ((args[0].isHeadNumber()) && (args[1].isHeadNumber())) {
      result = std::pow(args[0].head().asNumber(), args[1].head().asNumber());
    }
    else if ((args[0].isHeadNumber()) && (args[1].isHeadComplex())) {
      result_c = std::pow(args[0].head().asNumber(), args[1].head().asComplex());
      complex = true;
    }
    else if ((args[0].isHeadComplex()) && (args[1].isHeadNumber())) {
      result_c = std::pow(args[0].head().asComplex(), args[1].head().asNumber());
      complex = true;
    }
    else if ((args[0].isHeadComplex()) && (args[1].isHeadComplex())) {
      result_c = std::pow(args[0].head().asComplex(), args[1].head().asComplex());
      complex = true;
    }
    else {
      throw SemanticError("Error in call to power: invalid argument.");
    }
  }
  else {
    throw SemanticError("Error in call to power: invalid number of arguments.");
  }

  if (!complex) return Expression(result);
  else return Expression(result_c);
};

Expression ln(const std::vector<Expression> & args){

  double result = 0;

  // preconditions
  if (nargs_equal(args, 1)) {
    if (args[0].isHeadNumber()) {
      result = std::log(args[0].head().asNumber());
    }
    else {
      throw SemanticError("Error in call to natural log: invalid argument.");
    }
  }
  else {
    throw SemanticError("Error in call to natural log: invalid number of arguments.");
  }

  return Expression(result);
};

Expression sin(const std::vector<Expression> & args){

  double result = 0;

  // preconditions
  if (nargs_equal(args, 1)) {
    if (args[0].isHeadNumber()) {
      result = std::sin(args[0].head().asNumber());
    }
    else {
      throw SemanticError("Error in call to sine: invalid argument.");
    }
  }
  else {
    throw SemanticError("Error in call to sine: invalid number of arguments.");
  }

  return Expression(result);
};

Expression cos(const std::vector<Expression> & args){

  double result = 0;

  // preconditions
  if (nargs_equal(args, 1)) {
    if (args[0].isHeadNumber()) {
      result = std::cos(args[0].head().asNumber());
    }
    else {
      throw SemanticError("Error in call to cosine: invalid argument.");
    }
  }
  else {
    throw SemanticError("Error in call to cosine: invalid number of arguments.");
  }

  return Expression(result);
};

Expression tan(const std::vector<Expression> & args){

  double result = 0;

  // preconditions
  if (nargs_equal(args, 1)) {
    if (args[0].isHeadNumber()) {
      result = std::tan(args[0].head().asNumber());
    }
    else {
      throw SemanticError("Error in call to tangent: invalid argument.");
    }
  }
  else {
    throw SemanticError("Error in call to tangent: invalid number of arguments.");
  }

  return Expression(result);
};

Expression real(const std::vector<Expression> & args){
  
  double result = 0;

  // preconditions
  if (nargs_equal(args, 1)) {
    if (args[0].isHeadComplex()) {
      result = std::real(args[0].head().asComplex());
    }
    else {
      throw SemanticError("Error in call to retrieve real: invalid argument.");
    }
  }
  else {
    throw SemanticError("Error in call to retrieve real: invalid number of arguments.");
  }

  return Expression(result);
};

Expression imag(const std::vector<Expression> & args){
  
  double result = 0;

  // preconditions
  if (nargs_equal(args, 1)) {
    if (args[0].isHeadComplex()) {
      result = std::imag(args[0].head().asComplex());
    }
    else {
      throw SemanticError("Error in call to retrieve imaginary: invalid argument.");
    }
  }
  else {
    throw SemanticError("Error in call to retrieve imaginary: invalid number of arguments.");
  }

  return Expression(result);
};

Expression mag(const std::vector<Expression> & args){
  
  double result = 0;

  // preconditions
  if (nargs_equal(args, 1)) {
    if (args[0].isHeadComplex()) {
      result = std::abs(args[0].head().asComplex());
    }
    else {
      throw SemanticError("Error in call to retrieve magnitude: invalid argument.");
    }
  }
  else {
    throw SemanticError("Error in call to retrieve magnitude: invalid number of arguments.");
  }

  return Expression(result);
};

Expression arg(const std::vector<Expression> & args){
  
  double result = 0;

  // preconditions
  if (nargs_equal(args, 1)) {
    if (args[0].isHeadComplex()) {
      result = std::arg(args[0].head().asComplex());
    }
    else {
      throw SemanticError("Error in call to retrieve argument: invalid argument.");
    }
  }
  else {
    throw SemanticError("Error in call to retrieve argument: invalid number of arguments.");
  }

  return Expression(result);
};

Expression conj(const std::vector<Expression> & args){
  
  std::complex<double> result(0,0);

  // preconditions
  if (nargs_equal(args, 1)) {
    if (args[0].isHeadComplex()) {
      result = std::conj(args[0].head().asComplex());
    }
    else {
      throw SemanticError("Error in call to retrieve conjugate: invalid argument.");
    }
  }
  else {
    throw SemanticError("Error in call to retrieve conjugate: invalid number of arguments.");
  }

  return Expression(result);
};

Expression list(const std::vector<Expression> & args){

  Atom listHead("list");
  Expression result(listHead);

  for (auto & a : args) {
    if (a.isHeadNumber() || a.isHeadComplex() || a.isList()) {
      result.append(a);
    }
    else {
      throw SemanticError("Error in call to build list: invalid argument");
    }
  }
  
  return result;
};

Expression first(const std::vector<Expression> & args){

  if (nargs_equal(args, 1)) {
    if (args[0].isList()) {
      if (args[0].tailConstBegin() != args[0].tailConstEnd()) {
        return Expression(*args[0].tailConstBegin());
      }
      else {
        throw SemanticError("Error in call to first: argument is an empty list");
      }
    }
    else {
      throw SemanticError("Error in call to first: argument not a list");
    }
  }
  else {
    throw SemanticError("Error in call to first: more than one argument");
  }

};

Expression rest(const std::vector<Expression> & args) {

  Atom listHead("list");
  Expression result(listHead);

  if (nargs_equal(args, 1)) {
    if (args[0].isList()) {
      if (args[0].tailConstBegin() != args[0].tailConstEnd()) {
        for (auto e = args[0].tailConstBegin() + 1; e != args[0].tailConstEnd(); ++e) {
          result.append(*e);
        }
      }
      else {
        throw SemanticError("Error in call to rest: argument is an empty list");
      }
    }
    else {
      throw SemanticError("Error in call to rest: argument not a list");
    }
  }
  else {
    throw SemanticError("Error in call to rest: more than one argument");
  }

  return result;
};

Expression length(const std::vector<Expression> & args) {
  
  int result = 0;

  if (nargs_equal(args, 1)) {
    if (args[0].isList()) {
      if (args[0].tailConstBegin() == args[0].tailConstEnd()) {
        return Expression(result);
      }
      else {
        for (auto e = args[0].tailConstBegin(); e != args[0].tailConstEnd(); ++e) {
          result++;
        }
      }
    }
    else {
      throw SemanticError("Error in call to length: argument not a list");
    }
  }
  else {
    throw SemanticError("Error in call to length: more than one argument");
  }

  return Expression(result);
};

Expression append(const std::vector<Expression> & args){

  Atom listHead("list");
  Expression result(listHead);

  if (nargs_equal(args, 2)) {
    if (args[0].isList()) {

      for (auto e = args[0].tailConstBegin(); e != args[0].tailConstEnd(); ++e) {
        result.append(*e);
      }

      if (args[1].isHeadNumber() || args[1].isHeadComplex() || args[1].isList()) {
        result.append(args[1]);
      }
      else {
        throw SemanticError("Error in call to append: invalid second argument");
      }
    }
    else {
      throw SemanticError("Error in call to append: first argument not a list");
    }
  }
  else {
    throw SemanticError("Error in call to append: invalid number of arguments");
  }
  
  return result;
};

Expression join(const std::vector<Expression> & args){

  Atom listHead("list");
  Expression result(listHead);

  if (nargs_equal(args, 2)) {
    if (args[0].isList()) {

      for (auto e = args[0].tailConstBegin(); e != args[0].tailConstEnd(); ++e) {
        result.append(*e);
      }

      if (args[1].isList()) {
        for (auto e = args[1].tailConstBegin(); e != args[1].tailConstEnd(); ++e) {
          result.append(*e);
        }
      }
      else {
        throw SemanticError("Error in call to join: second argument not a list");
      }
    }
    else {
      throw SemanticError("Error in call to append: first argument not a list");
    }
  }
  else {
    throw SemanticError("Error in call to append: invalid number of arguments");
  }
  
  return result;
};

Expression range(const std::vector<Expression> & args) {

  Atom listHead("list");
  Expression result(listHead);

  double begin = args[0].head().asNumber();

  if (nargs_equal(args, 3)) {
    if (args[0].isHeadNumber() && args[1].isHeadNumber() && args[2].isHeadNumber()) {
      if (args[0].head().asNumber() < args[1].head().asNumber()) {
        if (args[2].head().asNumber() > 0) {
          double begin = args[0].head().asNumber();
          double end = args[1].head().asNumber();
          double inc = args[2].head().asNumber();

          for (double i = begin; i <= end; i += inc) {
            result.append(i);
          }
        }
        else {
          throw SemanticError("Error in call to range: negative or zero increment");
        }
      }
      else {
        throw SemanticError("Error in call to range: begin greater than or equal to end");
      }
    }
    else {
      throw SemanticError("Error in call to range: invalid argument");
    }
  }
  else {
    throw SemanticError("Error in call to range: invalid number of arguments");
  }

  return result;
};

const double PI = std::atan2(0, -1);
const double EXP = std::exp(1);
const std::complex<double> I(0,1);

Environment::Environment(){

  reset();
}

bool Environment::is_known(const Atom & sym) const{
  if(!sym.isSymbol()) return false;
  
  return envmap.find(sym.asSymbol()) != envmap.end();
}

bool Environment::is_exp(const Atom & sym) const{
  if(!sym.isSymbol()) return false;
  
  auto result = envmap.find(sym.asSymbol());
  return (result != envmap.end()) && (result->second.type == ExpressionType);
}

Expression Environment::get_exp(const Atom & sym) const{

  Expression exp;
  
  if(sym.isSymbol()){
    auto result = envmap.find(sym.asSymbol());
    if((result != envmap.end()) && (result->second.type == ExpressionType)){
      exp = result->second.exp;
    }
  }

  return exp;
}

void Environment::add_exp(const Atom & sym, const Expression & exp){

  if(!sym.isSymbol()){
    throw SemanticError("Attempt to add non-symbol to environment");
  }
    
  // error if overwriting symbol map
  if(envmap.find(sym.asSymbol()) != envmap.end()){
    throw SemanticError("Attempt to overwrite symbol in environemnt");
  }

  envmap.emplace(sym.asSymbol(), EnvResult(ExpressionType, exp)); 
}

bool Environment::is_proc(const Atom & sym) const{
  if(!sym.isSymbol()) return false;
  
  auto result = envmap.find(sym.asSymbol());
  return (result != envmap.end()) && (result->second.type == ProcedureType);
}

Procedure Environment::get_proc(const Atom & sym) const{

  //Procedure proc = default_proc;

  if(sym.isSymbol()){
    auto result = envmap.find(sym.asSymbol());
    if((result != envmap.end()) && (result->second.type == ProcedureType)){
      return result->second.proc;
    }
  }

  return default_proc;
}

void Environment::add_proc(const Atom & sym, const Expression & proc){

  if(!sym.isSymbol()){
    throw SemanticError("Attempt to add non-symbol to environment");
  }
    
  // error if overwriting symbol map
  if(envmap.find(sym.asSymbol()) != envmap.end()){
    throw SemanticError("Attempt to overwrite symbol in environemnt");
  }

  envmap.emplace(sym.asSymbol(), EnvResult(ProcedureType, proc)); 
}

/*
Reset the environment to the default state. First remove all entries and
then re-add the default ones.
 */
void Environment::reset(){

  envmap.clear();
  
  // Built-In value of pi
  envmap.emplace("pi", EnvResult(ExpressionType, Expression(PI)));

  // Built-In value of Euler's number
  envmap.emplace("e", EnvResult(ExpressionType, Expression(EXP)));

  // Built-In value of I
  envmap.emplace("I", EnvResult(ExpressionType, Expression(I)));

  // Procedure: add;
  envmap.emplace("+", EnvResult(ProcedureType, add)); 

  // Procedure: subneg;
  envmap.emplace("-", EnvResult(ProcedureType, subneg)); 

  // Procedure: mul;
  envmap.emplace("*", EnvResult(ProcedureType, mul)); 

  // Procedure: div;
  envmap.emplace("/", EnvResult(ProcedureType, div));

  // Procedure: sqrt;
  envmap.emplace("sqrt", EnvResult(ProcedureType, sqrt));

  // Procedure: pow;
  envmap.emplace("^", EnvResult(ProcedureType, pow));

  // Procedure: ln;
  envmap.emplace("ln", EnvResult(ProcedureType, ln));

  // Procedure: sin;
  envmap.emplace("sin", EnvResult(ProcedureType, sin));

  // Procedure: cos;
  envmap.emplace("cos", EnvResult(ProcedureType, cos));

  // Procedure: tan;
  envmap.emplace("tan", EnvResult(ProcedureType, tan));

  // Procedure: real;
  envmap.emplace("real", EnvResult(ProcedureType, real));

  // Procedure: imag;
  envmap.emplace("imag", EnvResult(ProcedureType, imag));

  // Procedure: mag;
  envmap.emplace("mag", EnvResult(ProcedureType, mag));

  // Procedure: arg;
  envmap.emplace("arg", EnvResult(ProcedureType, arg));

  // Procedure: conj;
  envmap.emplace("conj", EnvResult(ProcedureType, conj));

  // Procedure: list;
  envmap.emplace("list", EnvResult(ProcedureType, list));

  // Procedure: first;
  envmap.emplace("first", EnvResult(ProcedureType, first));

  // Procedure: rest;
  envmap.emplace("rest", EnvResult(ProcedureType, rest));

  // Procedure: length;
  envmap.emplace("length", EnvResult(ProcedureType, length));

  // Procedure: append;
  envmap.emplace("append", EnvResult(ProcedureType, append));

  // Procedure: join;
  envmap.emplace("join", EnvResult(ProcedureType, join));

  // Procedure: range;
  envmap.emplace("range", EnvResult(ProcedureType, range));
}
