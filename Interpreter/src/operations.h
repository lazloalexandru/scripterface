#pragma once

#include <vector>
#include <string>
#include "types.h"
#include "operation.h"
#include "token.h"



class Function : public Operation
{
private:
    std::string id;
    std::vector<OP_PTR> operations;
    std::vector<std::string> parameters;
public:
    Function(std::string functionId, std::vector<OP_PTR> ops, std::vector<std::string> parameters, Token idTok);
    std::string getId() const { return id; }
    std::vector<std::string> getParams() const { return parameters; }
    void Exec(Interpreter & interpreter);
};

typedef std::shared_ptr<Function> FUNC_PTR;



class FunctionCall : public Operation
{
private:
    std::string id;
    std::vector<OP_PTR> parameters;
public:
    FunctionCall(std::string functionId, std::vector<OP_PTR> parameters, Token idTok);
    void Exec(Interpreter & interpreter);
};



class SetFunctionValue : public Operation
{
private:
    std::string functionId;
    OP_PTR op;
public:
    SetFunctionValue(std::string ruleId, OP_PTR op, Token idTok);
    void Exec(Interpreter & interpreter);
};

class ConditionalBlock : public Operation
{
private:
    std::vector<OP_PTR> const sBlocks;  // serial blocks => AND
    std::vector<OP_PTR> const pBlocks;  // paralel blocks => OR
public:
    ConditionalBlock(std::vector<OP_PTR> sBlocks, std::vector<OP_PTR> pBlocks, Token idTok);
    ConditionalBlock(Token idTok);      // for atomic elements
    virtual void Exec(Interpreter & interpreter);
};


class IfStmt : public Operation
{
private:
    const OP_PTR condition;
    std::vector<OP_PTR> thanOperations;
    std::vector<OP_PTR> elseOperations;
public:
    IfStmt(OP_PTR cond, std::vector<OP_PTR> thanOps, std::vector<OP_PTR> elseOps, Token idTok);
    void Exec(Interpreter & interpreter);
};



class WhileLoop : public Operation
{
private:
    const OP_PTR condition;
    std::vector<OP_PTR> operations;
public:
    WhileLoop(OP_PTR cond, std::vector<OP_PTR> ops, Token idTok);
    void Exec(Interpreter & interpreter);
};




class ForLoop : public Operation
{
private:
    std::string id;
    const OP_PTR dataGetter;
    std::vector<OP_PTR> operations;
public:
    ForLoop(std::string i, OP_PTR d, std::vector<OP_PTR> o, Token idTok);
    void Exec(Interpreter & interpreter);
};



class Variable : public Operation
{
private:
    std::string id;
public:
    Variable(std::string varId, Token idTok);
    void Exec(Interpreter & interpreter);
};


class VariableAssignment : public Operation
{
private:
    std::string id;
    const OP_PTR valueGetter;
public:
    VariableAssignment(std::string varId, OP_PTR valueGetter, Token idTok);
    void Exec(Interpreter & interpreter);
};



class NOT : public Operation
{
private:
    const OP_PTR expression;
public:
    NOT(OP_PTR expression, Token idTok);
    void Exec(Interpreter & interpreter);
};



class TRUE_VAL : public Operation
{
public:
    TRUE_VAL(Token idTok);
    void Exec(Interpreter & interpreter);
};



class FALSE_VAL : public Operation
{
public:
    FALSE_VAL(Token idTok);
    void Exec(Interpreter & interpreter);
};



class NullCheck : public Operation
{
private:
    std::string varId;
public:
    NullCheck(std::string variableId, Token idTok);
    void Exec(Interpreter & interpreter);
};


class PRINTS : public Operation
{
private:
    std::string msg;
public:
    PRINTS(std::string message, Token idTok);
    void Exec(Interpreter & interpreter);
};



class PRINT : public Operation
{
private:
    const OP_PTR operation;
public:
    PRINT(OP_PTR op, Token idTok);
    void Exec(Interpreter & interpreter);
};



class CONDITIONS_OF : public Operation
{
private:
    const OP_PTR operation;
public:
    CONDITIONS_OF(OP_PTR op, Token idTok);
    void Exec(Interpreter & interpreter);
};



class PARENT : public Operation
{
private:
    const OP_PTR operation;
public:
    PARENT(OP_PTR op, Token idTok);
    void Exec(Interpreter & interpreter);
};



class GET_NODES_OF_TYPE : public Operation
{
private:
    std::string typeName;
public:
    GET_NODES_OF_TYPE(std::string nodeTypeName, Token idTok);
    void Exec(Interpreter & interpreter);
};


enum CheckType
{
    EQUAL,
    STARTS_WITH,
    ENDS_WITH,
    CONTAINS,
    LT, // lower than
    GT, // greater than
    LE, // lower or equal
    GE  // rgeater or equal
};

class AttributeCheck : public Operation
{
private:
    std::string varId;
    std::string attributeId;
    std::string attribVal;
    CheckType checkType;
private:
    int GetValue(std::string val);
public:
    AttributeCheck(std::string variableId, std::string attributeName, std::string attributeValue, CheckType checkType, Token idTok);
    void Exec(Interpreter & interpreter);
};



class CompareAttribute : public Operation
{
private:
    std::string varId1;
    std::string attributeId1;
    std::string prefix1;
    std::string postfix1;

    std::string varId2;
    std::string attributeId2;
    std::string prefix2;
    std::string postfix2;

    CheckType checkType;
private:
    int GetValueL(std::string val) const;
    int GetValueR(std::string val) const;
public:
    CompareAttribute(
        std::string variableId1, std::string attributeName1, std::string prefix1, std::string postfix1,
        std::string variableId2, std::string attributeName2, std::string prefix2, std::string postfix2,
        CheckType checkType, Token idTok);
    void Exec(Interpreter & interpreter);
};