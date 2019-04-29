#include "variable_set.h"
#include "../include/script_interpreter.h"
#include "operation_exceptions.h"
#include "utils.h"
#include "trace.h"
#include "types.h"
#include <sstream>


using namespace std;

VariableSet::VariableSet() 
{
}

void VariableSet::Add(std::string id, VarValue value)
{
    Args[id] = value;
}

void VariableSet::PushReturn(VarValue val)
{
    _return = val;
}

VarValue VariableSet::PopReturn()
{
    return _return;
}

void VariableSet::Remove(std::string id)
{
    auto it = Args.find(id);
    if (it != Args.end())
    {
        Args.erase(it);
    }
}

void VariableSet::Clear()
{
    Args.clear();
    _return.clear();
}

bool VariableSet::Exists(std::string id) const
{
    return Args.find(id) != Args.end();
}

VarValue VariableSet::GetValue(std::string id) const
{
    VarValue val;

    auto it = Args.find(id);
    if (it != Args.end())
    {
        val = it->second;
    }

    return val;
}


string VariableSet::ToStr(Interpreter const & interpreter) const
{
    stringstream ss;

    ss << "(";

    for (auto arg : Args)
    {
        ss << " " << arg.first << " ";   /// append variable name 

        ss << interpreter.ValToStr(arg.second);

        ss << " ";
    }
    /*
    ss << " return ";
    ss << ValToStr(_return);
    ss << " ";
    */
    ss << ")";

    return ss.str();
}