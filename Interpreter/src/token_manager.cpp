#include "token_manager.h"
#include "parser_exceptions.h"
#include <sstream>

using namespace std;

TokenManager::TokenManager()
    : current(0), backup(0)
{
}

void TokenManager::SetTokens(std::vector<Token> t)
{
    tokens = t;
    current = 0;
}

Token TokenManager::Current(bool skipSpaces)
{
    Token t;

    while (skipSpaces && " " == tokens[current].tok && current < tokens.size())
    {
        current++;
    }

    if (current < tokens.size())
    {
        t = tokens[current];
    }
    else
    {
        stringstream ss;
        ss << "No more tokens available!";
        throw BadSyntaxException(ss.str().c_str());
    }

    return t;
}

bool TokenManager::NoMoreTokens(bool skipSpaces)
{
    while (skipSpaces && current < tokens.size() && " " == tokens[current].tok)
    {
        current++;
    }

    return tokens.size()==current;
}

Token TokenManager::GetNext(bool skipSpaces)
{
    Token t;

    while (skipSpaces && current < tokens.size() && " " == tokens[current].tok)
    {
        current++;
    }

    if (current < tokens.size())
    {
        t = tokens[current++];
    }
    else
    {
        stringstream ss;
        ss << "No more tokens available!";
        throw ParseException(ss.str().c_str());
    }

    return t;
}

void TokenManager::SaveCurrent()
{
    backup = current;
}

void TokenManager::RestoreCurrent()
{
    current = backup;
}