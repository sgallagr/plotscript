#include "catch.hpp"

#include <string>
#include <sstream>
#include <fstream>
#include <iostream>

#include "semantic_error.hpp"
#include "interpreter.hpp"
#include "threadsafequeue.hpp"
#include "expression.hpp"

Expression run(const std::string & program){
  
  std::istringstream iss(program);
    
  Interpreter interp;
    
  bool ok = interp.parseStream(iss);
  if(!ok){
    std::cerr << "Failed to parse: " << program << std::endl; 
  }
  REQUIRE(ok == true);

  Expression result;
  REQUIRE_NOTHROW(result = interp.evaluate());

  return result;
}

TEST_CASE( "Test Interpreter parser with expected input", "[interpreter]" ) {

  std::string program = "(begin (define r 10) (* pi (* r r)))";

  std::istringstream iss(program);
 
  Interpreter interp;

  bool ok = interp.parseStream(iss);

  REQUIRE(ok == true);
}

TEST_CASE( "Test secondary Interpreter constructor using expected input", "[interpreter]" ) {

  std::string program = "(begin (define r 10) (* pi (* r r)))";

  std::istringstream iss(program);
 
  ThreadSafeQueue<std::string> str;
  ThreadSafeQueue<Expression> exp;

  Interpreter interp(&str, &exp);

  bool ok = interp.parseStream(iss);

  REQUIRE(ok == true);
}

TEST_CASE( "Test Interpreter parser with numerical literals", "[interpreter]" ) {

  std::vector<std::string> programs = {"(1)", "(+1)", "(+1e+0)", "(1e-0)"};
  
  for(auto program : programs){
    std::istringstream iss(program);
 
    Interpreter interp;

    bool ok = interp.parseStream(iss);

    REQUIRE(ok == true);
  }

  {
    std::istringstream iss("(define x 1abc)");
    
    Interpreter interp;

    bool ok = interp.parseStream(iss);

    REQUIRE(ok == false);
  }
}

TEST_CASE( "Test Interpreter parser with truncated input", "[interpreter]" ) {

  {
    std::string program = "(f";
    std::istringstream iss(program);
  
    Interpreter interp;
    bool ok = interp.parseStream(iss);
    REQUIRE(ok == false);
  }
  
  {
    std::string program = "(begin (define r 10) (* pi (* r r";
    std::istringstream iss(program);

    Interpreter interp;
    bool ok = interp.parseStream(iss);
    REQUIRE(ok == false);
  }
}

TEST_CASE( "Test Interpreter parser with extra input", "[interpreter]" ) {

  std::string program = "(begin (define r 10) (* pi (* r r))) )";
  std::istringstream iss(program);

  Interpreter interp;

  bool ok = interp.parseStream(iss);

  REQUIRE(ok == false);
}

TEST_CASE( "Test Interpreter parser with single non-keyword", "[interpreter]" ) {

  std::string program = "hello";
  std::istringstream iss(program);
  
  Interpreter interp;

  bool ok = interp.parseStream(iss);

  REQUIRE(ok == false);
}

TEST_CASE( "Test Interpreter parser with empty input", "[interpreter]" ) {

  std::string program;
  std::istringstream iss(program);
  
  Interpreter interp;

  bool ok = interp.parseStream(iss);

  REQUIRE(ok == false);
}

TEST_CASE( "Test Interpreter parser with empty expression", "[interpreter]" ) {

  std::string program = "( )";
  std::istringstream iss(program);
  
  Interpreter interp;

  bool ok = interp.parseStream(iss);

  REQUIRE(ok == false);
}

TEST_CASE( "Test Interpreter parser with bad number string", "[interpreter]" ) {

  std::string program = "(1abc)";
  std::istringstream iss(program);
  
  Interpreter interp;

  bool ok = interp.parseStream(iss);

  REQUIRE(ok == false);
}

TEST_CASE( "Test Interpreter parser with incorrect input. Regression Test", "[interpreter]" ) {

  std::string program = "(+ 1 2) (+ 3 4)";
  std::istringstream iss(program);
  
  Interpreter interp;

  bool ok = interp.parseStream(iss);

  REQUIRE(ok == false);
}

TEST_CASE( "Test Interpreter result with literal expressions", "[interpreter]" ) {
  
  { // Number
    std::string program = "(4)";
    Expression result = run(program);
    REQUIRE(result == Expression(4.));
  }

  { // Symbol
    std::string program = "(pi)";
    Expression result = run(program);
    REQUIRE(result == Expression(atan2(0, -1)));
  }

}

TEST_CASE( "Test Interpreter result with simple procedures (add)", "[interpreter]" ) {

  { // add, binary case
    std::string program = "(+ 1 2)";
    INFO(program);
    Expression result = run(program);
    REQUIRE(result == Expression(3.));
  }
  
  { // add, 3-ary case
    std::string program = "(+ 1 2 3)";
    INFO(program);
    Expression result = run(program);
    REQUIRE(result == Expression(6.));
  }

  { // add, 6-ary case
    std::string program = "(+ 1 2 3 4 5 6)";
    INFO(program);
    Expression result = run(program);
    REQUIRE(result == Expression(21.));
  }
}
  
TEST_CASE( "Test Interpreter special forms: begin and define", "[interpreter]" ) {

  {
    std::string program = "(define answer 42)";
    INFO(program);
    Expression result = run(program);
    REQUIRE(result == Expression(42.));
  }

  {
    std::string program = "(begin (define answer 42)\n(answer))";
    INFO(program);
    Expression result = run(program);
    REQUIRE(result == Expression(42.));
  }
  
  {
    std::string program = "(begin (define answer (+ 9 11)) (answer))";
    INFO(program);
    Expression result = run(program);
    REQUIRE(result == Expression(20.));
  }

  {
    std::string program = "(begin (define a 1) (define b 1) (+ a b))";
    INFO(program);
    Expression result = run(program);
    REQUIRE(result == Expression(2.));
  }
}

TEST_CASE( "Test a medium-sized expression", "[interpreter]" ) {

  {
    std::string program = "(+ (+ 10 1) (+ 30 (+ 1 1)))";
    Expression result = run(program);
    REQUIRE(result == Expression(43.));
  }
}

TEST_CASE( "Test number procedures", "[interpreter]" ) {

  {
    std::vector<std::string> programs = {"(+ 1 -2)",
           "(+ -3 1 1)",
           "(- 1)",
           "(- 1 2)",
           "(* 1 -1)",
           "(* 1 1 -1)",
           "(/ -1 1)",
           "(/ 1 -1)",
           "(- 0 (^ 8 0))",
             "(- (ln 1) 1)",
           "(- (sin 0) 1)",
           "(- 0 (cos 0))",
           "(- (tan 0) 1)"};

    for(auto s : programs){
      Expression result = run(s);
      REQUIRE(result == Expression(-1.));
    }
  }
}

TEST_CASE( "Test complex procedures", "[interpreter]" ) {
  std::complex<double> ans(0.,0.);
  Expression result;

  std::string program;
  
  INFO("trying complex add and negate")
  program = "(+ I (- I))";
  result = run(program);
  REQUIRE(result == Expression(ans));

  INFO("trying complex multiply")
  program = "(- (* I I) (* I I))";
  result = run(program);
  REQUIRE(result == Expression(ans));

  INFO("trying complex subtract 1")
  program =  "(- I I)";
  result = run(program);
  REQUIRE(result == Expression(ans));

  INFO("trying complex subtract 2")
  program = "(- I 0)";
  result = run(program);
  ans.imag(1.);
  REQUIRE(result == Expression(ans));

  INFO("trying complex subtract 3")
  program = "(- 0 I)";
  result = run(program);
  ans.imag(-1.);
  REQUIRE(result == Expression(ans));

  INFO("trying complex divide 1")
  program =  "(/ I I)";
  result = run(program);
  ans.real(1.);
  ans.imag(0.);
  REQUIRE(result == Expression(ans));

  INFO("trying complex divide 2")
  program = "(/ I 1)";
  result = run(program);
  ans.real(0.);
  ans.imag(1.);
  REQUIRE(result == Expression(ans));

  INFO("trying complex divide 3")
  program = "(/ 1 I)";
  result = run(program);
  ans.imag(-1.);
  REQUIRE(result == Expression(ans));

  INFO("trying complex divide 4")
  program = "(/ I)";
  result = run(program);
  ans.imag(-1.);
  REQUIRE(result == Expression(ans));

  INFO("trying real")
  program = "(real I)";
  result = run(program);
  REQUIRE(result == Expression(0.));

  INFO("trying imag")
  program = "(imag I)";
  result = run(program);
  REQUIRE(result == Expression(1.));

  INFO("trying mag")
  program = "(mag I)";
  result = run(program);
  REQUIRE(result == Expression(1.));

  INFO("trying arg")
  program = "(- (arg I) (arg I))";
  result = run(program);
  REQUIRE(result == Expression(0.));

  INFO("trying conj")
  program = "(conj I)";
  result = run(program);
  ans.real(0.);
  ans.imag(-1.);
  REQUIRE(result == Expression(ans));

  ans.real(0);
  ans.imag(0);

  INFO("trying sqrt")
  program = "(- (sqrt I) (sqrt I))";
  result = run(program);
  REQUIRE(result == Expression(ans));

  INFO("trying sqrt of -1")
  program = "(sqrt -1)";
  ans.imag(1);
  result = run(program);
  REQUIRE(result == Expression(ans));

  ans.imag(0);

  INFO("trying complex power 1")
  program =  "(- (^ I I) (^ I I))";
  result = run(program);
  REQUIRE(result == Expression(ans));

  INFO("trying complex power 2")
  program = "(- (^ I 1) (^ I 1))";
  result = run(program);
  REQUIRE(result == Expression(ans));

  INFO("trying complex power 3")
  program = "(- (^ 1 I) (^ 1 I))";
  result = run(program);
  REQUIRE(result == Expression(ans));
}

TEST_CASE( "Test some semantically invalid expresions", "[interpreter]" ) {
  
  std::vector<std::string> programs = {"(@ none)", // no such procedure
             "(+ 1)", // too few arguments
             "(* 1)",
             "(append 1)",
             "(join 1)",
             "(range 1)",
             "(set-property \"test\")",
             "(lambda (x))",
             "(get-property \"test\")",
             "(continuous-plot 1)",
             "(+ 1 (list 1 2 3))", // invalid argument
             "(* 1 (list 1 2 3))",
             "(- 1 (list 1 2 3))",
             "(/ 1 (list 1 2 3))",
             "(sqrt (list 1 2 3))",
             "(^ (list 1) (list 2))",
             "(ln (list 1 2 3))",
             "(first 1)",
             "(first (list))",
             "(rest 1)",
             "(rest (list))", 
             "(length 1)",
             "(append (list 1 2 3) (lambda (x y) (* x y)))",
             "(append 1 2)",
             "(join 1 (list 1 2 3))",
             "(join (list 1 2 3) 1)",
             "(range 0 1 -1)",
             "(range 1 1 1)",
             "(range 1 1 I)",
             "(list 1 2 (lambda (x) (* 2 x)))",
             "(define 3 4)",
             "(lambda (3) (* 3 3))",
             "(lambda (begin) (* 2 2))",
             "(lambda (+) (+ 2 2))",
             "(apply 3 (list 1 2 3))",
             "(apply + 3)",
             "(map + 3)",
             "(begin)",
             "(set-property 1 2 3)",
             "(lambda (a 2 3) (+ 1 2 3 4 5))",
             "(get-property 1 2)",
             "(apply x 2)",
             "(& 1 2)",
             "(define begin 1)", // redefine special form
             "(define map 1)",   // redefine built-in procedure
             "(- 1 1 2)", // too many arguments
             "(/ 1 2 3)",
             "(sqrt 1 2)",
             "(^ 4 5 6)",
             "(ln 4 5)",
             "(sin 2 3)",
             "(cos 6 7)",
             "(tan 8 9)",
             "(real I I)",
             "(imag I I)",
             "(mag I I)",
             "(arg I I)",
             "(conj I I)",
             "(first (list 1) (list 2))",
             "(rest (list 1) (list 2))",
             "(length (list 1) (list 2))",
             "(sin I)",	// invalid arguments
             "(cos I)",
             "(tan I)",
             "(real 3)",
             "(imag 4)",
             "(mag 5)",
             "(arg 6)",
             "(conj 7)"};
    for(auto s : programs){
      Interpreter interp;

      std::istringstream iss(s);
      
      bool ok = interp.parseStream(iss);
      REQUIRE(ok == true);
      
      REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
    }
}

TEST_CASE("Test lambda procedure", "[interpreter]") {
  double ans = 4;
  Expression result;

  INFO("trying to run lambda procedure with too many arguments")

  Interpreter interp;

  std::istringstream iss("(define f (lambda (x) (* 2 x)))");
  bool ok = interp.parseStream(iss);
  REQUIRE(ok == true);
  interp.evaluate();

  iss.clear();
  iss.str("(f 2 3)");
  ok = interp.parseStream(iss);
  REQUIRE(ok == true);

  REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);


  INFO("trying to run lambda procedure")

  iss.clear();
  iss.str("(f 2)");
  ok = interp.parseStream(iss);
  REQUIRE(ok == true);

  result = interp.evaluate();

  REQUIRE(result == Expression(4.));
}

TEST_CASE("Test list procedures", "[interpreter]") {
  Expression result;

  std::string program;

  INFO("trying first")
  program = "(first (list 1 2 3))";
  result = run(program);
  REQUIRE(result == Expression(1.));

  INFO("trying rest")
  program = "(first (rest (list 2 1)))";
  result = run(program);
  REQUIRE(Expression(result) == Expression(1.));

  INFO("trying length with empty list")
  program = "(length (list))";
  result = run(program);
  REQUIRE(result == Expression(0.));

  INFO("trying length")
  program = "(length (list 1 2 3))";
  result = run(program);
  REQUIRE(result == Expression(3.));

  INFO("trying append")
  program = "(length (append (list 1) (list 1)))";
  result = run(program);
  REQUIRE(result == Expression(2.));

  INFO("trying join")
  program = "(length (join (list 1) (list 2)))";
  result = run(program);
  REQUIRE(result == Expression(2.));

  INFO("trying range")
  program = "(length (range 1 2 1))";
  result = run(program);
  REQUIRE(result == Expression(2.));
}

TEST_CASE("Test apply and map procedures", "[interpreter]") {
  Expression result;

  std::string program;

  INFO("trying apply")
  program = "(apply + (list 1 2 3))";
  result = run(program);
  REQUIRE(result == Expression(6.));

  INFO("trying map")
  program = "(length (map / (list 1 2 3)))";
  result = run(program);
  REQUIRE(Expression(result) == Expression(3.));
}

TEST_CASE( "Test for exceptions from semantically incorrect input", "[interpreter]" ) {

  std::string input = R"(
(+ 1 a)
)";

  Interpreter interp;
  
  std::istringstream iss(input);
  
  bool ok = interp.parseStream(iss);
  REQUIRE(ok == true);
  
  REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
}

TEST_CASE( "Test malformed define", "[interpreter]" ) {

    std::string input = R"(
(define a 1 2)
)";

  Interpreter interp;
  
  std::istringstream iss(input);
  
  bool ok = interp.parseStream(iss);
  REQUIRE(ok == true);
  
  REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
}

TEST_CASE( "Test using number as procedure", "[interpreter]" ) {
    std::string input = R"(
(1 2 3)
)";

  Interpreter interp;
  
  std::istringstream iss(input);
  
  bool ok = interp.parseStream(iss);
  REQUIRE(ok == true);
  
  REQUIRE_THROWS_AS(interp.evaluate(), SemanticError);
}

TEST_CASE("Test set and get-property procdedures", "[interpreter]") {
  Expression result;
  Interpreter interp;

  std::string program;

  INFO("trying set-property")
  program = "(define x (set-property \"test\" 1 (+ 1 2)))";

  std::istringstream iss(program);
 
  bool ok = interp.parseStream(iss);
  if(!ok){
    std::cerr << "Failed to parse: " << program << std::endl; 
  }
  REQUIRE(ok == true);

  REQUIRE_NOTHROW(interp.evaluate());


  INFO("trying get-property")
  program = "(get-property \"test\" x)";

  iss.clear();
  iss.str(program);
    
  ok = interp.parseStream(iss);
  if(!ok){
    std::cerr << "Failed to parse: " << program << std::endl; 
  }
  REQUIRE(ok == true);

  REQUIRE_NOTHROW(result = interp.evaluate());

  REQUIRE(Expression(result) == Expression(1));

}

TEST_CASE("Test discrete-plot procdedure", "[interpreter]") {
  Expression result;
  Interpreter interp;

  std::string program;

  INFO("trying to define lambda")
  program = "(define f (lambda (x) (list x (+ (* 2 x) 1))))";

  std::istringstream iss(program);
 
  bool ok = interp.parseStream(iss);
  if(!ok){
    std::cerr << "Failed to parse: " << program << std::endl; 
  }
  REQUIRE(ok == true);

  REQUIRE_NOTHROW(interp.evaluate());


  INFO("trying discrete-plot")
  program = "(discrete-plot (map f (range -2 2 0.5)) (list (list \"title\" \"Test\") (list \"abscissa-label\" \"X\") (list \"ordinate-label\" \"Y\") (list \"text-scale\" 1)))";

  iss.clear();
  iss.str(program);
    
  ok = interp.parseStream(iss);
  if(!ok){
    std::cerr << "Failed to parse: " << program << std::endl; 
  }
  REQUIRE(ok == true);

  REQUIRE_NOTHROW(result = interp.evaluate());

  REQUIRE(result.tailSize() == 31);

}

TEST_CASE("Test continuous-plot procdedure", "[interpreter]") {
  Expression result;
  Interpreter interp;

  std::string program;

  INFO("trying to define lambda")
  program = "(define f (lambda (x) (+ (* 2 x) 1)))";

  std::istringstream iss(program);
 
  bool ok = interp.parseStream(iss);
  if(!ok){
    std::cerr << "Failed to parse: " << program << std::endl; 
  }
  REQUIRE(ok == true);

  REQUIRE_NOTHROW(interp.evaluate());


  INFO("trying continuous-plot")
  program = "(continuous-plot f (list -2 2) (list (list \"title\" \"Test\") (list \"abscissa-label\" \"X\") (list \"ordinate-label\" \"Y\") (list \"text-scale\" 1)))";

  iss.clear();
  iss.str(program);
    
  ok = interp.parseStream(iss);
  if(!ok){
    std::cerr << "Failed to parse: " << program << std::endl; 
  }
  REQUIRE(ok == true);

  REQUIRE_NOTHROW(result = interp.evaluate());

  REQUIRE(result.tailSize() == 63);

}
