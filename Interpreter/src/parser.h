#pragma once

#include <string>
#include <vector>
#include "operations.h"
#include "token_manager.h"
#include "../include/script_interpreter.h"


//SYMBOLS
const std::string symFOR("FOR");
const std::string symIN("IN");
const std::string symWHILE("WHILE");
const std::string symIF("IF");
const std::string symTHEN("THEN");
const std::string symELSE("ELSE");
const std::string symEND("END");
const std::string symOR("OR");
const std::string symAND("AND");
const std::string symNOT("NOT");
const std::string symGET_NODES_OF_TYPE("GET_NODES_OF_TYPE");
const std::string symCONDITIONS_OF("CONDITIONS_OF");
const std::string symPARENT("PARENT");
const std::string symPRINT("PRINT");
const std::string symPRINTS("PRINTS");
const std::string symENDL("endl");
const std::string symFALSE("FALSE");
const std::string symTRUE("TRUE");
const std::string symNULL("NULL");
const std::string symSTARTS_WITH("StartsWith");
const std::string symENDS_WITH("EndsWith");
const std::string symCONTAINS("Contains");
// DELIMITERS
const std::string symHASH("#");
const std::string symAT("@");
const std::string symCOLON(":");
const std::string symSEMICOLON(";");
const std::string symEQUAL("=");
const std::string symLOWER("<");
const std::string symGREATER(">");
const std::string symPLUS("+");
const std::string symDOT(".");
const std::string symCOMMA(",");
const std::string symQUOTE("\"");
const std::string symROUNDOPEN("(");
const std::string symROUNDCLOSE(")");
const std::string symSPACE(" ");

class LangParser
{
    typedef OP_PTR(LangParser::*OpParserFunc)();   // Function pointer

public:
    LangParser();
    
    std::vector<FUNC_PTR> parse(std::string ruleFilePath);

private:

    OP_PTR Try(OpParserFunc func);
    
    bool        IS_SYM(std::string sym, bool skipSpaces = true);
    void        SYM(std::string sym);
    int         NUM();
    std::string TXT();
    std::string ANY();
    std::string QUOTED_TEXT();

private:
    OP_PTR Parse_FunctionDefinition();
    OP_PTR Parse_FunctionSetReturnValue();
    std::vector<OP_PTR> Parse_EmbeddedOperations();

    std::vector<std::string> Parse_FnDefinitionParameters();
    std::vector<OP_PTR> Parse_FnCallParameters();
    
    OP_PTR Parse_Operation();
    OP_PTR Parse_ForLoop();
    OP_PTR Parse_WhileLoop();
    
    
    OP_PTR Parse_IfStmt();
    std::vector<OP_PTR> Parse_ThanOperations();
    std::vector<OP_PTR> Parse_ElseOperations();
    
    OP_PTR Parse_VariableAssignment();

    /// ValueExpressions ////////////////////////////
    OP_PTR Parse_FunctionCall();
    OP_PTR Parse_Variable();
    OP_PTR Parse_ParalelBlocks(); // group of condition blocks grlued with OR
    OP_PTR Parse_SerialBlock();   // group of condition blocks grlued with AND
    OP_PTR Parse_NOT();
    OP_PTR Parse_NullCheck();
    OP_PTR Parse_AttributeCheck();
    OP_PTR Parse_CompareAttribute();
    OP_PTR Parse_TRUE();
    OP_PTR Parse_FALSE();
    OP_PTR Parse_ValueExpression();
    OP_PTR Parse_Parent();
    OP_PTR Parse_BoolValue();
    /////////////////////////////////////////////////////
    
    OP_PTR Parse_GET_NODES_OF_TYPE();
    OP_PTR Parse_CONDITIONS_OF();
    OP_PTR Parse_PRINT();    
    OP_PTR Parse_PRINTS();
    
private:
    std::vector<std::string> delimiters;
    TokenManager tm;
};
