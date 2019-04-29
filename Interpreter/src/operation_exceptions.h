#pragma once

#include <exception>
#include <string>
#include <sstream>


class RuntimeException : public std::exception
{
private:
    std::string msg;
public:
    RuntimeException(std::string message) : msg(message) {};

    const char * what() const throw ()
    {
        return msg.c_str();
    }
};


class OperationBugException : public std::exception
{
private:
    std::string msg;
public:
    OperationBugException(std::string location) : msg(location) {};

    const char * what() const throw ()
    {
        std::stringstream ss;
        ss << "This is a bug in the Interpreter! @" << msg << std::endl;
        return ss.str().c_str();
    }
};
