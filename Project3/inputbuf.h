/*
 * Copyright (C) Rida Bazzi, 2017
 *
 * Do not share this file with anyone
 */

#ifndef __INPUT_BUFFER__H__
#define __INPUT_BUFFER__H__
#include <vector>
#include <string>

//#define VS

class InputBuffer {
public:
    InputBuffer();
    void GetChar(char&);
    char UngetChar(char);
    std::string UngetString(std::string);
    bool EndOfInput();
#ifdef VS
    void readFile();
#endif // VS

private:
    std::vector<char> input_buffer;
};

#endif  //__INPUT_BUFFER__H__
