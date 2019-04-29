#include "operations.h"
#include <iostream>
#include <sstream>
#include "trace.h"
#include "variable_set.h"
#include "utils.h"
#include "operation_exceptions.h"
#include "operation.h"
#include "../include/script_interpreter.h"

using namespace std;


IfStmt::IfStmt(OP_PTR cond, vector<OP_PTR> thanOps, vector<OP_PTR> elseOps, Token idTok)
    : condition(cond), thanOperations(thanOps), elseOperations(elseOps), Operation(idTok)
{   
    __TRACE_CONSTRUCT__; 
    if (!condition) { throw OperationBugException(__FUNCTION__ + __LINE__); }
}

void IfStmt::Exec(Interpreter & interpreter)
{
    __TRACE_EXEC__;
    
    condition->Exec(interpreter);
    VarValue val = interpreter.stack.PopReturn();

    if (!val.empty()) // evaluate the boolean value of condition statement .. empty vector means false
    {
        for (auto op : thanOperations)
        {
            if (!op) { throw OperationBugException(__FUNCTION__ + __LINE__); }
            op->Exec(interpreter);
        }
    }
    else
    {
        for (auto op : elseOperations)
        {
            if (!op) { throw OperationBugException(__FUNCTION__ + __LINE__); }
            op->Exec(interpreter);
        }
    }
}



ConditionalBlock::ConditionalBlock(Token idTok) // for atomic elements
    : Operation(idTok)
{
    __TRACE_CONSTRUCT__;
    _TRACE_("        NEW_ATOMIC >>> \"" << idTok.tok << "\" @line: " << idTok.line << endl);
}


ConditionalBlock::ConditionalBlock(vector<OP_PTR> SB, vector<OP_PTR> PB, Token idTok)
    : sBlocks(SB), pBlocks(PB), Operation(idTok)
{
    __TRACE_CONSTRUCT__;
    /* NOTES:
     - a conditional block can a series of OR blocks or a seriez of AND blocks or just a signgle element
    For an expression like: A =>
            => pBlocks has 1 elements { }
            => sBlocks has 0 elements { }
            => operation contains { [A] }
    For an expression like: A+B+C =>
            => pBlocks has 3 elements { [A], [B], [C] }
            => sBlocks has 0 elements { }
            => operation is NULL { }
    For an expression like: ABC =>
            => pBlocks has 0 elements { }
            => sBlocks has 3 elements { [A], [B], [C] }
            => operation is NULL { }
     For an expression like: AB+CD(EG+F)+H =>
            => pBlocks has 3 elements { [AB], [CD(EG+F)], [H] }
            => sBlocks has 0 elements { }
            => operation is NULL { }
     For an expression like: CD(EG+F) =>
            => pBlocks has 0 element { }
            => sBlocks has 3 elements { [C], [D], [(EG+F)]}
            => operation is NULL { }
    Either:
        pBlocks.size > 0 && sBlock.size ==0
        OR
        pBlocks.size == 0 && sBlock.size > 0
    */
    if ((pBlocks.size() > 0 && sBlocks.size() > 0) ||
        (pBlocks.size() == 0 && sBlocks.size() == 0))
    {
        throw OperationBugException(__FUNCTION__ + __LINE__);
    }
    _TRACE_("        NEW_BLOCK >>> SB: " << sBlocks.size() << " PB: " << pBlocks.size() << endl);
}

void ConditionalBlock::Exec(Interpreter & interpreter)
{
    __TRACE_EXEC__;

    VarValue retVal;

    if (sBlocks.size() == 1)
    {
        sBlocks[0]->Exec(interpreter);
        retVal = interpreter.stack.PopReturn();
    }
    else if (pBlocks.size() == 1)
    {
        pBlocks[0]->Exec(interpreter);
        retVal = interpreter.stack.PopReturn();
    }
    else if (sBlocks.size() > 1)
    {                           // here we evaluate a condition block constructed from ANDs (sBlocks)
        bool result = true;
        for (auto block : sBlocks)
        {
            block->Exec(interpreter);
            VarValue ret = interpreter.stack.PopReturn();
            result = result && (ret.size() > 0);
        }
        if (result) { retVal.push_back(0); } // return non empty vector => means true
    }
    else if (pBlocks.size() > 1)
    {                           // here we evaluate a condition block constructed from ORs (pBlocks)
        bool result = false;
        for (auto block : pBlocks)
        {
            block->Exec(interpreter);
            VarValue ret = interpreter.stack.PopReturn();
            result = result || (ret.size() > 0);
        }
        if (result) { retVal.push_back(0); } // return non empty vector => means true
    }
    else { throw OperationBugException(__FUNCTION__ + __LINE__); }

    interpreter.stack.PushReturn(retVal);
}



WhileLoop::WhileLoop(OP_PTR cond, vector<OP_PTR> ops, Token idTok)
    : condition(cond), operations(ops), Operation(idTok)
{
    __TRACE_CONSTRUCT__;
    if (!condition) { throw OperationBugException(__FUNCTION__ + __LINE__); }
}

void WhileLoop::Exec(Interpreter & interpreter)
{
    __TRACE_EXEC__;

    condition->Exec(interpreter);
    VarValue val = interpreter.stack.PopReturn();
    while (!val.empty()) // evaluate the boolean value of condition statement .. empty vector means false
    {
        for (auto op : operations)
        {
            if (!op) { throw OperationBugException(__FUNCTION__ + __LINE__); }
            op->Exec(interpreter);
        }

        condition->Exec(interpreter);
        val = interpreter.stack.PopReturn();
    }
}





ForLoop::ForLoop(std::string i, OP_PTR d, vector<OP_PTR> o, Token idTok)
    : id(i), dataGetter(d), operations(o), Operation(idTok)
{
    __TRACE_CONSTRUCT__;
    if (!dataGetter) { throw OperationBugException(__FUNCTION__ + __LINE__); }
}

void ForLoop::Exec(Interpreter & interpreter)
{
    __TRACE_EXEC__

    dataGetter->Exec(interpreter);
    VarValue dataSet = interpreter.stack.PopReturn();

    for (auto obj : dataSet)
    {
        VarValue loop_variable;
        loop_variable.push_back(obj);

        interpreter.stack.Add(id, loop_variable);

        for (auto op : operations)
        {
            if (!op) { throw OperationBugException(__FUNCTION__ + __LINE__); }
            op->Exec(interpreter);
        }

        interpreter.stack.Remove(id);
    }
}







Function::Function(string functionId, vector<OP_PTR> ops, vector<string> params, Token idTok)
    : id(functionId), operations(ops), parameters(params), Operation(idTok)
{
    __TRACE_CONSTRUCT__
}

void Function::Exec(Interpreter & interpreter)
{
    __TRACE_EXEC__;
    
    interpreter.stack.Add(id, VarValue()); // add variable for function return value

    for (auto op = operations.begin(); op != operations.end(); op++)
    {
        if (!*op) { throw OperationBugException(__FUNCTION__ + __LINE__); }
        (*op)->Exec(interpreter);
    }
    
    interpreter.stack.PushReturn(interpreter.stack.GetValue(id)); // set return value 
}




SetFunctionValue::SetFunctionValue(std::string id, OP_PTR operation, Token idTok)
    : functionId(id), op(operation), Operation(idTok)
{
    __TRACE_CONSTRUCT__;
    if (!op) { throw OperationBugException(__FUNCTION__ + __LINE__); }
}

void SetFunctionValue::Exec(Interpreter & interpreter)
{
    __TRACE_EXEC__;

    op->Exec(interpreter);
    
    interpreter.stack.Add(functionId, interpreter.stack.PopReturn());
}





FunctionCall::FunctionCall(std::string funcId, vector<OP_PTR> params, Token idTok)
    : id(funcId), parameters(params), Operation(idTok)
{
    __TRACE_CONSTRUCT__
}

void FunctionCall::Exec(Interpreter & interpreter)
{
    __TRACE_EXEC__;

     FUNC_PTR func = interpreter.getFunction(id);

     if (!func)
     {
         stringstream ss;
         ss << "Function with name \"" << id << "\" not defined! @Line: " << t.line;
         throw RuntimeException(ss.str().c_str());
     }

     auto targetParams = func->getParams();

     if (parameters.size() != targetParams.size())
     {
         stringstream ss;
         ss << "Function with name \"" << id << "\" called with " << parameters.size()
            << " parameters instead of " << targetParams.size() << ". @Line: " << t.line;
         throw RuntimeException(ss.str().c_str());
     }

     try
     {
         vector<VarValue> paramValues;

         for (size_t i = 0; i < parameters.size(); i++)
         {
             parameters[i]->Exec(interpreter); // evalueate param expression
             paramValues.push_back(interpreter.stack.PopReturn());
         }

         interpreter.PushStack(); // save callers stack
         interpreter.stack.Clear();
         
         for (size_t i = 0; i < parameters.size(); i++) 
         {   
             interpreter.stack.Add(targetParams[i], paramValues[i]); // add fn parameters with propper name&value
         }

         func->Exec(interpreter);

         VarValue retVal = interpreter.stack.PopReturn(); // save return value 
         interpreter.PopStack(); // restore callers stack
         interpreter.stack.PushReturn(retVal);
     }
     catch (const RuntimeException& e)
     {
         interpreter.PopStack(); // restore callers stack
         interpreter.stack.PushReturn(VarValue());
         RED("RUNTIME ERROR: " << e.what());
         _TRACE_("        RUNTIME ERROR: " << e.what() << endl);
     }
}



Variable::Variable(std::string varId, Token idTok)
    : id(varId), Operation(idTok)
{
    __TRACE_CONSTRUCT__
}

void Variable::Exec(Interpreter & interpreter)
{
    __TRACE_EXEC__;
    
    if (!interpreter.stack.Exists(id))
    {
        stringstream ss;
        ss << "Undefined variable: " << id << " @line: " << t.line;
        throw RuntimeException(ss.str().c_str());
    }

    VarValue val = interpreter.stack.GetValue(id); // find value of variable

    _TRACE_("        ID: " << id << endl);

    interpreter.stack.PushReturn(val);  /// return value
}




VariableAssignment::VariableAssignment(string varId, OP_PTR val, Token idTok)
    : id(varId), valueGetter(val), Operation(idTok)
{
    __TRACE_CONSTRUCT__;
    if (!valueGetter) { throw OperationBugException(__FUNCTION__ + __LINE__); }
}

void VariableAssignment::Exec(Interpreter & interpreter)
{
    __TRACE_EXEC__;

    _TRACE_("        ID: " << id << endl);

    valueGetter->Exec(interpreter);
    interpreter.stack.Add(id, interpreter.stack.PopReturn());
}





NOT::NOT(OP_PTR expr, Token idTok)
    : expression(expr), Operation(idTok)
{
    __TRACE_CONSTRUCT__;
    if (!expression) { throw OperationBugException(__FUNCTION__ + __LINE__); }
}

void NOT::Exec(Interpreter & interpreter)
{
    __TRACE_EXEC__;

    expression->Exec(interpreter);
    VarValue val = interpreter.stack.PopReturn();

    VarValue result;

    if (val.empty()) // if expression is false (empty vector) , we return true (non empty vector)
    {
        result.push_back(0);
    }

    interpreter.stack.PushReturn(result);
}



TRUE_VAL::TRUE_VAL(Token idTok)
    : Operation(idTok)
{
    __TRACE_CONSTRUCT__
}

void TRUE_VAL::Exec(Interpreter & interpreter)
{
    __TRACE_EXEC__

    VarValue val;

    val.push_back(0); // non empty vector means true, so we add an element

    interpreter.stack.PushReturn(val);  /// return value
}





FALSE_VAL::FALSE_VAL(Token idTok)
    : Operation(idTok)
{
    __TRACE_CONSTRUCT__
}

void FALSE_VAL::Exec(Interpreter & interpreter)
{
    __TRACE_EXEC__

    VarValue val;

    interpreter.stack.PushReturn(val);  /// return value
}





NullCheck::NullCheck(string variableId, Token idTok)
    : varId(variableId), Operation(idTok)
{
    __TRACE_CONSTRUCT__
}

void NullCheck::Exec(Interpreter & interpreter)
{
    __TRACE_EXEC__;

    if (!interpreter.stack.Exists(varId))
    {
        stringstream ss;
        ss << "Undefined variable: " << varId << " @line: " << t.line;
        throw RuntimeException(ss.str().c_str());
    }

    VarValue val = interpreter.stack.GetValue(varId);

    VarValue result;
    if (val.empty())
    {
        result.push_back(0); // return non empty => because variable value was 0
    }

    interpreter.stack.PushReturn(result);
}



PRINTS::PRINTS(std::string message, Token idTok)
    : msg(message), Operation(idTok)
{
    __TRACE_CONSTRUCT__
}

void PRINTS::Exec(Interpreter & interpreter)
{
    __TRACE_EXEC__

        if (0 == msg.compare("endl"))
        {
            cout << std::endl;
        }
        else
        {
            _TRACE_(msg << endl);

            YELLOW(msg);
        }

}



PRINT::PRINT(OP_PTR op, Token idTok)
    : operation(op), Operation(idTok)
{
    __TRACE_CONSTRUCT__;
    if (!operation) { throw OperationBugException(__FUNCTION__ + __LINE__); }
};

void PRINT::Exec(Interpreter & interpreter)
{
    __TRACE_EXEC__

        operation->Exec(interpreter);
    VarValue nodes = interpreter.stack.PopReturn();
    for (auto node : nodes)
    {
        auto attribs = interpreter.adapter->GetAttributesOf(node);
        
        if (attribs)
        {
            YELLOW(attribs->ToString());
            _TRACE_(attribs->ToString() << endl);
        }
    }
}





CONDITIONS_OF::CONDITIONS_OF(OP_PTR op, Token idTok)
    : operation(op), Operation(idTok)
{
    __TRACE_CONSTRUCT__;
    if (!operation) { throw OperationBugException(__FUNCTION__ + __LINE__); }
}

void CONDITIONS_OF::Exec(Interpreter & interpreter)
{
    __TRACE_EXEC__;

    operation->Exec(interpreter);
    VarValue val = interpreter.stack.PopReturn();

    if (val.empty())
    {
        stringstream ss;
        ss << t.tok << " called with expression which returned nothing. @Line: " << t.line;
        throw RuntimeException(ss.str().c_str());
    }

    if (0 == val[0])
    {
        stringstream ss;
        ss << "Operation " << t.tok << " parameter is NULL pointer. @Line: " << t.line;
        throw RuntimeException(ss.str().c_str());
    }

    interpreter.stack.PushReturn(interpreter.adapter->GetGeogConds(val[0]));
}



PARENT::PARENT(OP_PTR op, Token idTok)
    : operation(op), Operation(idTok)
{
    __TRACE_CONSTRUCT__;

    if (!operation) { throw OperationBugException(__FUNCTION__ + __LINE__); }
}

void PARENT::Exec(Interpreter & interpreter)
{
    __TRACE_EXEC__;

    operation->Exec(interpreter);
    VarValue val = interpreter.stack.PopReturn();

    VarValue parents;
    
    if (!val.empty())
    {
        parents = interpreter.adapter->GetParentsForNode(val[0]);
    }

    interpreter.stack.PushReturn(parents);
}



GET_NODES_OF_TYPE::GET_NODES_OF_TYPE(string nodeTypeName, Token idTok)
    : typeName(nodeTypeName), Operation(idTok)
{
    __TRACE_CONSTRUCT__
}

void GET_NODES_OF_TYPE::Exec(Interpreter & interpreter)
{
    __TRACE_EXEC__;

    if (!interpreter.adapter->Exists(typeName))
    {
        stringstream ss;
        ss << "Parameter \"" << typeName << "\" of " << t.tok << " is not a known Node Type. @Line: " << t.line;
        throw RuntimeException(ss.str().c_str());
    }

    interpreter.stack.PushReturn(interpreter.adapter->GetAllNodesOfType(typeName, 0));
}


int AttributeCheck::GetValue(string val)
{   
    int nval = 0;
    try {
        nval = stoi(val);
    }
    catch (invalid_argument&)
    {
        stringstream ss;
        ss << "Cannot compare value of " << varId << "." << attributeId << ":" << val << " with a number! @Line: " << t.line;
        throw RuntimeException(ss.str().c_str());
    }
    return nval;
}

AttributeCheck::AttributeCheck(string variableId, string attributeName, string attributeValue, CheckType chkType, Token idTok)
    : varId(variableId), attributeId(attributeName), attribVal(attributeValue), checkType(chkType), Operation(idTok)
{
    __TRACE_CONSTRUCT__
}

void AttributeCheck::Exec(Interpreter & interpreter)
{
    __TRACE_EXEC__;

    if (!interpreter.stack.Exists(varId))
    {
        stringstream ss;
        ss << "Undefined variable: " << varId << " @line: " << t.line;
        throw RuntimeException(ss.str().c_str());
    }

    VarValue val = interpreter.stack.GetValue(varId);

    if (val.empty())
    {
        stringstream ss;
        ss << "Variable named \"" << varId << "\" has empty value! @Line: " << t.line;
        throw RuntimeException(ss.str().c_str());
    }

    ATTR_PTR attributes = interpreter.adapter->GetAttributesOf(val[0]);

    if (0 == attributes)
    {
        stringstream ss;
        ss << "Variable named \"" << varId << "\" has no attributes! @Line: " << t.line;
        throw RuntimeException(ss.str().c_str());
    }

    if (!attributes->Exists(attributeId))
    {
        stringstream ss;
        ss << "Variable named \"" << varId << "\" has no attribute named \"" << attributeId << "\" ! @Line: " << t.line;
        throw RuntimeException(ss.str().c_str());
    }

    VarValue result;
    string currentVal = attributes->GetValueOf(attributeId);

    switch (checkType)
    {
    case CheckType::EQUAL:
        if (currentVal == attribVal)
        {
            result.push_back(0); 
        }
        break;
    case CheckType::STARTS_WITH:
        if (currentVal.compare(0,attribVal.length(), attribVal)==0)
        {
            result.push_back(0);
        }
        break;
    case CheckType::ENDS_WITH:
        if (currentVal.length() >= attribVal.length())
        {
            if (currentVal.substr(currentVal.length() - attribVal.length(), attribVal.length()) == attribVal)
            {
                result.push_back(0);
            }
        }
        break;
    case CheckType::CONTAINS:
        if (currentVal.find(attribVal) != string::npos)
        {
            result.push_back(0);
        }
        break;
    case CheckType::LT:
        if (GetValue(currentVal) < stoi(attribVal))
        {
            result.push_back(0);
        }
        break;
    case CheckType::GT:
        if (GetValue(currentVal) > stoi(attribVal))
        {
            result.push_back(0);
        }
        break;
    case CheckType::LE:
        if (GetValue(currentVal) <= stoi(attribVal))
        {
            result.push_back(0);
        }
        break;
    case CheckType::GE:
        if (GetValue(currentVal) >= stoi(attribVal))
        {
            result.push_back(0);
        }
        break;
    default:
        break;
    }

    interpreter.stack.PushReturn(result);
}




CompareAttribute::CompareAttribute(
    std::string variableId1, string attributeName1, string pref1, string postf1,
    std::string variableId2, string attributeName2, string pref2, string postf2,
    CheckType chType, Token idTok)
    :
    varId1(variableId1), attributeId1(attributeName1),
    varId2(variableId2), attributeId2(attributeName2),
    prefix1(pref1), postfix1(postf1),
    prefix2(pref2), postfix2(postf2),
    checkType(chType), Operation(idTok)
{
    __TRACE_CONSTRUCT__
}

int CompareAttribute::GetValueL(string val) const
{
    if (!isNumber(val))
    {
        stringstream ss;
        ss << "Value of " << varId1 << "." << attributeId1 << " \"" << val << "\" is not a number! @Line: " << t.line;
        throw RuntimeException(ss.str().c_str());
    }

    int nval = 0;
    try {
        nval = stoi(val);
        _TRACE_("        svalL = \"" << val << "\", nvalL = " << nval << endl);
    }
    catch (invalid_argument&)
    {
        stringstream ss;
        ss << "Value of " << varId1 << "." << attributeId1 << " \"" << val << "\" is not a number! @Line: " << t.line;
        throw RuntimeException(ss.str().c_str());
    }
    return nval;
}

int CompareAttribute::GetValueR(string val) const
{   
    if (!isNumber(val))
    {
        stringstream ss;
        ss << "Value of " << varId2 << "." << attributeId2 << " \"" << val << "\" is not a number! @Line: " << t.line;
        throw RuntimeException(ss.str().c_str());
    }

    int nval = 0;
    try {
        nval = stoi(val);
        _TRACE_("        svalR = \"" << val << "\", nvalR = " << nval << endl);
    }
    catch (invalid_argument&)
    {
        stringstream ss;
        ss << "Value of " << varId2 << "." << attributeId2 << " \"" << val << "\" is not a number! @Line: " << t.line;
        throw RuntimeException(ss.str().c_str());
    }
    return nval;
}

void CompareAttribute::Exec(Interpreter & interpreter)
{
    __TRACE_EXEC__;


    if (!interpreter.stack.Exists(varId1))
    {
        stringstream ss;
        ss << "Unknown variable " << varId1 << " @Line:" << t.line;
        throw RuntimeException(ss.str().c_str());
    }
    if (!interpreter.stack.Exists(varId2))
    {
        stringstream ss;
        ss << "Unknown variable " << varId2 << " @Line:" << t.line;
        throw RuntimeException(ss.str().c_str());
    }

    VarValue val1 = interpreter.stack.GetValue(varId1);
    VarValue val2 = interpreter.stack.GetValue(varId2);

    if (val1.size() != 1)
    {
        stringstream ss;
        ss << "Variable \"" << varId1 << "\" has no value! @Line: " << t.line;
        throw RuntimeException(ss.str().c_str());

    }
    if (val2.size() != 1)
    {
        stringstream ss;
        ss << "Variable \"" << varId2 << "\" has no value! @Line: " << t.line;
        throw RuntimeException(ss.str().c_str());
    }



    ATTR_PTR attributes1 = interpreter.adapter->GetAttributesOf(val1[0]);
    ATTR_PTR attributes2 = interpreter.adapter->GetAttributesOf(val2[0]);

    if (!attributes1)
    {
        stringstream ss;
        ss << "Variable named \"" << varId1 << "\" has no attributes! @Line: " << t.line;
        throw RuntimeException(ss.str().c_str());
    }
    if (!attributes2)
    {
        stringstream ss;
        ss << "Variable named \"" << varId2 << "\" has no attributes! @Line: " << t.line;
        throw RuntimeException(ss.str().c_str());
    }



    if (!attributes1->Exists(attributeId1))
    {
        stringstream ss;
        ss << "Variable named \"" << varId1 << "\" has no attribute named \"" << attributeId1 << "\" ! @Line: " << t.line;
        throw RuntimeException(ss.str().c_str());
    }
    if (!attributes2->Exists(attributeId2))
    {
        stringstream ss;
        ss << "Variable named \"" << varId2 << "\" has no attribute named \"" << attributeId2 << "\" ! @Line: " << t.line;
        throw RuntimeException(ss.str().c_str());
    }

    string valAttr1 = prefix1 + attributes1->GetValueOf(attributeId1) + postfix1;
    string valAttr2 = prefix2 + attributes2->GetValueOf(attributeId2) + postfix2;
    _TRACE_("        prefix1 = \"" << prefix1 << "\", postfix1 = \"" << postfix1 << "\"" << endl);
    _TRACE_("        prefix2 = \"" << prefix2 << "\", postfix2 = \"" << postfix2 << "\"" << endl);
    _TRACE_("        valAttr1 = \"" << valAttr1 << "\", valAttr2 = \"" << valAttr2 << "\"" << endl );
    
    VarValue result;
   
    switch (checkType)
    {
    case CheckType::EQUAL:

        if (valAttr1 == valAttr2)
        {
            result.push_back(0); // dummy element to signal TRUE ... empty vector means FALSE
        }

        break;
    case CheckType::LT:
        if (GetValueL(valAttr1) < GetValueR(valAttr2))
        {
            result.push_back(0);
        }
        break;
    case CheckType::GT:
        if (GetValueL(valAttr1) > GetValueR(valAttr2))
        {
            result.push_back(0);
        }
        break;
    case CheckType::LE:
        if (GetValueL(valAttr1) <= GetValueR(valAttr2))
        {
            result.push_back(0);
        }
        break;
    case CheckType::GE:
        if (GetValueL(valAttr1) >= GetValueR(valAttr2))
        {
            result.push_back(0);
        }
        break;
    default:
        break;
    }

    interpreter.stack.PushReturn(result);
}