/*
 * Copyright (C) Rida Bazzi, 2016
 *
 * Do not share this file with anyone
 */
#include <iostream>
#include <fstream>
#include <istream>
#include <vector>
#include <string>
#include <cstdio>

#include "inputbuf.h"

using namespace std;

bool InputBuffer::EndOfInput()
{
    if (!input_buffer.empty())
        return false;
    else
        return cin.eof();
}

char InputBuffer::UngetChar(char c)
{
    if (c != EOF)
        input_buffer.push_back(c);;
    return c;
}

void InputBuffer::GetChar(char& c)
{
    if (!input_buffer.empty()) {
        c = input_buffer.back();
        input_buffer.pop_back();
    } else {
        cin.get(c);
    }
}

string InputBuffer::UngetString(string s)
{
    for (int i = 0; i < s.size(); i++)
        input_buffer.push_back(s[s.size()-i-1]);
    return s;
}

void InputBuffer::readFile(std::string fileAbsPath) {
    std::string tempStr = "",str;
    std::ifstream in(fileAbsPath);
    while (std::getline(in, str))
    {
        if(str.size() > 0)
            tempStr = tempStr + str + "\n";
    }
    // tempStr = "";
    // std::ifstream myfile;
    // myfile.open(fileAbsPath);
    // if ( myfile.is_open() ) myfile >> tempStr;
    //cout << tempStr << endl;
    for (int i = tempStr.size(); i >= 0; i--) {
        input_buffer.push_back(tempStr[i]);
    }
}