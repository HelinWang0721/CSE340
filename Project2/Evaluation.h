//#pragma once
#ifndef __EVALUATION__H__
#define __EVALUATION__H__

#include "LineAnalysis.h"

class Runtime {
public:
    class Scope;
    class Variable;

    class Scope {
    public:
        bool hasVar(string id, Runtime::Scope*);
        bool addVar(Runtime::Variable* var);
        bool addScope(Runtime::Scope*);

        bool hasParent() { return this->parentScope != NULL; }

        Scope() { this->parentScope = NULL; }
        Scope(string className, Runtime::Scope* parentScope) {
            this->name = className;
            this->parentScope = parentScope;
        }

        string name;
        Runtime::Scope* parentScope;

        vector<Runtime::Variable*> varList;
        vector<Runtime::Scope*> scopeList;
    };

    class Variable {
    public:
        bool isPublic = true;
        Runtime::Scope* parentScope;
        string id;

        Variable(string id, bool isPubilc, Runtime::Scope* parentScope);

        string getID() { return this->id; }
    };

    Runtime::Scope* currentScope;
    Runtime::Scope* runtime_globalScope;

    Runtime();

    bool createScope(preEval::Scope& scope);
    bool createVar(string& id, bool isPubilc = true);
    string getCurrentScopeName();
    bool hasVar(string id);
    string getVarParentScope(string id);

    vector<string> expressionOutputList;
    void generateEvalOutput(Equation&);
};
class Evaluation {

public:
    Evaluation(LineAnalysis& la) { this->globalScope = &la.globalScope; }
    Scope* globalScope;
    Analysis_Status eval();
    void printExpressionOutput();
private: 
    Analysis_Status eval(preEval::Scope& scope);
    Runtime runtime;
};

#endif