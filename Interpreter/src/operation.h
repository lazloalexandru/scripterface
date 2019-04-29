#pragma once

#include <memory>
#include "token.h"
#include "variable_set.h"

class Interpreter;
struct Token;

class Operation
{
public:

    virtual void Exec(Interpreter & interpreter) = 0;

protected:

    Operation(Token idTok);

public:

    Token t;
};

typedef std::shared_ptr<Operation> OP_PTR;

#define __TRACE_EXEC__ _TRACE_("        RUN   >>>  " __FUNCTION__ << "@" << t.line << interpreter.stack.ToStr(interpreter) << endl);
#define __TRACE_CONSTRUCT__ _TRACE_("        NEW   >>>  " << __FUNCTION__ << endl);
#define ADAPTER (*Interpreter::Instance().GetAdapter())
