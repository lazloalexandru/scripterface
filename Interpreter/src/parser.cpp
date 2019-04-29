#include "parser.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <memory>
#include <regex>
#include <iterator>
#include "operations.h"
#include "trace.h"
#include "utils.h"
#include "parser_exceptions.h"

using namespace std;

#define INDENT "        "
#define __TRACE_PARSING__ _TRACE_( INDENT << "PARSING > " << __FUNCTION__ << endl);

LangParser::LangParser()
{
    __TRACE_PARSING__;
    
   _TRACE_( endl << "LANGUAGE KEYWORDS" << endl << "=================" << endl);
   _TRACE_( symFOR << endl);
   _TRACE_( symIN << endl);
   _TRACE_( symWHILE << endl);
   _TRACE_( symIF << endl);
   _TRACE_( symTHEN << endl);
   _TRACE_( symELSE << endl);
   _TRACE_( symEND << endl);
   _TRACE_( symNOT << endl);
   _TRACE_( symOR << endl);
   _TRACE_( symAND << endl);
   _TRACE_( symFALSE << endl);
   _TRACE_( symTRUE << endl);
   _TRACE_( symNULL << endl);
   _TRACE_(symENDL << endl);
   
   _TRACE_( endl << "VARIABLE PREFIX" << endl << "====================" << endl);

   _TRACE_( endl << "BOOL VARIABLE PREFIX" << endl << "====================" << endl);
   _TRACE_( symHASH << endl);

   _TRACE_( endl << "USER DEFINED / NONSPECIFIC KEYWORDS" << endl << "===================================" << endl);
   _TRACE_( symGET_NODES_OF_TYPE << endl);
   _TRACE_( symCONDITIONS_OF << endl);
   _TRACE_( symPARENT << endl);
   _TRACE_( symPRINT << endl);
   _TRACE_( symPRINTS << endl);
   _TRACE_( symSTARTS_WITH << endl);
   _TRACE_( symENDS_WITH << endl);
   _TRACE_( symCONTAINS << endl);
   
   _TRACE_( endl << "DELIMITERS" << endl << "==========" << endl);
   delimiters.push_back(symHASH);
   delimiters.push_back(symAT);
   delimiters.push_back(symCOLON);
   delimiters.push_back(symSEMICOLON);
   delimiters.push_back(symEQUAL);
   delimiters.push_back(symLOWER);
   delimiters.push_back(symGREATER);
   delimiters.push_back(symPLUS);
   delimiters.push_back(symDOT);
   delimiters.push_back(symCOMMA);
   delimiters.push_back(symQUOTE);
   delimiters.push_back(symROUNDOPEN);
   delimiters.push_back(symROUNDCLOSE);
   delimiters.push_back(symSPACE);
   for (auto delimiter:delimiters)
   {
       _TRACE_( delimiter << endl);
   }
   
   _TRACE_( endl << endl);
}

#define CALLFN(object,ptr)  ((object).*(ptr))

OP_PTR LangParser::Try(OpParserFunc func)
{
    OP_PTR result;

    tm.SaveCurrent();

    try
    {
        result = CALLFN(*this, func)(); // execute the parser function
    }
    catch (const ParseException&)
    {
        tm.RestoreCurrent(); // do nothing, we only tried to parse
    }

    return result;
}


vector<FUNC_PTR> LangParser::parse(string path)
{
    __TRACE_PARSING__;

    vector<FUNC_PTR> functions;
    
    cout << path;

    try
    {
        tm.SetTokens( Tokenizer::getTokensFromFile(path, delimiters) );

        OP_PTR op = Parse_FunctionDefinition();
        while (op)
        {
            functions.push_back(dynamic_pointer_cast<Function>(op));

            if (tm.NoMoreTokens()) { op = 0; }
            else                   { op = Parse_FunctionDefinition(); }
        }

        cout << "OK";
    }
    catch (const invalid_argument& e)
    {
        RED(endl << "PARSER ERROR: " << e.what() << endl);        
        _TRACE_("        PARSER ERROR: " << e.what() << endl);
    }
    catch (const ParseException& e)
    {
        RED(endl << "PARSER ERROR (file: " << path << ") " << e.what() << endl);
        _TRACE_("        PARSER ERROR (file: " << path << ") " << e.what() << endl);
    }
    catch (const BadSyntaxException& e)
    {
        RED(endl << "PARSER ERROR (file: " << path << ") " << e.what() << endl);
        _TRACE_("        PARSER ERROR (file: " << path << ") " << e.what() << endl);
    }

    return functions;
}


bool LangParser::IS_SYM(string sym, bool skipSpaces)
{
    Token t = tm.Current(skipSpaces);

    _TRACE_( INDENT << "IS_SYM checking: " << sym << " found: " << t.tok << " @Line: " << t.line << endl);

    return t.tok.compare(sym) == 0;
}


void LangParser::SYM(string sym)
{   
    Token t = tm.GetNext();

    if (t.tok.compare(sym) != 0)
    {
        stringstream ss;
        ss << "@line: " << t.line << " - Expected: " << sym << " found: " << t.tok;
        throw ParseException(ss.str().c_str());
    }

    _TRACE_("" << sym << " >> line " << t.line << " GET_SYM" << endl);
}


int LangParser::NUM()
{
    Token t = tm.GetNext();

    if (!isNumber(t.tok))
    {
        stringstream ss;
        ss << "@line: " << t.line << " -  Number expected, found: " << t.tok;
        throw ParseException(ss.str().c_str());
    }

    int n = stoi(t.tok);
    _TRACE_( n << " >> line " << t.line << " GET_NUM" << endl);
    return n;
}

string LangParser::ANY()
{
    Token t = tm.GetNext(false);

    _TRACE_( t.tok << " >> line " << t.line << " GET_ANY" << endl);

    return t.tok;
}

string LangParser::TXT()
{
    Token t = tm.GetNext();

    if (!isAlphaNumeric(t.tok))
    {
        stringstream ss;
        ss << "@line: " << t.line << " - Alpha-numeric expected, found: " << t.tok;
        throw ParseException(ss.str().c_str());
    }

    _TRACE_( t.tok << " >> line " << t.line << " GET_TXT" << endl);
    return t.tok;
}

vector<string> LangParser::Parse_FnDefinitionParameters()
{
    vector<string> params;

    if (IS_SYM(symROUNDOPEN))
    {
        SYM(symROUNDOPEN);

        params.push_back(TXT());
        while (IS_SYM(symCOMMA)) 
        {
            SYM(symCOMMA);
            params.push_back(TXT());
        }

        SYM(symROUNDCLOSE);
    }

    return params;
}

OP_PTR LangParser::Parse_FunctionDefinition()
{
    __TRACE_PARSING__;

    SYM(symAT);
    Token idTok = tm.Current();
    string ruleId = TXT();
    auto params = Parse_FnDefinitionParameters();
    SYM(symCOLON);
    vector<OP_PTR> operations = Parse_EmbeddedOperations();

    return make_shared<Function>(ruleId, operations, params, idTok);
}


vector<OP_PTR> LangParser::Parse_FnCallParameters()
{
    vector<OP_PTR> params;

    if (IS_SYM(symROUNDOPEN))
    {
        SYM(symROUNDOPEN);

        params.push_back(Parse_ValueExpression());
        while (IS_SYM(symCOMMA))
        {
            SYM(symCOMMA);
            params.push_back(Parse_ValueExpression());
        }

        SYM(symROUNDCLOSE);
    }

    return params;
}


OP_PTR LangParser::Parse_FunctionCall()
{
    __TRACE_PARSING__;

    SYM(symAT);
    Token idTok = tm.Current();
    string functionId = TXT();
    auto params = Parse_FnCallParameters();

    return make_shared<FunctionCall>(functionId, params, idTok);
}

OP_PTR LangParser::Parse_FunctionSetReturnValue()
{
    __TRACE_PARSING__;

    SYM(symAT);
    Token idTok = tm.Current();
    string functionId = TXT();
    SYM(symEQUAL);
    auto op = Parse_ParalelBlocks();

    return make_shared<SetFunctionValue>(functionId, op, idTok);
}

OP_PTR LangParser::Parse_Variable()
{
    __TRACE_PARSING__;

    SYM(symHASH);    
    Token idTok = tm.Current();
    string id = TXT();

    return make_shared<Variable>(id, idTok);
}

OP_PTR LangParser::Parse_NOT()
{
    __TRACE_PARSING__;

    Token idTok = tm.Current();
    SYM(symNOT);
    SYM(symROUNDOPEN);
    OP_PTR expression = Parse_ParalelBlocks();
    SYM(symROUNDCLOSE);
    
    return make_shared<NOT>(expression, idTok);
}

OP_PTR LangParser::Parse_BoolValue()
{
    __TRACE_PARSING__;

    OP_PTR      op = Try(&LangParser::Parse_TRUE);
    if (!op)    op = Parse_FALSE();
    if (!op)
    {
        stringstream ss;
        ss << "@line: " << tm.Current().line
            << " - Expected: < TRUE | FALSE > found: " << tm.Current().tok;
        throw BadSyntaxException(ss.str().c_str());
    }

    return op;
}

OP_PTR LangParser::Parse_VariableAssignment()
{
    __TRACE_PARSING__;

    SYM(symHASH);
    Token idTok = tm.Current();
    string variableName = TXT();
    SYM(symEQUAL);
    OP_PTR op = Parse_SerialBlock();
    if (!op)
    {
        stringstream ss;
        ss << "@line: " << tm.Current().line
            << " - Expected: < ValueExpression > found: "
            << tm.Current().tok;
        throw BadSyntaxException(ss.str().c_str());
    }
    return make_shared<VariableAssignment>(variableName, op, idTok);
}


OP_PTR LangParser::Parse_TRUE()
{
    __TRACE_PARSING__;

    Token idTok = tm.Current();
    SYM(symTRUE);

    return make_shared<TRUE_VAL>(idTok);
}

OP_PTR LangParser::Parse_FALSE()
{
    __TRACE_PARSING__;

    Token idTok = tm.Current();
    SYM(symFALSE);

    return make_shared<FALSE_VAL>(idTok);
}

OP_PTR LangParser::Parse_Parent() // its a try => valid is not out param!
{
    __TRACE_PARSING__;

    Token idTok = tm.Current();
    SYM(symPARENT);
    SYM(symROUNDOPEN);
    OP_PTR op = Parse_SerialBlock();
    SYM(symROUNDCLOSE);

    return make_shared<PARENT>(op, idTok);
}


OP_PTR LangParser::Parse_GET_NODES_OF_TYPE()
{
    __TRACE_PARSING__;

    Token idTok = tm.Current();
    SYM(symGET_NODES_OF_TYPE);
    SYM(symROUNDOPEN);
    string nodeKindEnumName = TXT();
    SYM(symROUNDCLOSE);

    return make_shared<GET_NODES_OF_TYPE>(nodeKindEnumName, idTok);
}


OP_PTR LangParser::Parse_CONDITIONS_OF()
{
    __TRACE_PARSING__;

    Token idTok = tm.Current();
    SYM(symCONDITIONS_OF);
    SYM(symROUNDOPEN);
    auto nodeExpr = Parse_SerialBlock();
    SYM(symROUNDCLOSE);

    return make_shared<CONDITIONS_OF>(nodeExpr, idTok);
}


OP_PTR LangParser::Parse_ForLoop()
{
    __TRACE_PARSING__;

    OP_PTR op;

    Token idTok = tm.Current();
    SYM(symFOR);
    SYM(symHASH);
    string loopVariableName = TXT();
    SYM(symIN);
    auto data_getter = Parse_ValueExpression();
    auto operations  = Parse_EmbeddedOperations();

    return make_shared<ForLoop>(loopVariableName, data_getter, operations, idTok);
}


OP_PTR LangParser::Parse_ValueExpression()
{
    __TRACE_PARSING__;

    OP_PTR op;

    if (IS_SYM(symROUNDOPEN))
    {
        SYM(symROUNDOPEN);
        op = Parse_ParalelBlocks();
        SYM(symROUNDCLOSE);
    }
    else
    {
        if (!op)    op = Try(&LangParser::Parse_CompareAttribute);
        if (!op)    op = Try(&LangParser::Parse_AttributeCheck);
        if (!op)    op = Try(&LangParser::Parse_NullCheck);
        if (!op)    op = Try(&LangParser::Parse_Variable);
        if (!op)    op = Try(&LangParser::Parse_Parent); 
        if (!op)    op = Try(&LangParser::Parse_NOT);
        if (!op)    op = Try(&LangParser::Parse_TRUE);
        if (!op)    op = Try(&LangParser::Parse_FALSE);
        if (!op)    op = Try(&LangParser::Parse_FunctionCall);
        if (!op)    op = Try(&LangParser::Parse_CONDITIONS_OF);
        if (!op)    op = Try(&LangParser::Parse_GET_NODES_OF_TYPE);
        if (!op)
        {
            stringstream ss;
            ss << "@line: " << tm.Current().line
                << " - Expected: < Variable | AttributeCheck | CompareAttribute | NullCheck | NOT | TRUE | FALSE | FunctionCall | CONDITIONS_OF | GET_NODES_OF_TYPE > found: "
                << tm.Current().tok;
            throw BadSyntaxException(ss.str().c_str());
        }
    }

    return op;
}


OP_PTR LangParser::Parse_SerialBlock() // parses operations glued by AND keyword
{
    __TRACE_PARSING__;

    Token idTok = tm.Current();

    vector<OP_PTR> sBlocks;
    
    sBlocks.push_back(Parse_ValueExpression());
    while (IS_SYM(symAND))
    {
        SYM(symAND);
        sBlocks.push_back(Parse_ValueExpression());
    }

    return make_shared<ConditionalBlock>(sBlocks, vector<OP_PTR>(), idTok);
}


OP_PTR LangParser::Parse_ParalelBlocks()
{
    __TRACE_PARSING__;

    Token idTok = tm.Current();

    vector<OP_PTR> pBlocks;
    
    pBlocks.push_back(Parse_SerialBlock());
    while (IS_SYM(symOR))
    {
        SYM(symOR);
        pBlocks.push_back(Parse_SerialBlock());
    }

    return make_shared<ConditionalBlock>(vector<OP_PTR>(), pBlocks, idTok);
}


OP_PTR LangParser::Parse_IfStmt()
{
    __TRACE_PARSING__;

    Token idTok = tm.Current();
    SYM(symIF);
    OP_PTR condition = Parse_ParalelBlocks();
    auto thanOperations = Parse_ThanOperations();
    auto elseOperations = Parse_ElseOperations();

    return make_shared<IfStmt>(condition, thanOperations, elseOperations, idTok);
}


OP_PTR LangParser::Parse_WhileLoop()
{
    __TRACE_PARSING__;

    Token idTok = tm.Current();
    SYM(symWHILE);
    OP_PTR condition = Parse_ParalelBlocks();
    auto operations = Parse_EmbeddedOperations();

    return make_shared<WhileLoop>(condition, operations, idTok);
}



OP_PTR LangParser::Parse_PRINT()
{
    __TRACE_PARSING__;

    Token idTok = tm.Current();
    SYM(symPRINT);
    SYM(symROUNDOPEN);
    OP_PTR nodeExpr = Parse_SerialBlock();
    SYM(symROUNDCLOSE);

    return make_shared<PRINT>(nodeExpr, idTok);
}

string LangParser::QUOTED_TEXT()
{
    string text;

    SYM(symQUOTE);
    
    while (!IS_SYM(symQUOTE, false))
    {
        text = text + ANY();
    }

    SYM(symQUOTE);

    return text;
}

OP_PTR LangParser::Parse_PRINTS()
{
    __TRACE_PARSING__;

    OP_PTR op;

    Token idTok = tm.Current();
    SYM(symPRINTS);
    SYM(symROUNDOPEN);

    string text;
    string space = "";

    if (IS_SYM(symENDL)) { SYM(symENDL); text += "\n"; }
    else { text += QUOTED_TEXT(); space = " "; }
    
    while (IS_SYM(symPLUS))
    {
        SYM(symPLUS);

        if (IS_SYM(symENDL)) { SYM(symENDL); text += "\n"; space = ""; }
        else { text += space + QUOTED_TEXT(); space = " "; }
    } 

    SYM(symROUNDCLOSE);

    return make_shared<PRINTS>(text, idTok);
}


OP_PTR LangParser::Parse_Operation()
{
    __TRACE_PARSING__;

    OP_PTR      op = Try(&LangParser::Parse_ForLoop);
    if (!op)	op = Try(&LangParser::Parse_IfStmt);
    if (!op)	op = Try(&LangParser::Parse_PRINT);
    if (!op)	op = Try(&LangParser::Parse_PRINTS);
    if (!op)	op = Try(&LangParser::Parse_FunctionSetReturnValue);
    if (!op)	op = Try(&LangParser::Parse_FunctionCall);
    if (!op)	op = Try(&LangParser::Parse_VariableAssignment);
    if (!op)	op = Try(&LangParser::Parse_WhileLoop);
    
    if (!op)
    {
        stringstream ss;
        ss << "@line: " << tm.Current().line
            << " - Expected: < ForLoop | IfStmt | PRINT | PRINTS | FunctionCall | VariableAssignment | WhileLoop > found: "
            << tm.Current().tok;
        throw BadSyntaxException(ss.str().c_str());
    }

    return op;
}


vector<OP_PTR> LangParser::Parse_EmbeddedOperations()
{
    __TRACE_PARSING__;

    vector<OP_PTR> operations;

    while (!IS_SYM(symEND))
    {
        operations.push_back(LangParser::Parse_Operation());
    }

    SYM(symEND);

    return operations;
}


vector<OP_PTR> LangParser::Parse_ThanOperations()
{
    __TRACE_PARSING__;

    SYM(symTHEN);

    vector<OP_PTR> operations;
    while (!IS_SYM(symELSE) && !IS_SYM(symEND))
    {
        operations.push_back(LangParser::Parse_Operation());
    }

    return operations;
}

vector<OP_PTR> LangParser::Parse_ElseOperations()
{
    __TRACE_PARSING__;

    vector<OP_PTR> operations;
    
    if (IS_SYM(symELSE))
    {
        SYM(symELSE);

        while (!IS_SYM(symEND))
        {
            operations.push_back(LangParser::Parse_Operation());
        }
    }

    SYM(symEND);

    return operations;
}

OP_PTR LangParser::Parse_NullCheck()
{
    __TRACE_PARSING__;

    OP_PTR op;

    Token idTok = tm.Current();
    SYM(symHASH);
    string variableId = TXT();
    SYM(symEQUAL);
    SYM(symEQUAL);
    SYM(symNULL);

    return make_shared<NullCheck>(variableId, idTok);
}

OP_PTR LangParser::Parse_AttributeCheck()
{
    __TRACE_PARSING__;

    OP_PTR op;

    Token idTok = tm.Current();
    SYM(symHASH);
    string variableId = TXT();
    SYM(symDOT);
    string attributeName = TXT();
    
    string attributeValue;
    CheckType checkType;

    if (IS_SYM(symEQUAL))
    {   
        SYM(symEQUAL);
        SYM(symEQUAL);
     
        checkType = CheckType::EQUAL;
        
        attributeValue = QUOTED_TEXT();
    }
    else if (IS_SYM(symLOWER))
    {
        SYM(symLOWER);
        if (IS_SYM(symEQUAL))
        {   
            SYM(symEQUAL);
            
            checkType = CheckType::LE;
        }
        else
        {
            checkType = CheckType::LT;
        }

        attributeValue = to_string(NUM());
    }
    else if (IS_SYM(symGREATER))
    {
        SYM(symGREATER);
        if (IS_SYM(symEQUAL))
        {
            SYM(symEQUAL);

            checkType = CheckType::GE;
        }
        else
        {
            checkType = CheckType::GT;
        }

        attributeValue = to_string(NUM());
    }
    else
    {
        SYM(symDOT);
        if (IS_SYM(symSTARTS_WITH))
        {
            checkType = CheckType::STARTS_WITH;
            SYM(symSTARTS_WITH);
        }
        else if (IS_SYM(symENDS_WITH))
        {
            checkType = CheckType::ENDS_WITH;
            SYM(symENDS_WITH);
        }
        else
        {
            checkType = CheckType::CONTAINS;
            SYM(symCONTAINS);
        }
        SYM(symROUNDOPEN);
        attributeValue = QUOTED_TEXT();
        SYM(symROUNDCLOSE);
    }

    return make_shared<AttributeCheck>(variableId, attributeName, attributeValue, checkType, idTok);
}


OP_PTR LangParser::Parse_CompareAttribute()
{
    __TRACE_PARSING__;

    Token idTok = tm.Current();
    string prefix1, postfix1;
    string prefix2, postfix2;
    CheckType checkType;
    
    // FISRST OPERAND
    
    if (IS_SYM(symQUOTE))
    {
        prefix1 = QUOTED_TEXT();
        SYM(symPLUS);
    }
    SYM(symHASH);
    string variableId1 = TXT();
    SYM(symDOT);
    string attributeName1 = TXT();
    if (IS_SYM(symPLUS))
    {
        SYM(symPLUS);
        postfix1 = QUOTED_TEXT();
    }

    
    if (IS_SYM(symEQUAL))
    {
        SYM(symEQUAL);
        SYM(symEQUAL);

        checkType = CheckType::EQUAL;
    }
    else if (IS_SYM(symLOWER))
    {
        SYM(symLOWER);
        if (IS_SYM(symEQUAL))
        {
            SYM(symEQUAL);

            checkType = CheckType::LE;
        }
        else
        {
            checkType = CheckType::LT;
        }
    }
    else if (IS_SYM(symGREATER))
    {
        SYM(symGREATER);
        if (IS_SYM(symEQUAL))
        {
            SYM(symEQUAL);

            checkType = CheckType::GE;
        }
        else
        {
            checkType = CheckType::GT;
        }
    }

    // SECOND OPERAND
    
    if (IS_SYM(symQUOTE))
    {
        prefix2 = QUOTED_TEXT();
        SYM(symPLUS);
    }
    SYM(symHASH);
    string variableId2 = TXT();
    SYM(symDOT);
    string attributeName2 = TXT();
    if (IS_SYM(symPLUS))
    {
        SYM(symPLUS);
        postfix2 = QUOTED_TEXT();
    }
       
    _TRACE_("        prefix1 = " << prefix1 << " postfix1 = " << postfix1 << endl);
    _TRACE_("        prefix2 = " << prefix2 << " postfix2 = " << postfix2 << endl);
    auto p = new CompareAttribute( // must use new ... shared_ptr doesn_t work with 9 parameters
        variableId1, attributeName1, prefix1, postfix1,
        variableId2, attributeName2, prefix2, postfix2,
        checkType, idTok);
    shared_ptr<CompareAttribute> res(p);

    return res;
}