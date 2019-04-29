#pragma once

#include <string>
#include <vector>
#include "adapter_interface.h"
#include "..\src\interpreter.h"

typedef std::vector<std::string> StrVec;

class ScriptInterpreter
{
    Interpreter interpreter;
public:
    ScriptInterpreter(IAdapter * const adapter);

    void Load(StrVec filePaths);
    StrVec GetLoadedFunctions();

    std::vector<bool> Execute(StrVec functions);
};