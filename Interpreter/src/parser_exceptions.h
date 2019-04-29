#pragma once

#include <exception>
#include <string>
#include <sstream>

class BadSyntaxException : public std::exception
{
private:
    std::string msg;
public:
    BadSyntaxException(std::string message) : msg(message) {};

    const char * what() const throw ()
    {
        return msg.c_str();
    }
};




class ParseException : public std::exception
{
private:
    std::string msg;
public:
    ParseException(std::string message) : msg(message) {};

    const char * what() const throw ()
    {
        return msg.c_str();
    }
};




class ParserBugException : public std::exception
{
private:
    std::string msg;
public:
    ParserBugException(std::string symbol)
    {
        std::stringstream ss;
        ss << "Software Bug! \"" << symbol << "\" object is not set up properly!";
        msg = ss.str();
    };

    const char * what() const throw ()
    {
        return msg.c_str();
    }
};