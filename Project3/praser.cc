/*
 * Copyright (C) Rida Bazzi, 2016
 *
 * Do not share this file with anyone
 */

#include <iostream>
#include <istream>
#include <vector>
#include <string>
#include <cctype>
#include <stdlib.h>
#include <string.h>

#include "inputbuf.h"
#include "lexer.h"


#ifdef VS
#pragma warning(disable : 4996)
#endif // VS

using namespace std;

string reserved[] = { "END_OF_FILE", "INT", "REAL", "BOO", "TR", "FA", "IF", "WHILE", "SWITCH", "CASE", "PUBLIC", "PRIVATE", "NUM", "REALNUM", "NOT", "PLUS", "MINUS", "MULT", "DIV", "GTEQ",
"GREATER", "LTEQ", "NOTEQUAL", "LESS", "LPAREN", "RPAREN", "EQUAL", "COLON", "COMMA", "SEMICOLON", "LBRACE", "RBRACE", "ID", "ERROR","LBRAC","RBRAC","DOT" // TODO: Add labels for new token types here (as string)
};

#define KEYWORDS_COUNT 11
string keyword[] = { "int", "real", "bool", "true", "false", "if", "while", "switch", "case", "public", "private" };

LexicalAnalyzer lexer;
Token token;
TokenType tempTokenType;


struct sTableEntry {
    string name;
    int binNo;
    int type; // -1 = unknown, INT, REAL, BOO
    int ID;
};

int BinNoCounter = 0;
int getNewBinNO() {
    return BinNoCounter++;
}

int varID = 0;
int getNewVarID() {
    return varID++;
}

struct varID_msg {
    int varID;
    string msg;
};

sTableEntry* searchList(string ID);

vector<varID_msg> outputBuffer;
void output_addLine(string outputLine, int varID) {
    struct varID_msg bm;
    bm.varID = varID;
    bm.msg = outputLine;

    bool inserted = false;
    for (int i = 0; i < outputBuffer.size(); i++) {
        if (outputBuffer[i].varID > varID) {
            outputBuffer.insert(outputBuffer.begin() + i, bm);
            inserted = true;
            break;
        }
    }
    if (!inserted) {
        outputBuffer.push_back(bm);
    }
}

void output_addVar_type(string name, int type, int varID) {
    string outputLine = name;
    outputLine += ": ";
    switch (type)
    {
    case INT:
        outputLine += "int #";
        break;
    case REAL:
        outputLine += "real #";
        break;
    case BOO:
        outputLine += "bool #";
        break;
    default:
        outputLine += "? #";
        break;
    }

    output_addLine(outputLine, varID);
}

void output_add_unknownType_vars(vector<string> varNames, int minVarID) {
    if (varNames.size() <= 0) {
        return;
    }
    string outputLine = "";
    for (int i = 0; i < varNames.size(); i++) {
        string name = varNames[i];
        if (i > 0) {
            outputLine += ", ";
        }
        outputLine += name;
    }
    outputLine += ": ? #";

    output_addLine(outputLine, minVarID);
}

void output_error(string errorMessage) {
    outputBuffer.clear();
    cout << errorMessage << endl;
    exit(0);
}

void output_syntaxError() {
    outputBuffer.clear();
    cout << "Syntax Error" << endl;
    exit(0);
}

string createErrorMessage(int line_no, int c_errorType) {
    string errorMessage("TYPE MISMATCH ");
    errorMessage += to_string(line_no);
    errorMessage += " C";
    errorMessage += to_string(c_errorType);
    return errorMessage;
}

int findMinVarID(vector<sTableEntry*>& var_list) {
    int minVarID = -1;
    sTableEntry* tempVarInfo = var_list[0];
    minVarID = tempVarInfo->ID;
    for (int i = 1; i < var_list.size(); i++) {
        tempVarInfo = var_list[i];
        if (minVarID > tempVarInfo->ID) {
            minVarID = tempVarInfo->ID;
        }
    }
    return minVarID;
}

vector<sTableEntry*> symbolTable;

vector<sTableEntry*> get_remove_unknownVarFromSymbolTable(int binNO) {
    vector<sTableEntry*> ret;
    for (int i = 0; i < symbolTable.size(); i++) {
        if (symbolTable[i]->binNo == binNO) {
            sTableEntry* temp = symbolTable[i];
            symbolTable.erase(symbolTable.begin() + i);
            ret.push_back(temp);
            i--;
        }
    }
    return ret;
}

void output() {
    while (symbolTable.size() > 0) {
        sTableEntry* temp = symbolTable.back();
        if (symbolTable.back()->type > 0) {
            symbolTable.pop_back();
            output_addVar_type(temp->name, temp->type, temp->ID);
            delete temp;
        }
        else {
            vector<sTableEntry*> unknownVar = get_remove_unknownVarFromSymbolTable(temp->binNo);
            vector<string> varNames;
            for (int i = 0; i < unknownVar.size(); i++)
            {
                sTableEntry* var = unknownVar[i];
                varNames.push_back(var->name);
            }
            output_add_unknownType_vars(varNames, findMinVarID(unknownVar));
            for (int i = 0; i < unknownVar.size(); i++)
            {
                sTableEntry* var = unknownVar[i];
                delete var;
            }
        }
    }

    for (int i = 0; i < outputBuffer.size(); i++) {
        cout << outputBuffer[i].msg << endl;
    }
}

sTableEntry* addList(string lexeme) {
    if (searchList(lexeme) != NULL) {
        return searchList(lexeme);
    }

    sTableEntry* newItem = new sTableEntry;

    newItem->name = string(lexeme);
    newItem->binNo = getNewBinNO();
    newItem->type = -1;
    newItem->ID = getNewVarID();

    symbolTable.push_back(newItem);

    return newItem;
}

vector<string> getsTableEntryByBinNo(int binNO) {
    vector<string> ret;
    for (int i = 0; i < symbolTable.size(); i++) {
        if (symbolTable[i]->binNo == binNO) {
            ret.push_back(symbolTable[i]->name);
        }
    }
    return ret;
}

sTableEntry* searchList(string ID) {
    bool found = false;
    sTableEntry* ret = NULL;

    for (int i = 0; i < symbolTable.size(); i++) {
        if (symbolTable[i]->name == ID) {
            ret = symbolTable[i];
        }
    }

    return ret;
}

void Token::Print()
{
    cout << "{" << this->lexeme << " , "
        << reserved[(int)this->token_type] << " , "
        << this->line_no << "}\n";
}

LexicalAnalyzer::LexicalAnalyzer()
{
#ifdef VS
    input.readFile();
#endif // VS
    this->line_no = 1;
    tmp.lexeme = "";
    tmp.line_no = 1;
    tmp.token_type = ERROR;
}

bool LexicalAnalyzer::SkipSpace()
{
    char c;
    bool space_encountered = false;

    input.GetChar(c);
    line_no += (c == '\n');

    while (!input.EndOfInput() && isspace(c)) {
        space_encountered = true;
        input.GetChar(c);
        line_no += (c == '\n');
    }

    if (!input.EndOfInput()) {
        input.UngetChar(c);
    }
    return space_encountered;
}

bool LexicalAnalyzer::SkipComments()
{

    bool comments = false;
    char c;
    if (input.EndOfInput()) {
        //input.UngetChar(c);
        return comments;
    }

    input.GetChar(c);


    if (c == '/') {
        input.GetChar(c);
        if (c == '/') {
            comments = true;
            while (c != '\n') {
                comments = true;
                input.GetChar(c);


            }
            line_no++;

            SkipComments();
        }
        else {
            comments = false;
            input.UngetChar(c);
            input.UngetChar('/');
        }


    }
    else {
        input.UngetChar(c);

        return comments;
    }





}

bool LexicalAnalyzer::IsKeyword(string s)
{
    for (int i = 0; i < KEYWORDS_COUNT; i++) {
        if (s == keyword[i]) {
            return true;
        }
    }
    return false;
}

TokenType LexicalAnalyzer::FindKeywordIndex(string s)
{
    for (int i = 0; i < KEYWORDS_COUNT; i++) {
        if (s == keyword[i]) {
            return (TokenType)(i + 1);
        }
    }
    return ERROR;
}

Token LexicalAnalyzer::ScanNumber()
{
    char c;
    bool realNUM = false;
    input.GetChar(c);
    if (isdigit(c)) {
        if (c == '0') {
            tmp.lexeme = "0";
            input.GetChar(c);
            if (c == '.') {

                //cout << "\n I am here too " << c << " \n";
                input.GetChar(c);

                if (!isdigit(c)) {
                    input.UngetChar(c);
                }
                else {
                    while (!input.EndOfInput() && isdigit(c)) {
                        tmp.lexeme += c;
                        input.GetChar(c);
                        realNUM = true;

                    }
                    input.UngetChar(c);
                }
            }
            else {
                input.UngetChar(c);
            }
        }
        else {
            tmp.lexeme = "";
            while (!input.EndOfInput() && isdigit(c)) {
                tmp.lexeme += c;
                input.GetChar(c);
            }
            if (c == '.') {

                //cout << "\n I am here too " << c << " \n";
                input.GetChar(c);

                if (!isdigit(c)) {
                    input.UngetChar(c);
                }
                else {
                    while (!input.EndOfInput() && isdigit(c)) {
                        tmp.lexeme += c;
                        input.GetChar(c);
                        realNUM = true;
                    }
                }
            }

            if (!input.EndOfInput()) {
                input.UngetChar(c);
            }
        }
        // TODO: You can check for REALNUM, BASE08NUM and BASE16NUM here!
        if (realNUM) {
            tmp.token_type = REALNUM;
        }
        else {
            tmp.token_type = NUM;
        }
        tmp.line_no = line_no;
        return tmp;
    }
    else {
        if (!input.EndOfInput()) {
            input.UngetChar(c);
        }
        tmp.lexeme = "";
        tmp.token_type = ERROR;
        tmp.line_no = line_no;
        return tmp;
    }
}

Token LexicalAnalyzer::ScanIdOrKeyword()
{
    char c;
    input.GetChar(c);

    if (isalpha(c)) {
        tmp.lexeme = "";
        while (!input.EndOfInput() && isalnum(c)) {
            tmp.lexeme += c;
            input.GetChar(c);
        }

        if (!input.EndOfInput()) {
            input.UngetChar(c);
        }
        tmp.line_no = line_no;

        if (IsKeyword(tmp.lexeme))
            tmp.token_type = FindKeywordIndex(tmp.lexeme);
        else
            tmp.token_type = ID;
    }
    else {
        if (!input.EndOfInput()) {
            input.UngetChar(c);
        }
        tmp.lexeme = "";
        tmp.token_type = ERROR;
    }
    return tmp;
}

// you should unget tokens in the reverse order in which they
// are obtained. If you execute
//
//    t1 = lexer.GetToken();
//    t2 = lexer.GetToken();
//    t3 = lexer.GetToken();
//
// in this order, you should execute
//
//    lexer.UngetToken(t3);
//    lexer.UngetToken(t2);
//    lexer.UngetToken(t1);
//
// if you want to unget all three tokens. Note that it does not
// make sense to unget t1 without first ungetting t2 and t3
//
TokenType LexicalAnalyzer::UngetToken(Token tok)
{
    tokens.push_back(tok);;
    return tok.token_type;
}

Token LexicalAnalyzer::GetToken()
{
    char c;

    // if there are tokens that were previously
    // stored due to UngetToken(), pop a token and
    // return it without reading from input
    if (!tokens.empty()) {
        tmp = tokens.back();
        tokens.pop_back();
        return tmp;
    }

    SkipSpace();
    SkipComments();
    SkipSpace();
    tmp.lexeme = "";
    tmp.line_no = line_no;
    input.GetChar(c);
    //cout << "\n Char obtained " << c << "\n";
    switch (c) {
    case '!':
        tmp.token_type = NOT;
        return tmp;
    case '+':
        tmp.token_type = PLUS;
        return tmp;
    case '-':
        tmp.token_type = MINUS;
        return tmp;
    case '*':
        tmp.token_type = MULT;
        return tmp;
    case '/':
        tmp.token_type = DIV;
        return tmp;
    case '>':
        input.GetChar(c);
        if (c == '=') {
            tmp.token_type = GTEQ;
        }
        else {
            input.UngetChar(c);
            tmp.token_type = GREATER;
        }
        return tmp;
    case '<':
        input.GetChar(c);
        if (c == '=') {
            tmp.token_type = LTEQ;
        }
        else if (c == '>') {
            tmp.token_type = NOTEQUAL;
        }
        else {
            input.UngetChar(c);
            tmp.token_type = LESS;
        }
        return tmp;
    case '(':
        //cout << "\n I am here" << c << " \n";
        tmp.token_type = LPAREN;
        return tmp;
    case ')':
        tmp.token_type = RPAREN;
        return tmp;
    case '=':
        tmp.token_type = EQUAL;
        return tmp;
    case ':':
        tmp.token_type = COLON;
        return tmp;
    case ',':
        tmp.token_type = COMMA;
        return tmp;
    case ';':
        tmp.token_type = SEMICOLON;
        return tmp;
    case '{':
        tmp.token_type = LBRACE;
        return tmp;
    case '}':
        tmp.token_type = RBRACE;
        return tmp;
    default:
        if (isdigit(c)) {
            input.UngetChar(c);
            return ScanNumber();
        }
        else if (isalpha(c)) {
            input.UngetChar(c);
            //cout << "\n ID scan " << c << " \n"; 
            return ScanIdOrKeyword();
        }
        else if (input.EndOfInput())
            tmp.token_type = END_OF_FILE;
        else
            tmp.token_type = ERROR;

        return tmp;
    }
}


// parse var_list
int parse_varlist(vector<string>& varNames_out) { // the head of new added sTable will be out
    token = lexer.GetToken();
    int tempI = 0;
    // Check First set of ID
    if (token.token_type == ID) {
        addList(token.lexeme);
        varNames_out.push_back(token.lexeme); // add var name into the array
        token = lexer.GetToken();
        if (token.token_type == COMMA) {
            tempI = parse_varlist(varNames_out);
        }
        else if (token.token_type == COLON) {
            tempTokenType = lexer.UngetToken(token);
        }
        else {
            output_syntaxError();
        }
    }
    else {
        output_syntaxError();
    }

    return(0);

}

int parse_body(void);

int parse_unaryOperator(void) {
    token = lexer.GetToken();

    if (token.token_type == NOT) {
    }
    else {
        output_syntaxError();
    }

    return(0);

}

int parse_binaryOperator(void) {
    token = lexer.GetToken();
    //keep track of the number of bin operations in binNo
    if (token.token_type == PLUS) {
        return 1;
    }
    else if (token.token_type == MINUS) {
        return 1;
    }
    else if (token.token_type == MULT) {
        return 1;
    }
    else if (token.token_type == DIV) {
        return 1;
    }
    else if (token.token_type == GREATER) {
        return 2;
    }
    else if (token.token_type == LESS) {
        return 2;
    }
    else if (token.token_type == GTEQ) {
        return 2;
    }
    else if (token.token_type == LTEQ) {
        return 2;
    }
    else if (token.token_type == EQUAL) {
        return 2;
    }
    else if (token.token_type == NOTEQUAL) {
        return 2;
    }
    else {
        output_syntaxError();
    }
    return(-1);

}

int parse_primary() { // 0 = unknown, -1 = error, otherwise return type
    token = lexer.GetToken();

    if (token.token_type == ID) {
        // search list for the token. If token available then return the type of the token. if not then add the token to the list
        // make its scope = "h" and make its type = -1;
        int retType = 0;
        string id = token.lexeme;
        sTableEntry* searchResult = searchList(id);
        if (searchResult == NULL) { // cannot find the id
            addList(id);
        }
        else {
            retType = searchResult->type;
            if (retType < 0) {
                retType = 0;
            }
        }
        return retType;
    }
    else if (token.token_type == NUM) {
        return INT;

    }
    else if (token.token_type == REALNUM) {
        return REAL;

    }
    else if (token.token_type == TR) {
        return BOO;

    }
    else if (token.token_type == FA) {
        return BOO;
    }
    else {
        output_syntaxError();
    }
    return(-1);

}

void determineType(vector<string>& var_name_list, int type);
void assignNewBinNO(int oldBinNO, int newBinNO);
void assignTypeToCertainBinNO(int binNO, int type);

int parse_expression(vector<string>& involved_var_name_list) { // return type, -1 = error, 0 = unknown
    int tempI;

    token = lexer.GetToken();

    // single token
    if (token.token_type == ID || token.token_type == NUM || token.token_type == REALNUM || token.token_type == TR || token.token_type == FA) {

        tempTokenType = lexer.UngetToken(token);
        tempI = parse_primary();
        if (token.token_type == ID) {
            string varName = token.lexeme;
            involved_var_name_list.push_back(varName);
        }
        return tempI; // return the type, -1 means error, 0 means unknown
    }


    else if (token.token_type == PLUS || token.token_type == MINUS || token.token_type == MULT || token.token_type == DIV ||
        token.token_type == GREATER || token.token_type == LESS || token.token_type == GTEQ || token.token_type == LTEQ || token.token_type == EQUAL || token.token_type == NOTEQUAL) {
        // can only solve + a b, or + a * b c,  cannot solve + (* a b) c
        //cout << "\n Rule Parsed: expression -> binary_operator expression expression \n";
        tempTokenType = lexer.UngetToken(token);
        tempI = parse_binaryOperator();

        if (tempI == 1) {
            int leftType = parse_expression(involved_var_name_list);
            int rightType = parse_expression(involved_var_name_list);

            if (leftType == -1 || rightType == -1) {
                output_syntaxError();
                return -1; // error
            }

            if (leftType == 0 && rightType == 0) { // means both left and right side are unknown type
                return 0;
            }
            else if (leftType == 0 || rightType == 0) {
                return max(leftType, rightType);
            }
            else if (leftType == rightType) {
                return leftType;
            }
            else { // it means they do not have same type
                // C2 error
                output_error(createErrorMessage(token.line_no, 2));
                return -1;
            }
        }


        // if gt gteq ---------------
        if (tempI == 2) {
            vector<string> temp_var_name_list;
            int leftType = parse_expression(temp_var_name_list);
            int rightType = parse_expression(temp_var_name_list);

            if (leftType == -1 || rightType == -1) {
                output_syntaxError();
                return -1; // error
            }

            if (leftType == 0 && rightType == 0) { // means both left and right side are unknown type
                determineType(temp_var_name_list, 0);
            }
            else if (leftType == 0 || rightType == 0) {
                determineType(temp_var_name_list, max(leftType, rightType));
            }
            else if (leftType != rightType) { // it means they do not have same type
                // C2 error
                output_error(createErrorMessage(token.line_no, 2));
                return -1;
            }

            return BOO;
        }


    }
    else if (token.token_type == NOT) {
        tempTokenType = lexer.UngetToken(token);
        int tempI1 = parse_unaryOperator();
        vector<string> var_afterNOT;
        int tempI2 = parse_expression(var_afterNOT);
        if (tempI2 == 0) // if unknown
        {
            determineType(var_afterNOT, BOO);
        }
        else if (tempI2 != BOO) {
            // ERROR C3
            output_error(createErrorMessage(token.line_no, 3));
            return -1;
        }

        return BOO;
    }
    else {
        output_syntaxError();
    }
    return(-1);

}

void determineType(vector<string>& var_name_list, int type) {
    // set all var to unknown type and bind them using 'binNO', or make sure they have same type
    if (type != INT && type != REAL && type != BOO) {
        type = -1; // -1 means unknown here
    }
    if (var_name_list.size() < 1) {
        return;
    }

    int newBinNO = getNewBinNO();
    for (int i = 0; i < var_name_list.size(); i++) {
        sTableEntry* varInfo = searchList(var_name_list[i]);
        assignNewBinNO(varInfo->binNo, newBinNO);
    }
    assignTypeToCertainBinNO(newBinNO, type);
}

void assignNewBinNO(int oldBinNO, int newBinNO) {
    if (oldBinNO == -1) {
        return;
    }

    for (int i = 0; i < symbolTable.size(); i++) {
        if (symbolTable[i]->binNo == oldBinNO) {
            symbolTable[i]->binNo = newBinNO;
        }
    }
}

void assignTypeToCertainBinNO(int binNO, int type) {
    if (type == -1) {
        return;
    }

    for (int i = 0; i < symbolTable.size(); i++) {
        if (symbolTable[i]->binNo == binNO) {
            symbolTable[i]->type = type;
        }
    }
}

int parse_assstmt(void) {
    int tempI;
    token = lexer.GetToken();
    if (token.token_type == ID) {
        // search for the token in the searchList --> the token is available, leftType = type of the available token
        // it is not available in the searchList, add the token to the list, make its type = -1; make its scope = "h".
        Token leftVarToken = token;
        lexer.UngetToken(token);
        string leftVarName = token.lexeme;
        int leftVarType = parse_primary(); // if type = 0, means unknown

        token = lexer.GetToken();
        if (token.token_type == EQUAL) {
            token = lexer.GetToken();
            if (token.token_type == ID || token.token_type == NUM || token.token_type == REALNUM || token.token_type == TR || token.token_type == FA ||
                token.token_type == PLUS || token.token_type == MINUS || token.token_type == MULT || token.token_type == DIV ||
                token.token_type == LESS || token.token_type == GREATER || token.token_type == GTEQ || token.token_type == LTEQ || token.token_type == EQUAL || token.token_type == NOTEQUAL ||
                token.token_type == NOT)
            {
                tempTokenType = lexer.UngetToken(token);

                vector<string> var_name;
                var_name.push_back(leftVarName);
                int type = parse_expression(var_name);
                if (type == -1) {
                    return 0;
                }

                if (leftVarType > 0) { // it means left var is NOT unknown type
                    if (type > 0) { // it means both side have type
                        if (type != leftVarType) {
                            // C1 ERROR
                            output_error(createErrorMessage(leftVarToken.line_no, 1));
                            return 0;
                        }
                    }
                    else { // it means left side has type but right side doesn't
                        type = leftVarType;
                    }
                }

                determineType(var_name, type);

                token = lexer.GetToken();

                if (token.token_type == SEMICOLON) {
                }
                else {
                    output_syntaxError();
                }
            }
            else {
                output_syntaxError();
            }
        }
        else {
            output_syntaxError();
        }

    }
    else {
        output_syntaxError();
    }
    return(0);
}

int parse_case(void) {

    int tempI = 0;
    token = lexer.GetToken();
    if (token.token_type == CASE) {
        token = lexer.GetToken();
        if (token.token_type == NUM) {
            token = lexer.GetToken();
            if (token.token_type == COLON) {
                tempI = parse_body();
            }
            else {
                output_syntaxError();
            }

        }
        else {
            output_syntaxError();
        }

    }
    else {
        output_syntaxError();
    }
    return tempI;
}

int parse_caselist(void) {

    int tempI;
    token = lexer.GetToken();
    if (token.token_type == CASE) {
        tempTokenType = lexer.UngetToken(token);
        tempI = parse_case();
        token = lexer.GetToken();
        if (token.token_type == CASE) {
            tempTokenType = lexer.UngetToken(token);
            tempI = parse_caselist();
        }
        else if (token.token_type == RBRACE) {
            tempTokenType = lexer.UngetToken(token);
        }
    }
    return(0);
}


int parse_switchstmt(void) {
    int tempI;

    token = lexer.GetToken();
    if (token.token_type == SWITCH) {
        token = lexer.GetToken();
        if (token.token_type == LPAREN) {
            vector<string> var_names;
            tempI = parse_expression(var_names);

            // if tempI != INT then throw type error
            // else if tempI = -1 ==> parse_expresssion retunred an ID, then go and change using searchList the type of ID to 1.


            if (tempI == 0) {
                determineType(var_names, INT);
            }
            else if (tempI != INT) {
                // C5 ERROR
                output_error(createErrorMessage(token.line_no, 5));
                return 0;
            }
            else {
                determineType(var_names, INT);
            }

            token = lexer.GetToken();
            if (token.token_type == RPAREN) {
                token = lexer.GetToken();
                if (token.token_type == LBRACE) {
                    tempI = parse_caselist();
                    token = lexer.GetToken();
                    if (token.token_type == RBRACE) {
                    }
                    else {
                        output_syntaxError();
                    }
                }
                else {
                    output_syntaxError();
                }

            }
            else {
                output_syntaxError();
            }
        }
        else {
            output_syntaxError();
        }
    }
    else {
        output_syntaxError();
    }
    return(0);
}


int parse_whilestmt(void) {
    int tempI;

    token = lexer.GetToken();
    if (token.token_type == WHILE) {
        token = lexer.GetToken();
        if (token.token_type == LPAREN) {

            vector<string> var_names;
            tempI = parse_expression(var_names);

            // if tempI != bool then throw type error
            // else if tempI = 0 ==> parse_expresssion retunred an ID, then go and change using searchList the type of ID to 2.


            if (tempI == 0) {
                determineType(var_names, BOO);
            }
            else if (tempI != BOO) {
                // C4 ERROR
                output_error(createErrorMessage(token.line_no, 4));
                return 0;
            }
            else {
                determineType(var_names, BOO);
            }

            token = lexer.GetToken();
            if (token.token_type == RPAREN) {
                tempI = parse_body();
            }
            else {
                output_syntaxError();
            }
        }
        else {
            output_syntaxError();
        }
    }
    else {
        output_syntaxError();
    }
    return(0);
}

int parse_ifstmt(void) {
    int tempI;

    token = lexer.GetToken();
    if (token.token_type == IF) {
        token = lexer.GetToken();
        if (token.token_type == LPAREN) {
            vector<string> var_names;
            tempI = parse_expression(var_names);

            // if tempI != bool then throw type error
            // else if tempI = 0 ==> parse_expresssion retunred an ID, then go and change using searchList the type of ID to 2.


            if (tempI == 0) {
                determineType(var_names, BOO);
            }
            else if (tempI != BOO) {
                // C4 ERROR
                output_error(createErrorMessage(token.line_no, 4));
                return 0;
            }
            else {
                determineType(var_names, BOO);
            }

            token = lexer.GetToken();
            if (token.token_type == RPAREN) {
                tempI = parse_body();
            }
            else {
                output_syntaxError();
            }
        }
        else {
            output_syntaxError();
        }
    }
    else {
        output_syntaxError();
    }
    return(0);
}

int parse_stmt(void) {
    int tempI;
    token = lexer.GetToken();
    if (token.token_type == ID) {
        tempTokenType = lexer.UngetToken(token);
        tempI = parse_assstmt();

    }
    else if (token.token_type == IF) {
        tempTokenType = lexer.UngetToken(token);
        tempI = parse_ifstmt();
    }
    else if (token.token_type == WHILE) {
        tempTokenType = lexer.UngetToken(token);
        tempI = parse_whilestmt();
    }
    else if (token.token_type == SWITCH) {
        tempTokenType = lexer.UngetToken(token);
        tempI = parse_switchstmt();
    }
    else {
        output_syntaxError();
    }
    return(0);
}

int parse_stmtlist(void) {

    token = lexer.GetToken();
    int tempI;
    if (token.token_type == ID || token.token_type == IF || token.token_type == WHILE || token.token_type == SWITCH) {
        tempTokenType = lexer.UngetToken(token);
        tempI = parse_stmt();
        token = lexer.GetToken();
        if (token.token_type == ID || token.token_type == IF || token.token_type == WHILE || token.token_type == SWITCH) {
            tempTokenType = lexer.UngetToken(token);
            tempI = parse_stmtlist();

        }
        else if (token.token_type == RBRACE) {
            tempTokenType = lexer.UngetToken(token);
        }
    }
    else {
        output_syntaxError();

    }
    return(0);
}



int parse_body(void) {

    token = lexer.GetToken();

    if (token.token_type == LBRACE) {
        parse_stmtlist();
        token = lexer.GetToken();
        if (token.token_type == RBRACE) {
            return(0);
        }
        else {
            output_syntaxError();
            return(0);
        }
    }
    else if (token.token_type == END_OF_FILE) {
        tempTokenType = lexer.UngetToken(token);
        return(0);
    }
    else {
        output_syntaxError();
        return(0);
    }

}

int parse_typename(void) {
    token = lexer.GetToken();
    if (token.token_type == INT || token.token_type == REAL || token.token_type == BOO) {
        //cout << "\n Rule parse: type_name -> " << token.token_type << "\n";
        if (token.token_type == INT) {
            return INT;
        }
        if (token.token_type == REAL) {
            return REAL;
        }
        if (token.token_type == BOO) {
            return BOO;
        }

    }
    else {
        output_syntaxError();
        return -1;
    }
}

int parse_vardecl(void) {
    int type;
    token = lexer.GetToken();
    if (token.token_type == ID) {
        tempTokenType = lexer.UngetToken(token);
        vector<string> varNames;
        type = parse_varlist(varNames);
        token = lexer.GetToken();
        if (token.token_type == COLON) {
            type = parse_typename(); // Reture: INT, REAL, BOO, -1

            if (type > 0) { // set new decled var's type
                for (int i = 0; i < varNames.size(); i++) {
                    if (searchList(varNames[i])->type > 0) {
                        output_error("Undefined type error: declare a symbol using a type different from the exist type");
                    }
                    searchList(varNames[i])->type = type;
                }
            }
            else {
                output_syntaxError();
            }

            //use the searchList to update the types of variables that are already in the symbolTable

            token = lexer.GetToken();
            if (token.token_type == SEMICOLON) {
            }
            else {
                output_syntaxError();
            }
        }
        else {
            output_syntaxError();
        }
    }
    else {
        output_syntaxError();

    }
    return(0);
}

int parse_vardecllist(void) {
    int tempI;
    token = lexer.GetToken();
    //while (token.token_type == ID) {
    //    tempTokenType = lexer.UngetToken(token);
    //    tempI = parse_vardecl();
    //}
    if (token.token_type == ID) {
        tempTokenType = lexer.UngetToken(token);
        tempI = parse_vardecl();
        parse_vardecllist();
        return 0;
    }
    lexer.UngetToken(token);
    return(0);
}

int parse_globalVars(void) {
    token = lexer.GetToken();
    int tempI;

    //check first set of var_list SEMICOLON
    if (token.token_type == ID) {
        tempTokenType = lexer.UngetToken(token);
        tempI = parse_vardecllist();
    }
    else {
        output_syntaxError();
    }
    return(0);

}


int parse_program(void) {

    token = lexer.GetToken();
    int tempI;
    while (token.token_type != END_OF_FILE)
    {
        // Check first set of global_vars scope
        if (token.token_type == ID) {
            tempTokenType = lexer.UngetToken(token);

            tempI = parse_globalVars();
            tempI = parse_body();

        }
        else if (token.token_type == LBRACE) {
            tempTokenType = lexer.UngetToken(token);
            tempI = parse_body();
        }
        else if (token.token_type == END_OF_FILE) {
            return(0);
        }
        else {
            output_syntaxError();
            return(0);
        }
        token = lexer.GetToken();
    }
}


int main()
{
    parse_program();
    output();
}
