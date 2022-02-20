#include "format.h"
using namespace std;
using namespace preEval;

FormatAnalyzer::FormatAnalyzer(vector<Token> tokenList){
    this->tokenList = tokenList;
}


Analysis_Status FormatAnalyzer::scan() {
    int tokenIndex = 0;
    vector<Token> saveline;
    vector<Scope*> currentScope;
    currentScope.push_back(&globalScope);
    while(tokenIndex < tokenList.size()){
        //tokenList[tokenIndex].Print();
        if (tokenList[tokenIndex].token_type == SEMICOLON) {
            Line* newLine = new Line(saveline);
            currentScope.back()->push(newLine);
            saveline.clear();
        }
        else if (tokenList[tokenIndex].token_type == LBRACE) {
            if (saveline.size() == 1 && saveline.front().token_type == ID) {  // is class_name {}
                Scope* newScope = new Scope();  
                newScope->addAttr(CLASS_NAME_ATTR, saveline[0].lexeme);    
                saveline.clear();
                currentScope.back()->push(newScope); //FILO
                currentScope.push_back(newScope);
            }
            else {
                return SYNTAX_ERROR;
            }
        }
        else if (tokenList[tokenIndex].token_type == RBRACE) {
            if(!saveline.empty()) return SYNTAX_ERROR;
            if (currentScope.size() == 1) return SYNTAX_ERROR;
            currentScope.pop_back();
        }
        else {
            saveline.push_back(tokenList[tokenIndex]);
        }
        tokenIndex++;
    }
    
    if (!saveline.empty() || currentScope.size() != 1) // if no ; at the end of a line; or there are unclosed scope in currentScope
    {
        return SYNTAX_ERROR; 
    }
    return FINISHED;
}

void Scope::push(prototype* p){
    this->items.push_back(p);
}

void Scope::addAttr(string key, string value){
    this->attr.insert({key, value});
}

Line::Line(vector<Token> line) {
    this->tokens = vector<Token>(line);
    this->isScope = false;
}

void Scope::destory(){
    cout<< "start Destory" << endl;
    for(int i = 0; i < items.size(); i++){
       if (items[i]->isScope) {
           ((Scope*)items[i])->destory();
           delete (Scope*)items[i];
           continue;
       }
       delete (Line*)items[i];
    }
}


static string reserved[] = { "END_OF_FILE",
    "IF", "WHILE", "DO", "THEN", "PRINT","PUBLIC","PRIVATE",
    "PLUS", "MINUS", "DIV", "MULT",
    "EQUAL", "COLON", "COMMA", "SEMICOLON",
    "LBRAC", "RBRAC", "LPAREN", "RPAREN",
    "NOTEQUAL", "GREATER", "LESS", "LTEQ", "GTEQ",
    "DOT", "NUM", "ID", "ERROR","REALNUM","BASE08NUM","BASE16NUM","COMMENT","LBRACE","RBRACE" // TODO: Add labels for new token types here (as string)
};


void Scope::Print(int indent) {
    string iStr = getIndent(indent);
    if (attr.count(CLASS_NAME_ATTR) > 0) { // exist 'class name'
        cout << iStr << "class scope: " << attr[CLASS_NAME_ATTR] << "    " << this->items.size() <<endl;
    }
    else {
        cout << iStr << "scope" << endl;
    }

    for(int i = 0; i < this->items.size(); i++){
        this->items[i]->Print(indent + 1);
    }
}

void Line::Print(int indent) {
    string iStr = getIndent(indent);
    cout << iStr;
    for(int i = 0; i < tokens.size(); i++){
        Token t = tokens[i];
        cout << "{" << t.lexeme << " , "
         << reserved[(int) t.token_type] << " , "
         << t.line_no << "}";
    }
    cout << endl;
}
string Scope::getAttr(string key) {
    return attr[key];
}

void prototype::Print(int i) {}