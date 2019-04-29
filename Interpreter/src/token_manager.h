#pragma once

#include <string>
#include <vector>
#include "token.h"

class TokenManager
{
private:
    unsigned int current;
    unsigned int backup;
    std::vector<Token> tokens;    
public:
    TokenManager();
    void SetTokens(std::vector<Token> tokens);
    Token Current(bool skipSpaces = true);
    Token GetNext(bool skipSpaces = true);
    bool NoMoreTokens(bool skipSpaces = true);
    void SaveCurrent();
    void RestoreCurrent();
};