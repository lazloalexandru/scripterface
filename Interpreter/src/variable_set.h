#pragma once

#include <string>
#include <map>
#include "types.h"

class Interpreter;
class VariableSet
{
private:
    VarValue _return;
    std::map<std::string, VarValue> Args;

public:
    void Add(std::string id, VarValue value);
    void Remove(std::string id);

    VarValue GetValue(std::string id) const;
    bool Exists(std::string id) const;

    void PushReturn(VarValue val);
    VarValue PopReturn();
    
    void Clear();
    std::string ToStr(Interpreter const &) const;

    VariableSet();
};
