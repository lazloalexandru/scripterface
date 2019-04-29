#include "utils.h"
#include "token.h"
#include <regex>
#include <sstream>
#include <fstream>
#include <iostream>


using namespace std;

const std::string TOKENIZER_SYMBOL = "\xff"; // using ASCII char with code 255 as symbol


bool isNumber(string str)
{
    const regex rx("\\d+");
    return regex_match(str, rx);
}

bool isAlphaNumeric(string str)
{
    const regex rx("^[a-zA-Z0-9_]+$");
    return regex_match(str, rx);
}


string Tokenizer::compressWhiteSpaces(string str)
{
    regex ws("\\t+|\\n+|\\r+");
    return regex_replace(str, ws, TOKENIZER_SYMBOL); 
}

string Tokenizer::separateTokens(string str, vector<string> delimiters)
{
    for (auto s : delimiters)
    {
        string wide(TOKENIZER_SYMBOL + s + TOKENIZER_SYMBOL);

        auto pos = str.find(s);
        while (string::npos != pos)
        {
            str.replace(pos, s.length(), wide);
            pos = str.find(s, pos + wide.length());
        }
    }

    return str;
}


vector<Token> Tokenizer::tokenize(string str, int line_number)
{
    vector<Token> tokens;
    string token;
    stringstream ss(str);

    while (getline(ss, token, TOKENIZER_SYMBOL[0]))
    {
        Token t = { token, line_number };
        if (token[0]) tokens.push_back(t); // skip the \0 termination character
    }

    return tokens;
}


vector<Token> Tokenizer::getTokensFromFile(string path, vector<string> delimiters)
{
    vector<Token> tokens;

    ifstream ifs;
    ifs.open(path);

    if (ifs.is_open() && ifs.good() && ifs)   // add condition here
    {
        int line_count = 0;
        string line;

        while (std::getline(ifs, line))
        {
            line_count++;

            if (!line.empty() && line[0] != ';')
            {
                line = separateTokens(line, delimiters);
                line = compressWhiteSpaces(line);

                vector<Token> lineTokens = tokenize(line, line_count);

                tokens.insert(tokens.end(), lineTokens.begin(), lineTokens.end());
            }
        }
    }
    else {
        stringstream ss;
        ss << "Could not open file: " << path;
        throw invalid_argument(ss.str().c_str());
    }

    return tokens;
}


/*
Here, \033 is the ESC character, ASCII 27. It is followed by [, then zero
or more numbers separated by ;, and finally the letter m.
The numbers describe the colour and format to switch to from that point onwards.

foreground background
black        30         40
red          31         41
green        32         42
yellow       33         43
blue         34         44
magenta      35         45
cyan         36         46
white        37         47

reset             0  (everything back to normal)
bold/bright       1  (often a brighter shade of the same colour)
underline         4
inverse           7  (swap foreground and background colours)
bold/bright off  21
underline off    24
inverse off      27
*/

void COLOR(unsigned short color)
{
    static HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, color);
}

