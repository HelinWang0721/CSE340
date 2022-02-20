#ifndef __format__H__
#define __format__H__

#include <vector>
#include <string>
#include <map>
#include <iostream>

#include "lexer.h"

#define CLASS_NAME_ATTR "class_name"

using namespace std;

namespace preEval{

class prototype { // determine if is scope or line
public:
    bool isScope;
    virtual void Print(int);
    virtual ~prototype() {}
};

class Scope: public prototype { // A block of code belong to same scope
public:
    
	vector<prototype*> items;
    map<string, string> attr;
    Scope() {prototype::isScope = true;}
    void destory();
    void push(prototype*);
    prototype* read(int index) { return items[index]; }
    prototype* operator [] (int index) { return items[index]; }
    int size() { return items.size(); }
    void addAttr(string, string);
    void set(int i, prototype* itemPtr) {items[i] = itemPtr;}
    void Print(int indent);
    string getAttr(string);
    bool hasAttr(string key) { return attr.count(key) != 0; }
    ~Scope() {}
};

class Line: public prototype{
public:
    vector<Token> tokens;
    Line() {prototype::isScope = false;}
    Line(vector<Token>);

    ~Line() {}

    virtual void Print(int indent);
};

typedef enum { FINISHED = 0, SYNTAX_ERROR
} Analysis_Status;

class FormatAnalyzer {
public: 
    Scope globalScope;
    FormatAnalyzer(vector<Token> tokenList);
    Analysis_Status scan(); // starting point of format scan

private:
    vector<Token> tokenList;
};

}

#endif