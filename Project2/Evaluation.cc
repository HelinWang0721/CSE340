#include "Evaluation.h"

Analysis_Status Evaluation::eval() {
    this->runtime = Runtime();
    return eval(*globalScope);
}

Analysis_Status Evaluation::eval(preEval::Scope& scope) {

    for (int i = 0;i < scope.size(); i++) {
        if (scope[i]->isScope) {
            Scope* scopePtr = (preEval::Scope*)scope[i];
            if (!runtime.createScope(*scopePtr)) {
                return SYNTAX_ERROR;
            }

            eval(*scopePtr);
        }
        else {
            ProcessLine* linePrt = (ProcessLine*)(scope[i]);

            if (!linePrt->isEquation) {  // is Declaration
                Declaration* declLinePtr = (Declaration*)linePrt;
                bool isPubilc = true;
                int lineTokenIndex = 0;
                if (declLinePtr->declTokens[lineTokenIndex].token_type == PRIVATE) {
                    isPubilc = false;
                    lineTokenIndex++;
                }
                else if (declLinePtr->declTokens[lineTokenIndex].token_type == PUBLIC)
                    lineTokenIndex++;
                for (; lineTokenIndex < declLinePtr->declTokens.size(); lineTokenIndex++) {
                    if (!runtime.createVar(declLinePtr->declTokens[lineTokenIndex].lexeme, isPubilc))
                        return SYNTAX_ERROR;
                }
            }
            else { // is an equation
                Equation* equaLinePtr = (Equation*)linePrt;

                runtime.generateEvalOutput(*equaLinePtr);
            }
        }
    }

    if (runtime.currentScope->hasParent()) {  // let runtime jump out the current scope
        runtime.currentScope = runtime.currentScope->parentScope;
    }
}


void Evaluation::printExpressionOutput() {

    for (int i = 0; i < runtime.expressionOutputList.size(); i++) {
        cout << runtime.expressionOutputList[i] << endl;
    }
}

void Runtime::generateEvalOutput(Equation& equalLine) {
    string output = "";
    for (int i = 0; i < equalLine.leftIDs.size(); i++) {
        string id = equalLine.leftIDs[i].lexeme;
        if (this->hasVar(id)) {
            string name = this->getVarParentScope(id);
            if (name == "::") 
                output = output + "::" + id;
            else
                output = output + name + "." + id;
        }
        else {
            output = output + "?." + id;
        }
        output = output + " ";
    }
    output = output + "= ";
    string rightValueId = equalLine.rightExpression[0].lexeme;

    if (this->hasVar(rightValueId)) {
        string name = this->getVarParentScope(rightValueId);
        if (name == "::") 
            output = output + "::" + rightValueId;
        else
            output = output + name + "." + rightValueId;
    }
    else {
        output = output + "?." + rightValueId;
    }
    this->expressionOutputList.push_back(output);
}

Runtime::Runtime() {
    this->currentScope = new Runtime::Scope();
    this->runtime_globalScope = this->currentScope;
}


bool Runtime::createScope(preEval::Scope& scope) {
    Runtime::Scope* newScope = new Runtime::Scope();
    if (scope.hasAttr(CLASS_NAME_ATTR)) {
        newScope->name = scope.getAttr(CLASS_NAME_ATTR);
    }
    newScope->parentScope = this->currentScope;
    bool status = this->currentScope->addScope(newScope);
    if (status)                                                 //move the scope pointer to the next scope
        this->currentScope = newScope;
    return status;
}

bool Runtime::createVar(string& id, bool isPubilc) {
    Runtime::Variable* newVar = new Runtime::Variable(id, isPubilc, this->currentScope); //Variable(string id, bool isPubilc, Runtime::Scope* parentScope);
    return this->currentScope->addVar(newVar);
}

string Runtime::getCurrentScopeName() {
    return this->currentScope->name;
}

bool Runtime::hasVar(string varID) {
    Runtime::Scope* targetScope = this->currentScope;
    while (true) {
        if (targetScope->hasVar(varID, this->currentScope)) {
            return true;
        }
        if (!targetScope->hasParent())
            break;
        targetScope = targetScope->parentScope;
    }
    return false;
}

string Runtime::getVarParentScope(string varID) {
    Runtime::Scope* targetScope = this->currentScope;
    while (true) {
        if (targetScope->hasVar(varID, this->currentScope)) {
            if (this->runtime_globalScope == targetScope)
                return "::";
            else
                return targetScope->name;
        }
        if (!targetScope->hasParent())
            break;
        targetScope = targetScope->parentScope;
    }
    return "";
}

bool Runtime::Scope::hasVar(string id, Runtime::Scope* scopePtr) {
    for (int i = 0; i < this->varList.size(); i++) {
        if (id == this->varList[i]->getID()) {
            if (!this->varList[i]->isPublic) // is private
            {
                return scopePtr == this; // if in the same scope, can find private var
            }
            return true;
        }
    }
    return false;
}

bool Runtime::Scope::addVar(Runtime::Variable* var) { // not allow re-add
    if (this->hasVar(var->getID(), this)) {
        return false;
    }
    this->varList.push_back(var);
    return true;
}

bool Runtime::Scope::addScope(Runtime::Scope* sc) {
    for (int i = 0; i < sc->scopeList.size();i++) {
        if (sc->name == sc->scopeList[i]->name) //compare the name of input with every scope name
            return false;
    }
    this->scopeList.push_back(sc);
    return true;
}

Runtime::Variable::Variable(string id, bool isPublic, Runtime::Scope* parentScope) {
    this->id = id;
    this->isPublic = isPublic;
    this->parentScope = parentScope;
}
