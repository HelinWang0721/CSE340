/*
 * Copyright (C) Rida Bazzi, 2016
 *
 * Do not share this file with anyone
 */
#ifndef __INPUT_BUFFER__H__
#define __INPUT_BUFFER__H__

#include <string>
#include <vector>

class InputBuffer {
  public:
    void GetChar(char&);
    char UngetChar(char);
    std::string UngetString(std::string);
    bool EndOfInput();
    
    void readFile(std::string fileAbsPath);

  private:
    std::vector<char> input_buffer;
};

#endif  //__INPUT_BUFFER__H__
