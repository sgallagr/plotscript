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

  // carry over properties
  propmap = a.propmap;
}

Expression & Expression::operator=(const Expression & a){

  // prevent self-assignment
  if(this != &a){
    m_head = a.m_head;
    m_tail.clear();
    for(auto e : a.m_tail){
      m_tail.push_back(e);
    } 

    // carry over properties
    propmap = a.propmap;
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

bool Expression::isStringLit() const noexcept {
  return m_head.asSymbol().front() == '"';
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

int Expression::tailSize() {
  return m_tail.size();
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
      if (head.asSymbol().front() == '"') {
        return Expression(head);
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
  
  if(env.is_proc(m_tail[0].head()) || m_tail[0].head().asSymbol() == "apply" || m_tail[0].head().asSymbol() == "map"
    ||  m_tail[0].head().asSymbol() == "set-property" ||  m_tail[0].head().asSymbol() == "get-property"){
    throw SemanticError("Error during evaluation: attempt to redefine a built-in procedure");
  }
  
  // eval tail[1]
  Expression result = m_tail[1].eval(env);
    
  //and add to env
  env.add_exp(m_tail[0].head(), result);
  
  return result;
}


Expression Expression::handle_lambda(Environment & env){

  // tail must have size 2 or error
  if(m_tail.size() != 2){
    throw SemanticError("Error during evaluation: invalid number of arguments to lambda");
  }
  
  // tail[0] must contain symbols only
  if (!m_tail[0].isHeadSymbol()) {
    throw SemanticError("Error during evaluation: invalid parameter for lambda");
  }
  for (auto it = m_tail[0].tailConstBegin(); it != m_tail[0].tailConstEnd(); ++it) {
    if (!Expression(*it).isHeadSymbol()) {
      throw SemanticError("Error during evaluation: invalid parameter for lambda");
    }
  }

  // tail[0] must not be a special-form or procedure
  std::string s = m_tail[0].head().asSymbol();
  if((s == "define") || (s == "begin") || (s == "lambda")){
    throw SemanticError("Error during evaluation: attempt to use special-form as parameter symbol");
  }
  
  if(env.is_proc(m_tail[0].head())){
    throw SemanticError("Error during evaluation: attempt to use existing procedure as parameter symbol");
  }
  
  // create list of parameter symbols
  Expression params;
  params.append(m_tail[0].head());
  for (auto e = m_tail[0].tailConstBegin(); e != m_tail[0].tailConstEnd(); ++e) {
    params.append(*e);
  }

  // create procedure expression
  Expression proc = m_tail[1];

  // create full lambda expression
  Expression result(Atom("lambda"));
  result.append(params);
  result.append(proc);

  return result;
}

void Expression::set_property(const Atom & sym, const Expression & exp){
    
  // allow overwriting of properties
  if(propmap.find(sym.asSymbol()) != propmap.end()){
    propmap[sym.asSymbol()] = exp;
  }
  else propmap.emplace(sym.asSymbol(),exp); 
}

Expression Expression::get_property(const Atom & sym) const{
  Expression exp;
  
  if(sym.isSymbol()){
    auto result = propmap.find(sym.asSymbol());
    if(result != propmap.end()){
      exp = result->second;
    }
  }

  return exp;
}

Expression Expression::handle_set_property(Environment & env){

  // tail must have size 3 or error
  if(m_tail.size() != 3){
    throw SemanticError("Error during evaluation: invalid number of arguments to set-property");
  }
  
  // tail[0] must contain a string
  if (!m_tail[0].isStringLit()) {
    throw SemanticError("Error during evaluation: first argument to set-property not a string");
  }
  
  Atom key = m_tail[0].head().asSymbol();

  Expression value = m_tail[1].eval(env);

  Expression result = m_tail[2].eval(env);

  result.set_property(key, value);

  return result;
}

Expression Expression::handle_get_property(Environment & env){
  Expression result;

  // tail must have size 2 or error
  if(m_tail.size() != 2){
    throw SemanticError("Error during evaluation: invalid number of arguments to get-property");
  }
  
  // tail[0] must contain a string
  if (!m_tail[0].isStringLit()) {
    throw SemanticError("Error during evaluation: first argument to get-property not a string");
  }

  if(env.is_exp(m_tail[1].head())){
    result = env.get_exp(m_tail[1].head()).get_property(m_tail[0].head());
  }
  else {
    result = m_tail[1].get_property(m_tail[0].head());
  }
 
  return result;
}

Expression Expression::handle_discrete_plot(Environment & env){

  Expression result(Atom("list"));
  result.set_property(Atom("\"discrete-plot\""), Expression(Atom("\"true\"")));

  double x_min, y_min, x_max, y_max, x_val, y_val;
  double text_scale = 1;
  
  Expression resetLine(Atom("list")), line, left_bound, right_bound, upper_bound, lower_bound, ordinate_cross, abscissa_cross;
  resetLine.set_property(Atom("\"object-name\""), Expression(Atom("\"line\"")));
  resetLine.set_property(Atom("\"thickness\""), Expression(Atom(0)));
  left_bound = right_bound = upper_bound = lower_bound = ordinate_cross = abscissa_cross = line = resetLine;

  Expression resetPoint(Atom("list")), point, pointA, pointB;
  resetPoint.set_property(Atom("\"object-name\""), Expression(Atom("\"point\"")));
  resetPoint.set_property(Atom("\"size\""), Expression(Atom(0.5)));
  pointA = pointB = point = resetPoint;

  Expression text;

  Expression data = m_tail[0].eval(env);

  // tail must have size 2 or error
  if(m_tail.size() != 2){
    throw SemanticError("Error during evaluation: invalid number of arguments to discrete-plot");
  }
  
  // tail[0] must contain a non-empty list
  if (!data.isList() || data.tailSize() == 0) {
    throw SemanticError("Error during evaluation: first argument to discrete-plot not a non-empty list");
  }

  // tail[1] must contain a list
  if (!m_tail[1].isList()) {
    throw SemanticError("Error during evaluation: second argument to discrete-plot not a list");
  }

  // Determine max and min x and y values for plot
  Expression tempPoint = data.m_tail[0].eval(env);
  x_min = x_max = tempPoint.m_tail[0].head().asNumber();
  y_min = y_max = tempPoint.m_tail[1].head().asNumber();

  for(auto it = data.tailConstBegin(); it != data.tailConstEnd(); ++it){
    tempPoint = *it;
    x_val = tempPoint.m_tail[0].head().asNumber();
    y_val = tempPoint.m_tail[1].head().asNumber();
    
    if (x_min > x_val) x_min = x_val;
    else if (x_max < x_val) x_max = x_val;
     
    if (y_min > y_val) y_min = y_val;
    else if (y_max < y_val) y_max = y_val; 
  }

  // Create all lines necessary for plot
  double right, left, upper, lower;

  double x_coeff = 20 / (x_max - x_min);
  double y_coeff = 20 / (y_max - y_min);

  right = x_max * x_coeff;
  left = x_min * x_coeff;
  upper = y_max * y_coeff;
  lower = y_min * y_coeff;

  // make left bound line
  pointA.append(left);
  pointA.append(-upper);
  left_bound.append(pointA);
  pointB.append(left);
  pointB.append(-lower);
  left_bound.append(pointB);
  result.append(left_bound);
  pointA = pointB = resetPoint;

  // make right bound line
  pointA.append(right);
  pointA.append(-upper);
  right_bound.append(pointA);
  pointB.append(right);
  pointB.append(-lower);
  right_bound.append(pointB);
  result.append(right_bound);
  pointA = pointB = resetPoint;

  // make lower bound line (inverted in view)
  pointA.append(left);
  pointA.append(-lower);
  upper_bound.append(pointA);
  pointB.append(right);
  pointB.append(-lower);
  upper_bound.append(pointB);
  result.append(upper_bound);
  pointA = pointB = resetPoint;

  // make upper bound line (inverted in view)
  pointA.append(left);
  pointA.append(-upper);
  lower_bound.append(pointA);
  pointB.append(right);
  pointB.append(-upper);
  lower_bound.append(pointB);
  result.append(lower_bound);
  pointA = pointB = resetPoint;

  // make ordinate cross line
  if (x_min < 0) {
    pointA.append(0);
    pointA.append(-upper);
    ordinate_cross.append(pointA);
    pointB.append(0);
    pointB.append(-lower);
    ordinate_cross.append(pointB);
    result.append(ordinate_cross);
    pointA = pointB = resetPoint;
  }

  // make abscissa cross line
  if (y_min < 0) {
    pointA.append(left);
    pointA.append(0);
    abscissa_cross.append(pointA);
    pointB.append(right);
    pointB.append(0);
    abscissa_cross.append(pointB);
    result.append(abscissa_cross);
  }

  // Organize required points and lines for plot
  for(auto it = data.tailConstBegin(); it != data.tailConstEnd(); ++it){
    x_val = it->m_tail[0].head().asNumber();
    y_val = it->m_tail[1].head().asNumber();

    // scale
    if (x_val >= 0) x_val *= (right / x_max);
    else x_val *= (std::abs(left) / std::abs(x_min));
 
    // y-axis inverted in view
    if (y_val >= 0) y_val *= (upper / y_max) * -1;
    else y_val = y_val * (std::abs(lower) / std::abs(y_min)) * -1;

    pointA = pointB = point = resetPoint;
    line = resetLine;

    point.append(x_val);
    point.append(y_val);
    result.append(point);

    pointA.append(x_val);
    if (y_min < 0) pointA.append(0);
    else pointA.append(-lower);
    pointB.append(x_val);
    pointB.append(y_val);
    line.append(pointA);
    line.append(pointB);

    result.append(line);
  } 

  
  // Find text scaling value if any
  for(auto it = m_tail[1].tailConstBegin(); it != m_tail[1].tailConstEnd(); ++it){
    if (it->tailConstBegin()->head().asSymbol() == "\"text-scale\"") {
      text_scale = (it->tailConstEnd() - 1)->head().asNumber();
    }
  }

  for(auto it = m_tail[1].tailConstBegin(); it != m_tail[1].tailConstEnd(); ++it){
    point = resetPoint;
    point.set_property(Atom("\"size\""), Expression(Atom(0)));

    if (it->tailConstBegin()->head().asSymbol() == "\"title\"") {
      text = Expression((it->tailConstEnd() - 1)->head().asSymbol());
      text.set_property(Atom("\"object-name\""), Expression(Atom("\"text\"")));
      text.set_property(Atom("\"text-scale\""), Expression(Atom(text_scale)));
      point.append(Atom(((right - left) / 2) + left));
      point.append(Atom(-upper - 3));
      text.set_property(Atom("\"position\""), point);
      result.append(text);
    }

    else if (it->tailConstBegin()->head().asSymbol() == "\"abscissa-label\"") {
      text = Expression((it->tailConstEnd() - 1)->head().asSymbol());
      text.set_property(Atom("\"object-name\""), Expression(Atom("\"text\"")));
      text.set_property(Atom("\"text-scale\""), Expression(Atom(text_scale)));
      point.append(Atom(((right - left) / 2) + left));
      point.append(Atom(-lower + 3));
      text.set_property(Atom("\"position\""), point);
      result.append(text);
    }

    else if (it->tailConstBegin()->head().asSymbol() == "\"ordinate-label\"") {
      text = Expression((it->tailConstEnd() - 1)->head().asSymbol());
      text.set_property(Atom("\"object-name\""), Expression(Atom("\"text\"")));
      text.set_property(Atom("\"text-scale\""), Expression(Atom(text_scale)));
      text.set_property(Atom("\"text-rotation\""), Expression(Atom(-std::atan2(0, -1) / 2)));
      point.append(Atom(left - 3));
      point.append(Atom(-1 * (((upper - lower) / 2) + lower)));
      text.set_property(Atom("\"position\""), point);
      result.append(text);
    }
  }

  std::ostringstream out;
  out.precision(2);

  point = resetPoint;
  out.str("");
  out << y_max;
  text = Expression(Atom(out.str()));
  text.set_property(Atom("\"object-name\""), Expression(Atom("\"text\"")));
  text.set_property(Atom("\"text-scale\""), Expression(Atom(text_scale)));
  point.append(Atom(left - 2));
  point.append(Atom(-upper));
  text.set_property(Atom("\"position\""), point);
  result.append(text);

  point = resetPoint;
  out.str("");
  out << y_min;
  text = Expression(Atom(out.str()));
  text.set_property(Atom("\"object-name\""), Expression(Atom("\"text\"")));
  text.set_property(Atom("\"text-scale\""), Expression(Atom(text_scale)));
  point.append(Atom(left - 2));
  point.append(Atom(-lower));
  text.set_property(Atom("\"position\""), point);
  result.append(text);

  point = resetPoint;
  out.str("");
  out << x_max;
  text = Expression(Atom(out.str()));
  text.set_property(Atom("\"object-name\""), Expression(Atom("\"text\"")));
  text.set_property(Atom("\"text-scale\""), Expression(Atom(text_scale)));
  point.append(Atom(right));
  point.append(Atom(-lower + 2));
  text.set_property(Atom("\"position\""), point);
  result.append(text);

  point = resetPoint;
  out.str("");
  out << x_min;
  text = Expression(Atom(out.str()));
  text.set_property(Atom("\"object-name\""), Expression(Atom("\"text\"")));
  text.set_property(Atom("\"text-scale\""), Expression(Atom(text_scale)));
  point.append(Atom(left));
  point.append(Atom(-lower + 2));
  text.set_property(Atom("\"position\""), point);
  result.append(text);
  

  return result;
}

Expression Expression::handle_continuous_plot(Environment & env){
  Expression initial;
  Expression result(Atom("list"));
  result.set_property(Atom("\"discrete-plot\""), Expression(Atom("\"true\"")));

  double x_min, y_min, x_max, y_max, x_val, y_val;
  double text_scale = 1;
  
  Expression resetLine(Atom("list")), line, left_bound, right_bound, upper_bound, lower_bound, ordinate_cross, abscissa_cross;
  resetLine.set_property(Atom("\"object-name\""), Expression(Atom("\"line\"")));
  resetLine.set_property(Atom("\"thickness\""), Expression(Atom(0)));
  left_bound = right_bound = upper_bound = lower_bound = ordinate_cross = abscissa_cross = line = resetLine;

  Expression resetPoint(Atom("list")), point, pointA, pointB;
  resetPoint.set_property(Atom("\"object-name\""), Expression(Atom("\"point\"")));
  resetPoint.set_property(Atom("\"size\""), Expression(Atom(0.5)));
  pointA = pointB = point = resetPoint;

  Expression text;


  // tail must have size 2 or 3 or error
  if(m_tail.size() != 2 && m_tail.size() != 3){
    throw SemanticError("Error during evaluation: invalid number of arguments to continuous-plot");
  }
  
  // tail[0] must contain a lambda function
  if (env.get_exp(m_tail[0].head()).head().asSymbol() != "lambda") {
    throw SemanticError("Error during evaluation: first argument to continuous-plot not a lambda function");
  }

  // tail[1] must contain a list that is not empty
  if (!m_tail[1].isList() || m_tail[1].m_tail.size() != 2) {
    throw SemanticError("Error during evaluation: second argument to continuous-plot not a list or incorrect number of items in list");
  }

  // tail[2] must contain a list
  if (m_tail.size() == 3 && !m_tail[2].isList()) {
    throw SemanticError("Error during evaluation: third argument to continuous-plot not a list");
  }



 Expression x_values = m_tail[1].eval(env);
 Expression proc(m_tail[0].head().asSymbol());
 Expression y_values;
 Expression temp;

 x_min = x_values.m_tail[0].head().asNumber();
 x_max = x_values.m_tail[1].head().asNumber();

 double inc_val = (x_max - x_min) / 50.0;

   // Find y values
  for(double i = x_min; i < x_max + inc_val; i += inc_val){

    temp = proc;
    x_val = i;
    temp.append(x_val);
    y_val = temp.eval(env).head().asNumber();

    y_values.append(y_val);
  }

 // Determine max and min y values for plot
  y_min = y_max = y_values.m_tail[0].head().asNumber();

  for(unsigned i = 0; i < y_values.m_tail.size(); ++i){
    y_val = y_values.m_tail[i].head().asNumber();
     
    if (y_min > y_val) y_min = y_val;
    else if (y_max < y_val) y_max = y_val; 
  }

  // Create all lines necessary for plot
  double right, left, upper, lower;

  double x_coeff = 20 / (x_max - x_min);
  double y_coeff = 20 / (y_max - y_min);

  right = x_max * x_coeff;
  left = x_min * x_coeff;
  upper = y_max * y_coeff;
  lower = y_min * y_coeff;

 
  double x_valNext, y_valNext;

  // Make all lines
  for(double i = x_min; i < x_max; i += inc_val){
   
    pointA = pointB = resetPoint;
    line = resetLine;

    temp = proc;
    x_val = i;
    temp.append(x_val);
    y_val = temp.eval(env).head().asNumber();

    temp = proc;
    x_valNext = x_val + inc_val;
    temp.append(x_valNext);
    y_valNext = temp.eval(env).head().asNumber();

    // scale
    if (x_val >= 0) x_val *= (right / x_max);
    else x_val *= (std::abs(left) / std::abs(x_min));
 
    // y-axis inverted in view
    if (y_val >= 0) y_val *= (upper / y_max) * -1;
    else y_val = y_val * (std::abs(lower) / std::abs(y_min)) * -1;

    // scale
    if (x_valNext >= 0) x_valNext *= (right / x_max);
    else x_valNext *= (std::abs(left) / std::abs(x_min));
 
    // y-axis inverted in view
    if (y_valNext >= 0) y_valNext *= (upper / y_max) * -1;
    else y_valNext = y_valNext * (std::abs(lower) / std::abs(y_min)) * -1;

    pointA.append(x_val);
    pointA.append(y_val);
    line.append(pointA);
    pointB.append(x_valNext);
    pointB.append(y_valNext);
    line.append(pointB);
    result.append(line);

  }

  // make left bound line
  pointA = pointB = resetPoint;
  pointA.append(left);
  pointA.append(-upper);
  left_bound.append(pointA);
  pointB.append(left);
  pointB.append(-lower);
  left_bound.append(pointB);
  result.append(left_bound);
  pointA = pointB = resetPoint;

  // make right bound line
  pointA.append(right);
  pointA.append(-upper);
  right_bound.append(pointA);
  pointB.append(right);
  pointB.append(-lower);
  right_bound.append(pointB);
  result.append(right_bound);
  pointA = pointB = resetPoint;

  // make lower bound line (inverted in view)
  pointA.append(left);
  pointA.append(-lower);
  upper_bound.append(pointA);
  pointB.append(right);
  pointB.append(-lower);
  upper_bound.append(pointB);
  result.append(upper_bound);
  pointA = pointB = resetPoint;

  // make upper bound line (inverted in view)
  pointA.append(left);
  pointA.append(-upper);
  lower_bound.append(pointA);
  pointB.append(right);
  pointB.append(-upper);
  lower_bound.append(pointB);
  result.append(lower_bound);
  pointA = pointB = resetPoint;

  // make ordinate cross line
  if (x_min < 0) {
    pointA.append(0);
    pointA.append(-upper);
    ordinate_cross.append(pointA);
    pointB.append(0);
    pointB.append(-lower);
    ordinate_cross.append(pointB);
    result.append(ordinate_cross);
    pointA = pointB = resetPoint;
  }

  // make abscissa cross line
  if (y_min < 0) {
    pointA.append(left);
    pointA.append(0);
    abscissa_cross.append(pointA);
    pointB.append(right);
    pointB.append(0);
    abscissa_cross.append(pointB);
    result.append(abscissa_cross);
  }

// Find text scaling value if any
  if (m_tail.size() == 3) {
    for(auto it = m_tail[2].tailConstBegin(); it != m_tail[2].tailConstEnd(); ++it){
      if (it->tailConstBegin()->head().asSymbol() == "text-scale") {
        text_scale = (it->tailConstEnd() - 1)->head().asNumber();
      }
    }

    for(auto it = m_tail[2].tailConstBegin(); it != m_tail[2].tailConstEnd(); ++it){
      point = resetPoint;
      point.set_property(Atom("\"size\""), Expression(Atom(0)));

      if (it->tailConstBegin()->head().asSymbol() == "\"title\"") {
        text = Expression((it->tailConstEnd() - 1)->head().asSymbol());
        text.set_property(Atom("\"object-name\""), Expression(Atom("\"text\"")));
        text.set_property(Atom("\"text-scale\""), Expression(Atom(text_scale)));
        point.append(Atom(((right - left) / 2) + left));
        point.append(Atom(-upper - 3));
        text.set_property(Atom("\"position\""), point);
        result.append(text);
      }

      else if (it->tailConstBegin()->head().asSymbol() == "\"abscissa-label\"") {
        text = Expression((it->tailConstEnd() - 1)->head().asSymbol());
        text.set_property(Atom("\"object-name\""), Expression(Atom("\"text\"")));
        text.set_property(Atom("\"text-scale\""), Expression(Atom(text_scale)));
        point.append(Atom(((right - left) / 2) + left));
        point.append(Atom(-lower + 3));
        text.set_property(Atom("\"position\""), point);
        result.append(text);
      }

      else if (it->tailConstBegin()->head().asSymbol() == "\"ordinate-label\"") {
        text = Expression((it->tailConstEnd() - 1)->head().asSymbol());
        text.set_property(Atom("\"object-name\""), Expression(Atom("\"text\"")));
        text.set_property(Atom("\"text-scale\""), Expression(Atom(text_scale)));
        text.set_property(Atom("\"text-rotation\""), Expression(Atom(-std::atan2(0, -1) / 2)));
        point.append(Atom(left - 3));
        point.append(Atom(-1 * (((upper - lower) / 2) + lower)));
        text.set_property(Atom("\"position\""), point);
        result.append(text);
      }
    }
  }

  std::ostringstream out;
  out.precision(2);

  point = resetPoint;
  out.str("");
  out << y_max;
  text = Expression(Atom(out.str()));
  text.set_property(Atom("\"object-name\""), Expression(Atom("\"text\"")));
  text.set_property(Atom("\"text-scale\""), Expression(Atom(text_scale)));
  point.append(Atom(left - 2));
  point.append(Atom(-upper));
  text.set_property(Atom("\"position\""), point);
  result.append(text);

  point = resetPoint;
  out.str("");
  out << y_min;
  text = Expression(Atom(out.str()));
  text.set_property(Atom("\"object-name\""), Expression(Atom("\"text\"")));
  text.set_property(Atom("\"text-scale\""), Expression(Atom(text_scale)));
  point.append(Atom(left - 2));
  point.append(Atom(-lower));
  text.set_property(Atom("\"position\""), point);
  result.append(text);

  point = resetPoint;
  out.str("");
  out << x_max;
  text = Expression(Atom(out.str()));
  text.set_property(Atom("\"object-name\""), Expression(Atom("\"text\"")));
  text.set_property(Atom("\"text-scale\""), Expression(Atom(text_scale)));
  point.append(Atom(right));
  point.append(Atom(-lower + 2));
  text.set_property(Atom("\"position\""), point);
  result.append(text);

  point = resetPoint;
  out.str("");
  out << x_min;
  text = Expression(Atom(out.str()));
  text.set_property(Atom("\"object-name\""), Expression(Atom("\"text\"")));
  text.set_property(Atom("\"text-scale\""), Expression(Atom(text_scale)));
  point.append(Atom(left));
  point.append(Atom(-lower + 2));
  text.set_property(Atom("\"position\""), point);
  result.append(text);
 
  return result;
}


// this is a simple recursive version. the iterative version is more
// difficult with the ast data structure used (no parent pointer).
// this limits the practical depth of our AST
Expression Expression::eval(Environment & env) {

  // lookup only if tail is empty and the head is not list
  if (m_tail.empty() && m_head.asSymbol() != "list") {
    return handle_lookup(m_head, env);
  }
  // handle begin special-form
  else if (m_head.isSymbol() && m_head.asSymbol() == "begin") {
    return handle_begin(env);
  }
  // handle define special-form
  else if (m_head.isSymbol() && m_head.asSymbol() == "define") {
    return handle_define(env);
  }
  // handle lambda special-form
  else if (m_head.isSymbol() && m_head.asSymbol() == "lambda") {
    return handle_lambda(env);
  }
  // handle set-property special procedure
  else if (m_head.isSymbol() && m_head.asSymbol() == "set-property") {
    return handle_set_property(env);
  }
  // handle get-property special procedure
  else if (m_head.isSymbol() && m_head.asSymbol() == "get-property") {
    return handle_get_property(env);
  }
  // handle discrete-plot special procedure
  else if (m_head.isSymbol() && m_head.asSymbol() == "discrete-plot") {
    return handle_discrete_plot(env);
  }
  // handle continuous-plot special procedure
  else if (m_head.isSymbol() && m_head.asSymbol() == "continuous-plot") {
    return handle_continuous_plot(env);
  }
  // else attempt to treat as procedure
  else {
    std::vector<Expression> results;

    // lambda procedure -----------------------------------------------------------------------------------
    if (env.get_exp(m_head).head().asSymbol() == "lambda") {
      Expression params = env.get_exp(m_head).m_tail[0];
      Expression proc = env.get_exp(m_head).m_tail[1];

      // preconditions
      if (params.m_tail.size() == m_tail.size()) {
        // create shadow environment
        Environment shadow = env;

        int count = 0;

        // link respective values to parameter variables
        for (auto it = params.tailConstBegin(); it != params.tailConstEnd(); ++it) {
          Expression singleparam(Atom("define"));
          singleparam.append(*it);
          singleparam.append(m_tail[count]);
          singleparam.eval(shadow);
          ++count;
          }

        // evaluate lambda procedure with passed paramter values
        Expression result = proc.eval(shadow);

        // copy over properties from overall lambda to result if any
        if (!env.get_exp(m_head).propmap.empty()) result.propmap = env.get_exp(m_head).propmap;

        return result;
        }
      else {
        throw SemanticError("Error in call to lambda procedure: invalid number of arguments");
        }
      }

    // apply procedure -----------------------------------------------------------------------------------
    else if (m_head.isSymbol() && m_head.asSymbol() == "apply") {

      // preconditions
      if ((env.is_proc(m_tail[0].head()) && (m_tail[0].tailConstBegin() == m_tail[0].tailConstEnd())) ||
        env.get_exp(m_tail[0].head()).head().asSymbol() == "lambda") {
        if (m_tail[1].isList()) {

          // move procedure and arguments into evaluable expression and evaluate
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

    // map procedure -----------------------------------------------------------------------------------
    else if (m_head.isSymbol() && m_head.asSymbol() == "map") {

      // preconditions
      if ((env.is_proc(m_tail[0].head()) && (m_tail[0].tailConstBegin() == m_tail[0].tailConstEnd())) ||
        env.get_exp(m_tail[0].head()).head().asSymbol() == "lambda") {

        // evaluate each argument according to procedure and place in result list
        Atom proc = m_tail[0].head();
        Expression args = m_tail[1].eval(env);
        Expression result(Atom("list"));

        for (auto it = args.tailConstBegin(); it != args.tailConstEnd(); ++it) {
          Expression temp(proc);
          temp.append(*it);
          result.append(temp.eval(env));
        }
        
        if (m_tail[1].isList() || (args.isList() && args.tailConstBegin() != args.tailConstEnd())) {
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

    // basic procedures -----------------------------------------------------------------------------------
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

  if ((exp.head().isNone() || exp.isList()) && (exp.tailConstBegin() == exp.tailConstEnd())){ 
    out << "NONE";
    return out;
  }

  if(!complex) out << "(";

  // prevent showing heads for list or lambda expressions
  if (!exp.isList() && !exp.isLambda()) { 
    out << exp.head();

    // display procedure heads correctly
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
