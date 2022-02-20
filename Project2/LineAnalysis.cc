#include "LineAnalysis.h"

using namespace preEval;

Equation::Equation(vector<Token>left, vector<Token>right, Line* line) {
    this->leftIDs = left;
    this->rightExpression = right;
    this->tokens = line->tokens;
    this->isScope = false;
    this->isEquation = true;
}

Declaration::Declaration(Line* line, vector<Token>dec) {
    this->declTokens = dec;
    this->tokens = line->tokens;
    this->isScope = false;
    this->isEquation = false;
}
LineAnalysis::LineAnalysis(FormatAnalyzer& fa) {
    this->globalScope = fa.globalScope;
}


Analysis_Status LineAnalysis::scan() {
    return scan(&this->globalScope);
}

Analysis_Status LineAnalysis::scan(Scope* s) {
    Scope& scope = *s;
    for (int i = 0; i < scope.size(); i++) { // scope
        if (scope[i]->isScope) {
            auto status = scan(((Scope*)scope[i]));
            if (status == SYNTAX_ERROR)
                return SYNTAX_ERROR;
        }
        else { // line
            Line* linePtr = (Line*)scope[i];
            if (isExpression(linePtr)) { //expression 
                Equation* newEquation;
                auto status = leftORright(linePtr, &newEquation);
                if (status == SYNTAX_ERROR)
                    return SYNTAX_ERROR;
                scope.set(i, newEquation);
                delete linePtr;
            }
            else { //declearation
                Declaration* newDeclaration;
                auto status = declLine(linePtr, &newDeclaration);
                if (status == SYNTAX_ERROR)
                    return SYNTAX_ERROR;
                scope.set(i, newDeclaration);
                delete linePtr;
            }
        }
    }
    return FINISHED;
}



ProcessLine::ProcessLine(Line& line) {
    this->tokens = line.tokens;
}

bool LineAnalysis::isExpression(Line* line) {
    for (int i = 0; i < line->tokens.size(); i++) {
        if (line->tokens[i].token_type == EQUAL)
            return true;
    }
    return false;
}

vector<Token> leftValue_getIDs(vector<Token> leftTokens) {
    bool mustID = true;
    vector<Token> IDs;
    for (int i = 0; i < leftTokens.size(); i++) {
        if (mustID && leftTokens[i].token_type != ID) {
            IDs.clear();
            return IDs;
        }
        else if (!mustID && leftTokens[i].token_type != COMMA) {
            IDs.clear();
            return IDs;
        }
        if (leftTokens[i].token_type == ID)
            IDs.push_back(leftTokens[i]);

        mustID = !mustID;
    }
    return IDs;
}

vector<Token> rightValue_getExpression(vector<Token> rightTokens) {
    bool mustID = true;
    vector<Token> ExpressionTokens;
    if (rightTokens.size() != 1 || rightTokens[0].token_type != ID) {
        return ExpressionTokens;
    }
    ExpressionTokens.push_back(rightTokens[0]);
    return ExpressionTokens;
}

Analysis_Status LineAnalysis::leftORright(Line* line, Equation** newEquation) {
    int i = 0;
    vector<Token> leftTokens;
    vector<Token> rightTokens;
    while (line->tokens[i].token_type != EQUAL) {
        leftTokens.push_back(line->tokens[i]);
        i++;
    }
    i++;
    for (; i < line->tokens.size(); i++) {
        if (line->tokens[i].token_type == EQUAL) {
            return SYNTAX_ERROR;
        }
        rightTokens.push_back(line->tokens[i]);
    }
    if (rightTokens.empty() || leftTokens.empty())
        return SYNTAX_ERROR;


    vector<Token> leftIDs = leftValue_getIDs(leftTokens);
    vector<Token> rightExpression = rightValue_getExpression(rightTokens);

    if (leftIDs.empty() || rightExpression.empty()) {
        return SYNTAX_ERROR;
    }

    *newEquation = new Equation(leftIDs, rightExpression, line);
    return FINISHED; // edit
}


Analysis_Status LineAnalysis::declLine(Line* line, Declaration** newDeclaration) {
    vector<Token> declTokens; // public ID ID
    int i = 0;
    if (line->tokens[i].token_type == PUBLIC || line->tokens[i].token_type == PRIVATE) {
        i++;//to the next token, expect ":"
        if (line->tokens[i].token_type == COLON) {
            declTokens.push_back(line->tokens[0]);
            i++; //jump to next token, expect jump after ":"
        }
        else return SYNTAX_ERROR;
    }

    bool mustID = true;
    for (; i < line->tokens.size(); i++) {
        if (mustID && line->tokens[i].token_type != ID) {
            return SYNTAX_ERROR;
        }
        else if (!mustID && line->tokens[i].token_type != COMMA) {
            return SYNTAX_ERROR;
        }
        if (line->tokens[i].token_type == ID) {
            declTokens.push_back(line->tokens[i]);
        }
        mustID = !mustID;
    }
    *newDeclaration = new Declaration(line, declTokens);
    return FINISHED;
}


void Equation::Print(int indent) {
    string iStr = getIndent(indent);
    cout << iStr;
    for (int i = 0; i < leftIDs.size(); i++) {
        Token t = leftIDs[i];
        cout << t.lexeme << "   ";
    }
    cout << " = ";
    for (int i = 0; i < rightExpression.size(); i++) {
        Token t = rightExpression[i];
        if (t.token_type == ID) cout << t.lexeme;
        else cout << "notID";
        cout << "  ";
    }
    cout << endl;
}

void Declaration::Print(int indent) {
    string iStr = getIndent(indent);
    cout << iStr;
    for (int i = 0; i < declTokens.size(); i++) {
        Token t = declTokens[i];
        if (t.token_type == ID) cout << t.lexeme;
        else cout << "PUBorPRI";
        cout << "  ";
    }
    cout << endl;
}