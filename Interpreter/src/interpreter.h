#pragma once

#include <string>
#include <vector>
#include <map>
#include "operations.h"
#include "../include/adapter_interface.h"

class Interpreter
{
public:
    VariableSet stack;
    IAdapter * const adapter;
private:
    std::map<std::string, FUNC_PTR> functions;
    std::vector<VariableSet> callStack;

public:
    Interpreter(IAdapter * const adapter);

    void Load(std::vector<std::string> filePaths);
    std::vector<bool> Execute(std::vector<std::string> functions);
    std::vector<std::string> GetLoadedFunctions() const;

    FUNC_PTR getFunction(std::string id) const;

    void PushStack();
    void PopStack();

    std::string ValToStr(VarValue val) const;
};

