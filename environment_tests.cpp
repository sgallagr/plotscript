#include "catch.hpp"

#include "environment.hpp"
#include "semantic_error.hpp"

#include <cmath>

TEST_CASE( "Test default constructor", "[environment]" ) {

  Environment env;

  REQUIRE(env.is_known(Atom("pi")));
  REQUIRE(env.is_exp(Atom("pi")));

  REQUIRE(env.is_known(Atom("e")));
  REQUIRE(env.is_exp(Atom("e")));

  REQUIRE(env.is_known(Atom("I")));
  REQUIRE(env.is_exp(Atom("I")));

  REQUIRE(!env.is_known(Atom("hi")));
  REQUIRE(!env.is_exp(Atom("hi")));

  REQUIRE(env.is_proc(Atom("+")));
  REQUIRE(env.is_proc(Atom("-")));
  REQUIRE(env.is_proc(Atom("*")));
  REQUIRE(env.is_proc(Atom("/")));
  REQUIRE(env.is_proc(Atom("sqrt")));
  REQUIRE(env.is_proc(Atom("^")));
  REQUIRE(env.is_proc(Atom("ln")));
  REQUIRE(env.is_proc(Atom("sin")));
  REQUIRE(env.is_proc(Atom("cos")));
  REQUIRE(env.is_proc(Atom("tan")));
  REQUIRE(env.is_proc(Atom("real")));
  REQUIRE(env.is_proc(Atom("imag")));
  REQUIRE(env.is_proc(Atom("mag")));
  REQUIRE(env.is_proc(Atom("arg")));
  REQUIRE(env.is_proc(Atom("conj")));
  REQUIRE(!env.is_proc(Atom("op")));
}

TEST_CASE( "Test get expression", "[environment]" ) {
  Environment env;

  REQUIRE(env.get_exp(Atom("pi")) == Expression(std::atan2(0, -1)));
  REQUIRE(env.get_exp(Atom("hi")) == Expression());
}

TEST_CASE( "Test add expression", "[environment]" ) {
  Environment env;

  Expression a(Atom(1.0));
  env.add_exp(Atom("one"), a);
  REQUIRE(env.is_known(Atom("one")));
  REQUIRE(env.is_exp(Atom("one")));
  REQUIRE(env.get_exp(Atom("one")) == a);

  Expression b(Atom("hello"));
  env.add_exp(Atom("hi"), b);
  REQUIRE(env.is_known(Atom("hi")));
  REQUIRE(env.is_exp(Atom("hi")));
  REQUIRE(env.get_exp(Atom("hi")) == b);

  REQUIRE_THROWS_AS(env.add_exp(Atom(1.0), b), SemanticError);
}

TEST_CASE( "Test get built-in procedure", "[environment]" ) {
  Environment env;

  INFO("default procedure")
  Procedure p1 = env.get_proc(Atom("doesnotexist"));
  Procedure p2 = env.get_proc(Atom("alsodoesnotexist"));
  REQUIRE(p1 == p2);
  std::vector<Expression> args;
  REQUIRE(p1(args) == Expression());
  REQUIRE(p2(args) == Expression());

  INFO("trying add procedure")
  Procedure padd = env.get_proc(Atom("+"));
  args.emplace_back(1.0);
  args.emplace_back(2.0);
  REQUIRE(padd(args) == Expression(3.0));

  INFO("trying complex add procedure")
  args.clear();
  std::complex<double> a(0,1);
  std::complex<double> b(0,2);
  std::complex<double> c(0,3);
  args.emplace_back(a);
  args.emplace_back(b);
  REQUIRE(padd(args) == Expression(c));

  INFO("trying add procedure with one argument")
  args.clear();
  args.emplace_back(1.0);
  REQUIRE_THROWS_AS(padd(args), SemanticError);

  INFO("trying complex mul procedure")
  args.clear();
  padd = env.get_proc(Atom("*"));
  c.real(-2);
  c.imag(0);
  args.emplace_back(a);
  args.emplace_back(b);
  REQUIRE(padd(args) == Expression(c));
}

TEST_CASE( "Test reset", "[environment]" ) {
  Environment env;

  Expression a(Atom(1.0));
  env.add_exp(Atom("one"), a);
  Expression b(Atom("hello"));
  env.add_exp(Atom("hi"), b);

  env.reset();
  REQUIRE(!env.is_known(Atom("one")));
  REQUIRE(!env.is_exp(Atom("one")));
  REQUIRE(env.get_exp(Atom("one")) == Expression());
  REQUIRE(!env.is_known(Atom("hi")));
  REQUIRE(!env.is_exp(Atom("hi")));
  REQUIRE(env.get_exp(Atom("hi")) == Expression());
}

TEST_CASE( "Test semeantic errors", "[environment]" ) {

  Environment env;

  {
    Expression exp(Atom("begin"));
    
    REQUIRE_THROWS_AS(exp.eval(env), SemanticError);
  }
}

