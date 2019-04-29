#pragma once

#include <string>
#include <vector>
#include <windows.h>

using namespace std;

void COLOR(unsigned short color);

#define RED(str) COLOR(4); std::cout << str; COLOR(15);
#define GREEN(str) COLOR(2); std::cout << str; COLOR(15);
#define YELLOW(str) COLOR(14); std::cout << str; COLOR(15);

bool isNumber(std::string str);
bool isAlphaNumeric(std::string str);

struct Token;
namespace Tokenizer {
    std::string compressWhiteSpaces(std::string str);
    std::string separateTokens(std::string str, std::vector<std::string> delimiters);

    std::vector<Token> tokenize(std::string str, int line_number);
    std::vector<Token> getTokensFromFile(std::string path, std::vector<std::string> delimiters);
}
