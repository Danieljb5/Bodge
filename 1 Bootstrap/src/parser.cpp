#include "tokens.hpp"
#include <string>
#include <iostream>

Token lex();
std::string get_id();
void error(const std::string& msg);

Token tok;
std::string strOut = "";

void next()
{
    tok = lex();
    // std::cout << "[" << TokenNames[tok] << "]";
}

bool accept(Token t)
{
    if(tok == t)
    {
        next();
        return true;
    }
    return false;
}

bool expect(Token t)
{
    if(accept(t)) return true;

    error("unexpected " + TokenNames[tok] + ", expected " + TokenNames[t]);
    return false;
}

// TODO: finish arithmetic expressions (not all needed for bootstrap compiler)

std::string arithexpr();

std::string value()
{
    if(accept(IDENTIFIER))
    {
        return get_id();
    }
    else if(accept(NUMBER))
    {
        return get_id();
    }
    else if(accept(OPEN))
    {
        std::string result = "(";
        result += arithexpr();
        expect(CLOSE);
        result += ")";
        return result;
    }
    else
    {
        error("syntax error: expecting lvalue, rvalue or nested expression");
        return "";
    }
}

std::string addsub()
{
    std::string result = "";
    if(tok == ADD || tok == SUB)
    {
        if(tok == ADD) result = "+";
        if(tok == SUB) result = "-";
        next();
    }
    result += value();
    while(tok == ADD || tok == SUB)
    {
        if(tok == ADD) result += "+";
        if(tok == SUB) result += "-";
        next();
        result += value();
    }
    return result;
}

std::string muldiv()
{
    std::string result = addsub();
    while(tok == MUL || tok == DIV)
    {
        if(tok == MUL) result += "*";
        if(tok == DIV) result += "/";
        next();
        result += addsub();
    }
    return result;
}

std::string arithexpr()
{
    return muldiv();
}

std::string typemods()
{
    std::string result = "";
    if(accept(MUL))
    {
        result += "*";
    }
    else if(accept(OPENSQ))
    {
        expect(CLOSESQ);
        result += "[]";
    }
    else
    {
        return result;
    }
    result += typemods();
}

std::string args(bool make_constref = true)
{
    std::string result = "";
    if(accept(IDENTIFIER))
    {
        auto type = get_id();
        type += typemods();
        if(make_constref) result += "const " + type + "& ";
        else result += type + " ";
        expect(IDENTIFIER);
        result += get_id();
    }
    if(accept(COMMA)) result += ", " + args(make_constref);
    else if(accept(CLOSE)) return result;
    else
    {
        error("unexpected " + TokenNames[tok]);
        return result;
    }
}

void block()
{
    if(accept(IDENTIFIER))
    {
        std::string strIdent = get_id();
        if(accept(OPEN))
        {   // function prototype
            bool mainF = (strIdent == "main");
            std::string args_ = args(!mainF);
            std::string retType = "void";
            if(mainF) retType = "int";
            if(accept(COLON))
            {   // function return type
                expect(IDENTIFIER);
                retType = get_id();
            }

            strOut += retType + " " + strIdent + "(" + args_ + ")";

            if(accept(OPENB))
            {   // function body
                strOut += "\n{\n";
                // TODO: scope
                expect(CLOSEB);
                strOut += "}\n";
            }
            else
            {
                expect(SEMICOLON);
                strOut += ";";
            }
            strOut += "\n";
        }
        else if(accept(IDENTIFIER))
        {   // variable declaration
            strOut += strIdent + " " + get_id();

            if(accept(ASSIGN))
            {
                strOut += " = ";
                if(accept(STRING))
                {
                    strOut += get_id();
                }
                else
                {
                    strOut += arithexpr();
                }
            }

            expect(SEMICOLON);
            strOut += ";\n";
        }
        else
        {
            error("syntax error, unexpected " + TokenNames[tok]);
            return;
        }
    }
    else if(accept(T_EOF))
    {
        return;
    }
    block();
}

std::string parse()
{
    next();
    block();
    return strOut;
}