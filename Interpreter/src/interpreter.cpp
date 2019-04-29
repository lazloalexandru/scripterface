#include "../include/script_interpreter.h"
#include "parser.h"
#include "trace.h"
#include "utils.h"
#include "operations.h"
#include "operation_exceptions.h"

using namespace std;



ScriptInterpreter::ScriptInterpreter(IAdapter * const adapter)
    : interpreter(adapter)
{   
}

vector<bool> ScriptInterpreter::Execute(vector<string> functions)
{
    return interpreter.Execute(functions);
}

void ScriptInterpreter::Load(vector<string> filePaths)
{
    interpreter.Load(filePaths);
}

vector<string> ScriptInterpreter::GetLoadedFunctions()
{
    return interpreter.GetLoadedFunctions();
}




Interpreter::Interpreter(IAdapter * const adptr)
    : adapter(adptr)
{
    if (!adapter) throw std::invalid_argument("Adapter is null!");
}

void Interpreter::Load(vector<string> paths)
{
    _TRACE_("        " << __FUNCTION__ << endl);

    LangParser parser;

    cout << endl << "Parsing files ..." << endl;
    _TRACE_("        " << "Parsing files ..." << endl);

    functions.clear();

    for (auto path : paths)
    {
        _TRACE_("        Parsing file: \"" << path << "\"" << endl);
        vector<FUNC_PTR> res = parser.parse(path);

        for (auto func : res)
        {
            auto it = functions.find(func->getId());
            if (it != functions.end())
            {
                _TRACE_("        PARSER ERROR: In file " << path << "\" @line: " << func->t.line <<
                    " function: \"" << func->getId() << " already defined in another rule file @line: " << (*it).second->t.line << endl);
                RED(endl << "PARSER ERROR: In file " << path << "\" @line: " << func->t.line <<
                    " function: \"" << func->getId() << " already defined in another rule file @line: " << (*it).second->t.line << endl);
            }
            else
            {
                functions[func->getId()] = func;

            }
        }

        cout << endl;
    }

    cout << endl;

    _TRACE_("        " << "Finished parsing files." << endl);
}

vector<bool> Interpreter::Execute(vector<string> functionIds)
{       
    vector<bool> results(functionIds.size());

    _TRACE_("        " << "Executing operations ..." << endl);

    int idx = 0;
    for (auto functionId : functionIds)
    {
        auto it = functions.find(functionId);

        if (it != functions.end() && it->second)
        {
            FUNC_PTR fn = it->second;

            if (fn->getParams().size() > 0)
            {
                _TRACE_("Function with name \"" << fn->getId() << "\" has " << fn->getParams().size() << " parameters. Cannot be executed directly." << endl);
                RED("Function with name \"" << fn->getId() << "\" has " << fn->getParams().size() << " parameters. Cannot be executed directly." << endl);
            }
            else
            {
                try
                {
                    stack.Clear();

                    cout << "        [EXECUTING: " << fn->getId() << "]" << endl;
                    _TRACE_("        EXECUTING FUNCTION: \"" << fn->getId() << "\"" << endl);

                    fn->Exec(*this);

                    results[idx] = stack.PopReturn().size() > 0; // save return value 

                    cout << endl << "        [FINISHED: " << functionId << "] => " << results[idx] << endl << endl;
                    _TRACE_(endl << "        FINISHED: " << functionId << " => " << results[idx] << endl);
                }
                catch (const RuntimeException& e)
                {
                    RED("RUNTIME ERROR: " << e.what());
                    _TRACE_("        RUNTIME ERROR: " << e.what() << endl);
                }
            }
        }
        else
        {
            _TRACE_("        ERROR: Function with id: " << functionId <<" does not exist!" << endl);
        }

        idx++;
    }

    _TRACE_("        " << "Finished executing operations ..." << endl);

    return results;
}

vector<string> Interpreter::GetLoadedFunctions() const
{
    vector<string> fnames;
    for (auto f : functions)
    {
        fnames.push_back(f.first);
    }
    return fnames;
}

FUNC_PTR Interpreter::getFunction(string id) const
{
    FUNC_PTR res;

    auto it = functions.find(id);
    if (it != functions.end()) { res = it->second; }

    return res;
}

void Interpreter::PushStack()
{
    callStack.push_back(stack);
}

void Interpreter::PopStack()
{
    stack = callStack.back();
    callStack.pop_back();
}

string Interpreter::ValToStr(VarValue val) const
{
    stringstream ss;

    if (val.empty())
    {
        ss << "{}"; // empty value vector
    }
    else
    {
        for (auto node : val)  // iterate through all nodes of the Value vector
        {
            if (node)
            {
                ATTR_PTR attributes = adapter->GetAttributesOf(node);
                if (attributes)
                {
                    ss << attributes->ToString();
                }
            }
            else
            {
                ss << "{ 0 }"; // node pointer is null
            }
        }
    }

    return ss.str();
}
