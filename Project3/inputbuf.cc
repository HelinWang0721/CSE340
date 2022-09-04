/*
 * Copyright (C) Rida Bazzi, 2016
 *
 * Do not share this file with anyone
 */
#include <iostream>
#include <istream>
#include <vector>
#include <string>
#include <cstdio>
#include <fstream>
#include <iterator>


#include "inputbuf.h"


using namespace std;

bool InputBuffer::EndOfInput()
{
#ifndef VS
    if (!input_buffer.empty())
        return false;
    else
        return cin.eof();
#endif // !VS

#ifdef VS
    return (input_buffer.empty());
#endif // VS
}

char InputBuffer::UngetChar(char c)
{
    if (c != EOF)
        input_buffer.push_back(c);
    return c;
}

void InputBuffer::GetChar(char& c)
{
    if (!input_buffer.empty()) {
        c = input_buffer.back();
        input_buffer.pop_back();
    } else {
        
#ifndef VS
        cin.get(c);
#endif // !VS

#ifdef VS
        c = EOF;
#endif // VS
    }
}

string InputBuffer::UngetString(string s)
{
    for (int i = 0; i < s.size(); i++)
        input_buffer.push_back(s[s.size()-i-1]);
    return s;
}


#ifdef VS
void InputBuffer::readFile() {
    char a[3];
    vector<char> chars;
    ifstream file("C:\\Users\\13681\\Desktop\\HL-P3\\tests\\test_pdf_08.txt");

    typedef std::istreambuf_iterator<char> buf_iter;
    for (buf_iter i(file), e; i != e; ++i) {
        char c = *i;
        input_buffer.insert(input_buffer.begin(), c);
    }

    a[0] = 10;
}
#endif // VS