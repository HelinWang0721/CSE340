/*
 * Copyright (C) Rida Bazzi, 2016
 *
 * Do not share this file with anyone
 */
#ifndef __LEXER__H__
#define __LEXER__H__

#include <vector>
#include <string>

#include "inputbuf.h"

// ------- token types -------------------

typedef enum { END_OF_FILE = 0,
    IF, WHILE, DO, THEN, PRINT,PUBLIC,PRIVATE,
    PLUS, MINUS, DIV, MULT,
    EQUAL, COLON, COMMA, SEMICOLON,
    LBRAC, RBRAC, LPAREN, RPAREN,
    NOTEQUAL, GREATER, LESS, LTEQ, GTEQ,
    DOT, NUM, ID, ERROR, REALNUM, BASE08NUM, BASE16NUM, COMMENT, LBRACE, RBRACE // TODO: Add labels for new token types here
} TokenType;

class Token {
  public:
    void Print();

    std::string lexeme;
    TokenType token_type;
    int line_no;
};

class LexicalAnalyzer {
  public:
    std::vector<Token> tokenList;
    InputBuffer input;

    Token GetToken();
    TokenType UngetToken(Token);
    LexicalAnalyzer();
    //void readFile(std::string fileAbsPath);

  private:
    std::vector<Token> tokens;
    int line_no;
    Token tmp;
    

    bool SkipSpace();
    bool IsKeyword(std::string);
    TokenType FindKeywordIndex(std::string);
    Token ScanIdOrKeyword();
    Token ScanNumber();
    Token ScanRealNumber();
    Token ScanBASE08NUM();
    Token ScanBASE16NUM();
    Token scanComment();
};


std::string getIndent(int indent);


#endif  //__LEXER__H__
