/*
 * Copyright (C) Rida Bazzi, 2017
 *
 * Do not share this file with anyone
 */
#include <cstdio>
#include <cstdlib>
#include <cstdarg>
#include <cctype>
#include <cstring>
#include <string>
#include "compiler.h"
#include "lexer.h"
#include <iostream>
using namespace std;


#define DEBUG 0     // 1 => Turn ON debugging, 0 => Turn OFF debugging

int mem[1000];
vector<string> mem_name_index_map;
LexicalAnalyzer lexer;

std::vector<int> inputs;
int next_input = 0;

struct InstructionNode* inst_list;
struct InstructionNode* last_inst;

void parse_for();
void parse_while();
void parse_if();
void parse_switch();
struct InstructionNode* parse_condition();
struct InstructionNode* parse_assign_stmt();
void parse_input();
void parse_output();
void parse_expression(int* operand1_index_out, ArithmeticOperatorType* op_out, int* operand2_index_out);
void parse_stmtlist();
int parse_primary();
void parse_case_list(int case_var_memIndex, vector<struct InstructionNode*>& JMP_to_switch_end, vector<struct InstructionNode*>& CJMP_list);
bool parse_default_case(vector<struct InstructionNode*>& JMP_to_switch_end, vector<struct InstructionNode*>& CJMP_list);
bool parse_case(int case_var_memIndex, struct InstructionNode** cjmp_out, struct InstructionNode** break_out);


void syntax_error() {
    cout << "Syntax Error" << endl;
    exit(0);
}

void debug(const char* format, ...)
{
    va_list args;
    if (DEBUG)
    {
        va_start (args, format);
        vfprintf (stdout, format, args);
        va_end (args);
    }
}

int mem_find(string id) {
    for (int i = 0; i < mem_name_index_map.size(); i++) {
        if (mem_name_index_map[i] == id) {
            return i;
        }
    }
    return -1;
}

int mem_add(string id) {
    int ret = mem_find(id);
    if (ret < 0) {
        ret = mem_name_index_map.size();
        mem_name_index_map.push_back(id);
    }
    return ret;
}

int mem_token(Token token) {
    if (token.token_type == NUM) {
        int ret = mem_add(token.lexeme);
        mem[ret] = stoi(token.lexeme);
        return ret;
    }
    else if (token.token_type == ID) {
        return mem_find(token.lexeme);
    }
    else {
        return -1;
    }
}

void push_inst(struct InstructionNode* inst) {
    if (inst_list == NULL) {
        inst_list = inst;
        last_inst = inst;
        return;
    }

    last_inst->next = inst;
    last_inst = inst;
}
struct InstructionNode* get_inst_end() {
    return last_inst;
}

void parse_var_list() {
    Token token = lexer.GetToken();
   
    if (token.token_type == ID) {
        mem_add(token.lexeme);
        parse_var_list();
    }
    else if (token.token_type == COMMA) {
        parse_var_list();
    }
    else if (token.token_type == SEMICOLON) {
        return;
    }
}

void parse_stmt_body() {
    Token token = lexer.GetToken();

    if (token.token_type == LBRACE) {
        parse_stmtlist();
        token = lexer.GetToken();
        if (token.token_type == RBRACE) {
            return;
        }
        else {
            return;
        }
    }
    else if (token.token_type == END_OF_FILE) {
        return;
    }
    else {
        return;
    }
}

void parse_stmtlist() {

    Token token = lexer.GetToken();
    if (token.token_type == RBRACE) {
        lexer.UngetToken(1);
        return;
    }
    if (token.token_type == FOR || token.token_type == IF || token.token_type == WHILE || token.token_type == SWITCH ||
        token.token_type == ID || token.token_type == OUTPUT || token.token_type == INPUT) {
        lexer.UngetToken(1);
        parse_for();
        parse_if();
        parse_while();
        parse_switch();
        auto temp = parse_assign_stmt();
        if (temp != NULL) {
            push_inst(temp);
        }
        parse_output();
        parse_input();
    }
    else {
        syntax_error();
    }
    parse_stmtlist();
}

void parse_for(){
    Token token = lexer.GetToken();
    if (token.token_type != FOR) {
        lexer.UngetToken(1);
        return;
    }
    struct InstructionNode* cjmp;
    struct InstructionNode* self_increasement; // e.g. i++
    if (lexer.GetToken().token_type != LPAREN) {
        syntax_error();
    }
    push_inst(parse_assign_stmt()); // e.g. i = 0
    cjmp = parse_condition(); //i < x
    if (lexer.GetToken().token_type != SEMICOLON) {
        syntax_error();
    }
    push_inst(cjmp);
    self_increasement = parse_assign_stmt(); //e.g. i = i + 1

    if (lexer.GetToken().token_type != RPAREN) {
        syntax_error();
    }

    parse_stmt_body();
    push_inst(self_increasement);

    struct InstructionNode* jmp = new struct InstructionNode;
    jmp->next = NULL;
    jmp->type = JMP;
    jmp->jmp_inst.target = cjmp; //jump to condiction 
    push_inst(jmp);

    struct InstructionNode* noop = new struct InstructionNode;
    noop->next = NULL;
    noop->type = NOOP;
    push_inst(noop);
    cjmp->cjmp_inst.target = noop;//if condiction is flase jump to noop
}

struct InstructionNode* parse_condition() {
    struct InstructionNode* cjmp = new InstructionNode;
    cjmp->next = NULL;
    cjmp->type = CJMP;
    int left_mem_index = parse_primary();
    cjmp->cjmp_inst.operand1_index = left_mem_index;
    Token token = lexer.GetToken();
    if (token.token_type == GREATER) {
        cjmp->cjmp_inst.condition_op = CONDITION_GREATER;
    }
    else if (token.token_type == LESS) {
        cjmp->cjmp_inst.condition_op = CONDITION_LESS;
    }
    else if (token.token_type == NOTEQUAL) {
        cjmp->cjmp_inst.condition_op = CONDITION_NOTEQUAL;
    }

    int right_mem_index = parse_primary();
    cjmp->cjmp_inst.operand2_index = right_mem_index;

    return cjmp;
}

int parse_primary() { // return the index in memory
    Token token = lexer.GetToken();
    if (token.token_type == ID) {
        return mem_find(token.lexeme);
    }
    else if (token.token_type == NUM) {
        int ret = mem_add(token.lexeme);
        mem[ret] = stoi(token.lexeme);
        return ret;
    }
    else {
        return -1;
    }
}

void parse_while() {
    Token token = lexer.GetToken();
    if (token.token_type != WHILE) {
        lexer.UngetToken(1);
        return;
    }
    struct InstructionNode* cjmp;
    cjmp = parse_condition(); //i < x

    push_inst(cjmp);

    parse_stmt_body();

    struct InstructionNode* jmp = new struct InstructionNode;
    jmp->next = NULL;
    jmp->type = JMP;
    jmp->jmp_inst.target = cjmp; //jump to condiction 
    push_inst(jmp);

    struct InstructionNode* noop = new struct InstructionNode;
    noop->next = NULL;
    noop->type = NOOP;
    push_inst(noop);
    cjmp->cjmp_inst.target = noop;//if condiction is flase jump to noop
}

void parse_if() {
    Token token = lexer.GetToken();
    if (token.token_type != IF) {
        lexer.UngetToken(1);
        return;
    }

    struct InstructionNode* cjmp;
    cjmp = parse_condition(); //i < x

    push_inst(cjmp);

    parse_stmt_body();

    struct InstructionNode* noop = new struct InstructionNode;
    noop->next = NULL;
    noop->type = NOOP;
    push_inst(noop);
    cjmp->cjmp_inst.target = noop;//if condiction is flase jump to noop
}

void parse_switch() {
    Token token = lexer.GetToken();
    if (token.token_type != SWITCH) {
        lexer.UngetToken(1);
        return;
    }
    
    token = lexer.GetToken();
    if (token.token_type != ID) {
        syntax_error();
    }
    int case_var_memIndex = mem_find(token.lexeme);//save the index of ID in the memory location

    vector<struct InstructionNode*> JMP_to_switch_end ;
    vector<struct InstructionNode*> CJMP_list;

    if (lexer.GetToken().token_type != LBRACE) {
        syntax_error();
    }

    parse_case_list(case_var_memIndex, JMP_to_switch_end, CJMP_list);
    bool hasDefault = parse_default_case(JMP_to_switch_end, CJMP_list);

    struct InstructionNode* noop = new struct InstructionNode;
    noop->next = NULL;
    noop->type = NOOP;
    push_inst(noop);
    for (int i = 0; i < JMP_to_switch_end.size(); i++) {
        JMP_to_switch_end[i]->jmp_inst.target = noop;
    }
    if (!hasDefault) {
        CJMP_list.back()->cjmp_inst.target = noop;
    }

    if (lexer.GetToken().token_type != RBRACE) {
        syntax_error();
    }
}


void parse_case_list(int case_var_memIndex, vector<struct InstructionNode*>& JMP_to_switch_end, vector<struct InstructionNode*>& CJMP_list) {
    
    while (true) {
        struct InstructionNode* cjmp;
        struct InstructionNode* break_inst;
        if (parse_case(case_var_memIndex, &cjmp, &break_inst)) {
            if (CJMP_list.size() > 0) {
                CJMP_list.back()->cjmp_inst.target = cjmp;
            }
            CJMP_list.push_back(cjmp);
            JMP_to_switch_end.push_back(break_inst);
        }
        else {
            break;
        }
    }
}

bool parse_case(int case_var_memIndex, struct InstructionNode** cjmp_out, struct InstructionNode** break_out){ // return true if find any case body
    if (lexer.GetToken().token_type != CASE) {
        lexer.UngetToken(1);
        return false;
    }

    Token token = lexer.GetToken();
    if (token.token_type != NUM) {
        syntax_error();
    }

    *cjmp_out = new struct InstructionNode;
    (*cjmp_out)->next = NULL;
    (* cjmp_out)->type = CJMP;
    (*cjmp_out)->cjmp_inst.condition_op = CONDITION_EQUAL;
    (*cjmp_out)->cjmp_inst.operand1_index = case_var_memIndex;
    (*cjmp_out)->cjmp_inst.operand2_index = mem_token(token);
    push_inst(*cjmp_out);
    if (lexer.GetToken().token_type != COLON) {
        syntax_error();
    }

    parse_stmt_body();

    struct InstructionNode* jmp = new struct InstructionNode;
    jmp->next = NULL;
    jmp->type = JMP;
    push_inst(jmp);
    *break_out = jmp;

    return true;
}

bool parse_default_case(vector<struct InstructionNode*>& JMP_to_switch_end, vector<struct InstructionNode*>& CJMP_list) {
    if (lexer.GetToken().token_type != DEFAULT) {
        lexer.UngetToken(1);
        return false;
    }
    if (lexer.GetToken().token_type != COLON) {
        syntax_error();
    }
    struct InstructionNode* last_inst_record = get_inst_end();
    parse_stmt_body();
    if (CJMP_list.size() > 0) {
        CJMP_list.back()->cjmp_inst.target = last_inst_record->next;
    }

    struct InstructionNode* jmp = new struct InstructionNode;
    jmp->next = NULL;
    jmp->type = JMP;
    push_inst(jmp);
    JMP_to_switch_end.push_back(jmp);
    return true;
}


struct InstructionNode* parse_assign_stmt() {
    Token token = lexer.GetToken();
    if (token.token_type == ID) {
        struct InstructionNode* assign_inst = new InstructionNode;
        assign_inst->next = NULL;
        assign_inst->type = ASSIGN;
        assign_inst->assign_inst.left_hand_side_index = mem_find(token.lexeme);
        token = lexer.GetToken();
        if (token.token_type == EQUAL) {
            parse_expression(
                &(assign_inst->assign_inst.operand1_index), 
                &(assign_inst->assign_inst.op),
                &(assign_inst->assign_inst.operand2_index)
            );
            token = lexer.GetToken();
            if (token.token_type == SEMICOLON) {
                return assign_inst;
            }
        }
    }
    else {
        lexer.UngetToken(1);
        return NULL;
    }
    syntax_error();
}

void parse_expression(int* operand1_index_out, ArithmeticOperatorType* op_out, int* operand2_index_out) {
    if (lexer.peek(2).token_type != SEMICOLON) { // + - * /
        if (lexer.peek(2).token_type == PLUS) {
            *operand1_index_out = mem_token(lexer.peek(1));
            *op_out = OPERATOR_PLUS;
            *operand2_index_out = mem_token(lexer.peek(3));
        }
        else if (lexer.peek(2).token_type == MINUS) {
            *operand1_index_out = mem_token(lexer.peek(1));
            *op_out = OPERATOR_MINUS;
            *operand2_index_out = mem_token(lexer.peek(3));
        }
        else if (lexer.peek(2).token_type == MULT) {
            *operand1_index_out = mem_token(lexer.peek(1));
            *op_out = OPERATOR_MULT;
            *operand2_index_out = mem_token(lexer.peek(3));
        }
        else if (lexer.peek(2).token_type == DIV) {
            *operand1_index_out = mem_token(lexer.peek(1));
            *op_out = OPERATOR_DIV;
            *operand2_index_out = mem_token(lexer.peek(3));
        }
        lexer.GetToken();
        lexer.GetToken();
        lexer.GetToken();
    }
    else {
        *operand1_index_out = parse_primary();
        *op_out = OPERATOR_NONE;
    }
}

void parse_input(){
    if (lexer.GetToken().token_type != INPUT) {
        lexer.UngetToken(1);
        return;
    }
    Token token = lexer.GetToken();
    if (token.token_type != ID) {
        syntax_error();
    }
    struct InstructionNode* input_inst = new struct InstructionNode;
    input_inst->next = NULL;
    input_inst->type = IN;
    input_inst->input_inst.var_index = mem_find(token.lexeme);
    push_inst(input_inst);

    if (lexer.GetToken().token_type != SEMICOLON) {
        syntax_error();
    }
}

void parse_output() {
    if (lexer.GetToken().token_type != OUTPUT) {
        lexer.UngetToken(1);
        return;
    }
    Token token = lexer.GetToken();
    if (token.token_type != ID) {
        syntax_error();
    }
    struct InstructionNode* output_inst = new struct InstructionNode;
    output_inst->next = NULL;
    output_inst->type = OUT;
    output_inst->output_inst.var_index = mem_find(token.lexeme);
    push_inst(output_inst);

    if (lexer.GetToken().token_type != SEMICOLON) {
        syntax_error();
    }
}

void parse_input_num_list() {
    Token token = lexer.GetToken();
    while (token.token_type == NUM) {
        inputs.push_back(stoi(token.lexeme));
        token = lexer.GetToken();
    }
}

struct InstructionNode* parse_generate_intermediate_representation() {
    //lexer = LexicalAnalyzer();

    parse_var_list();

    inst_list = NULL;

    parse_stmt_body();

    parse_input_num_list();

    return inst_list;
}



