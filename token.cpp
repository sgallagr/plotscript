#include "token.hpp"

// system includes
#include <cctype>
#include <iostream>

// define constants for special characters
const char OPENCHAR = '(';
const char CLOSECHAR = ')';
const char COMMENTCHAR = ';';
const char LITERALCHAR = '"';

Token::Token(TokenType t): m_type(t){}

Token::Token(const std::string & str): m_type(STRING), value(str) {}

Token::TokenType Token::type() const{
  return m_type;
}

std::string Token::asString() const{
  switch(m_type){
    case OPEN:
      return "(";
    case CLOSE:
      return ")";
    case STRING:
      return value;
  }
  return "";
}


// add token to sequence unless it is empty, clears token
void store_ifnot_empty(std::string & token, TokenSequenceType & seq){
  if(!token.empty()){
    seq.emplace_back(token);
    token.clear();
  }
}

TokenSequenceType tokenize(std::istream & seq){
  TokenSequenceType tokens;
  std::string token;
  
  while(true){
    char c = seq.get();
    if(seq.eof()) break;
    
    if(c == COMMENTCHAR){
      // chomp until the end of the line
      while((!seq.eof()) && (c != '\n')){
        c = seq.get();
      }
      if(seq.eof()) break;
    }
    else if(c == OPENCHAR){
      store_ifnot_empty(token, tokens);
      tokens.push_back(Token::TokenType::OPEN);
    }
    else if(c == CLOSECHAR){
      store_ifnot_empty(token, tokens);
      tokens.push_back(Token::TokenType::CLOSE);
    }
    else if (c == LITERALCHAR) {
      std::string temp = "string";
      store_ifnot_empty(temp, tokens);
      token.push_back(c);

      // store entire string in one token
      while((!seq.eof()) && (seq.peek() != LITERALCHAR)){
        c = seq.get();
        token.push_back(c);
      }

      if (!seq.eof()) {
        c = seq.get();
        token.push_back(c);
        store_ifnot_empty(token, tokens);
      }
      else break;
    }
    else if(isspace(c)){
      store_ifnot_empty(token, tokens);
    }
    else{
      token.push_back(c);
    }
  }
  store_ifnot_empty(token, tokens);

  return tokens;
}
