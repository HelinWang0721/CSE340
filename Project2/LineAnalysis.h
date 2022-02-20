#pragma once
#ifndef __LineAnalysis__H__
#define __LineAnalysis__H__


#include "format.h"

using namespace preEval;

class Declaration;
class Equation;

class LineAnalysis {
public:
	LineAnalysis(FormatAnalyzer&);
	Analysis_Status scan(); // entry
    Scope globalScope;
    
private:
	
    Analysis_Status scan(Scope*);
    bool isExpression(Line* line);
    Analysis_Status declLine(Line* line, Declaration** newDeclaration);
    Analysis_Status leftORright(Line* line, Equation** newEquation);
};

class ProcessLine: public Line {
public:

    ProcessLine(Line&);
	ProcessLine() {isScope = false;}
    bool isEquation; // equation or declaration only

    virtual void Print(int i) {}
};


class Equation: public ProcessLine {
public: 
    vector<Token> leftIDs;
	vector<Token> rightExpression;
	Equation(){}
    Equation(vector<Token>, vector<Token>, Line*);

    void Print(int indent);
};

class Declaration: public ProcessLine {
public:
	vector<Token> declTokens;
	Declaration() { this-> isScope = false; }
	Declaration(Line*, vector<Token>);

    void Print(int indent);
};

#endif