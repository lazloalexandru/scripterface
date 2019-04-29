#pragma once

#include <string>
#include <vector>
#include <memory>

class CSyntaxNode;

class IAttributes
{
public:
    virtual bool Exists(std::string AttributeName) const = 0;
    virtual std::string GetValueOf(const std::string& AttributeName) const = 0;
    virtual std::string ToString() const = 0;
};


typedef std::shared_ptr<IAttributes> ATTR_PTR;


class IAdapter
{
public:
    virtual std::vector<CSyntaxNode*> GetParentsForNode(CSyntaxNode* p) const = 0;
    virtual std::vector<CSyntaxNode*> GetAllNodesOfType(std::string nodeTypeName, CSyntaxNode* root) const = 0;
    virtual std::vector <CSyntaxNode*> GetGeogConds(CSyntaxNode* p) const = 0;
    virtual std::shared_ptr<IAttributes> GetAttributesOf(CSyntaxNode* p) const = 0;
    virtual bool Exists(std::string nodeType) const = 0;
    virtual bool Exists(unsigned int nodeType) const = 0;
};


class AdapterException : public std::exception
{
    std::string msg;
public:
    AdapterException(std::string message) : msg(message) {};
    const char * what() const throw () { return msg.c_str(); }
};