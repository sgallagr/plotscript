#include "expression.hpp"

#include <sstream>

#include "environment.hpp"
#include "semantic_error.hpp"

Expression::Expression(){}

Expression::Expression(const Atom & a){

  m_head = a;
}

// recursive copy
Expression::Expression(const Expression & a){

  m_head = a.m_head;
  for(auto e : a.m_tail){
    m_tail.push_back(e);
  }
}

Expression & Expression::operator=(const Expression & a){

  // prevent self-assignment
  if(this != &a){
    m_head = a.m_head;
    m_tail.clear();
    for(auto e : a.m_tail){
      m_tail.push_back(e);
    } 
  }
  
  return *this;
}


Atom & Expression::head(){
  return m_head;
}

const Atom & Expression::head() const{
  return m_head;
}

bool Expression::isHeadNumber() const noexcept{
  return m_head.isNumber();
}

bool Expression::isHeadSymbol() const noexcept{
  return m_head.isSymbol();
}

bool Expression::isHeadComplex() const noexcept {
  return m_head.isComplex();
}

bool Expression::isList() const noexcept {
  return m_head.asSymbol() == "list";
}

bool Expression::isLambda() const noexcept {
  return m_head.asSymbol() == "lambda";
}

void Expression::append(const Atom & a){
  m_tail.emplace_back(a);
}

void Expression::append(const Expression & e) {
  m_tail.push_back(e);
}

Expression * Expression::tail(){
  Expression * ptr = nullptr;
  
  if(m_tail.size() > 0){
    ptr = &m_tail.back();
  }

  return ptr;
}

Expression::ConstIteratorType Expression::tailConstBegin() const noexcept{
  return m_tail.cbegin();
}

Expression::ConstIteratorType Expression::tailConstEnd() const noexcept{
  return m_tail.cend();
}

Expression apply(const Atom & op, const std::vector<Expression> & args, const Environment & env){

  // head must be a symbol
  if(!op.isSymbol()){
    throw SemanticError("Error during evaluation: procedure name not symbol");
  }
  
  // must map to a proc
  if(!env.is_proc(op)){
    throw SemanticError("Error during evaluation: symbol does not name a procedure");
  }
  
  // map from symbol to proc
  Procedure proc = env.get_proc(op);
  
  // call proc with args
  return proc(args);
}

Expression Expression::handle_lookup(const Atom & head, const Environment & env){
    if(head.isSymbol()){ // if symbol is in env return value
      if(env.is_exp(head)){
        return env.get_exp(head);
      }
      else{
        throw SemanticError("Error during evaluation: unknown symbol");
      }
    }
    else if(head.isNumber()){
      return Expression(head);
    }
    else{
      throw SemanticError("Error during evaluation: Invalid type in terminal expression");
    }
}

Expression Expression::handle_begin(Environment & env){
  
  if(m_tail.size() == 0){
    throw SemanticError("Error during evaluation: zero arguments to begin");
  }

  // evaluate each arg from tail, return the last
  Expression result;
  for(Expression::IteratorType it = m_tail.begin(); it != m_tail.end(); ++it){
    result = it->eval(env);
  }
  
  return result;
}


Expression Expression::handle_define(Environment & env){

  // tail must have size 3 or error
  if(m_tail.size() != 2){
    throw SemanticError("Error during evaluation: invalid number of arguments to define");
  }
  
  // tail[0] must be symbol
  if(!m_tail[0].isHeadSymbol()){
    throw SemanticError("Error during evaluation: first argument to define not symbol");
  }

  // but tail[0] must not be a special-form or procedure
  std::string s = m_tail[0].head().asSymbol();
  if((s == "define") || (s == "begin") || (s == "lambda")){
    throw SemanticError("Error during evaluation: attempt to redefine a special-form");
  }
  
  if(env.is_proc(m_head)){
    throw SemanticError("Error during evaluation: attempt to redefine a built-in procedure");
  }
  
  // eval tail[1]
  Expression result;
  /*std::vector<Expression> function;
  Expression proc;
  if (m_tail[1].head().asSymbol() == "lambda"){
    result = m_tail[1];
    for (auto e = m_tail[1].tailConstBegin(); e != m_tail[1].tailConstEnd(); ++e) {
      function.push_back(Expression(*e));
    }
  }*/
  result = m_tail[1].eval(env);

  /*if(env.is_exp(m_head)){
    throw SemanticError("Error during evaluation: attempt to redefine a previously defined symbol");
  }*/
    
  //and add to env
  //if(m_tail[1].head().asSymbol() == "lambda") env.add_proc(m_tail[0].head(), function);
  env.add_exp(m_tail[0].head(), result);
  
  return result;
}


Expression Expression::handle_lambda(Environment & env){

  // tail must have size 3 or error
  if(m_tail.size() != 2){
    throw SemanticError("Error during evaluation: invalid number of arguments to lambda");
  }
  
  for (auto e = m_tail[0].tailConstBegin(); e != m_tail[0].tailConstEnd(); ++e) {
    if (!Expression(*e).isHeadSymbol()) {
      throw SemanticError("Error during evaluation: invalid parameter for lambda");
    }
  }

  // tail[0] must not be a special-form or procedure
  std::string s = m_tail[0].head().asSymbol();
  if((s == "define") || (s == "begin") || (s == "lambda")){
    throw SemanticError("Error during evaluation: attempt to use special-form as parameter symbol");
  }
  
  if(env.is_proc(m_head)){
    throw SemanticError("Error during evaluation: attempt to use procedure as parameter symbol");
  }
  
  Expression params;
  params.append(m_tail[0].head());
  for (auto e = m_tail[0].tailConstBegin(); e != m_tail[0].tailConstEnd(); ++e) {
    params.append(*e);
  }

  Expression function = m_tail[1];

  Expression result(Atom("lambda"));
  result.append(params);
  result.append(function);
  /*if(env.is_exp(m_head)){
    throw SemanticError("Error during evaluation: attempt to redefine a previously defined symbol");
  }*/
    
  //and add to env
  //env.add_exp(m_tail[0].head(), result);
  //env.add_exp(m_tail, result);
  
  //return *result.tail();
  return result;
}


// this is a simple recursive version. the iterative version is more
// difficult with the ast data structure used (no parent pointer).
// this limits the practical depth of our AST
Expression Expression::eval(Environment & env){

  // lookup only if tail is empty and the head is not list
  if(m_tail.empty() && m_head.asSymbol() != "list"){
    return handle_lookup(m_head, env);
  }
  // handle begin special-form
  else if(m_head.isSymbol() && m_head.asSymbol() == "begin"){
    return handle_begin(env);
  }
  // handle define special-form
  else if(m_head.isSymbol() && m_head.asSymbol() == "define"){
    return handle_define(env);
  }
  // handle lambda special-form
  else if(m_head.isSymbol() && m_head.asSymbol() == "lambda"){
    return handle_lambda(env);
  }
  // else attempt to treat as procedure
  else{ 
    std::vector<Expression> results;

    if (env.get_exp(m_head).head().asSymbol() == "lambda") {
      Expression params = env.get_exp(m_head).m_tail[0];
      Expression function = env.get_exp(m_head).m_tail[1];
      if (params.m_tail.size() == m_tail.size()) {
        Environment envcopy = env;
        int count = 0;
        for (auto it = params.tailConstBegin(); it != params.tailConstEnd(); ++it ) {
          Expression singleparam(Atom("define"));
          singleparam.append(*it);
          singleparam.append(m_tail[count]);
          singleparam.eval(envcopy);
          ++count;
        }
        return function.eval(envcopy);
      }
      else {
        throw SemanticError("Error in call to lambda procedure: invalid number of arguments");
      }
    }
    else if (m_head.isSymbol() && m_head.asSymbol() == "apply") {
      if ((env.is_proc(m_tail[0].head()) && (m_tail[0].tailConstBegin() == m_tail[0].tailConstEnd())) || env.get_exp(m_tail[0].head()).head().asSymbol() == "lambda") {
        if (m_tail[1].isList()) {
          Expression result(m_tail[0].head());
          for (auto it = m_tail[1].tailConstBegin(); it != m_tail[1].tailConstEnd(); ++it) {
            result.append(*it);
          }
          return result.eval(env);
        }
        else {
          throw SemanticError("Error in call to apply: second argument not a list");
        }
      }
      else {
        throw SemanticError("Error in call to apply: first argument not a procedure");
      }
    }
    else if (m_head.isSymbol() && m_head.asSymbol() == "map") {
      if ((env.is_proc(m_tail[0].head()) && (m_tail[0].tailConstBegin() == m_tail[0].tailConstEnd())) || env.get_exp(m_tail[0].head()).head().asSymbol() == "lambda") {
        if (m_tail[1].isList()) {
          Atom proc = m_tail[0].head();
          Expression result;
          for (auto it = m_tail[1].tailConstBegin(); it != m_tail[1].tailConstEnd(); ++it) {
            Expression temp(proc);
            temp.append(*it);
            result.append(temp.eval(env));
          }
          return result;
        }
        else {
          throw SemanticError("Error in call to map: second argument not a list");
        }
      }
      else {
        throw SemanticError("Error in call to map: first argument not a procedure");
      }
    }
    else{
      for(Expression::IteratorType it = m_tail.begin(); it != m_tail.end(); ++it){
        results.push_back(it->eval(env));
      }
      return apply(m_head, results, env);
    }
  }
}


std::ostream & operator<<(std::ostream & out, const Expression & exp){

  // prevent double parentheses with complex results
  bool complex = false;
  if (exp.head().isComplex()) complex = true;

  if(!complex) out << "(";

  if (!exp.isList() && !exp.isLambda()) {
    out << exp.head();
    if (exp.isHeadSymbol() && (exp.tailConstBegin() != exp.tailConstEnd())) out << " ";
  }

  for(auto e = exp.tailConstBegin(); e != exp.tailConstEnd(); ++e){
    out << *e;
    if (e != exp.tailConstEnd() - 1) out << " ";
  }

  if(!complex) out << ")";

  return out;
}

bool Expression::operator==(const Expression & exp) const noexcept{

  bool result = (m_head == exp.m_head);

  result = result && (m_tail.size() == exp.m_tail.size());

  if(result){
    for(auto lefte = m_tail.begin(), righte = exp.m_tail.begin();
    (lefte != m_tail.end()) && (righte != exp.m_tail.end());
    ++lefte, ++righte){
      result = result && (*lefte == *righte);
    }
  }

  return result;
}

bool operator!=(const Expression & left, const Expression & right) noexcept{

  return !(left == right);
}
