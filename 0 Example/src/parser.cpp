#include "tokens.hpp"
#include <iostream>
#include <string>

Token tok;

Token next_tok();
std::string get_str();
void error(const char* msg);

void nexttok()
{
    tok = next_tok();
}

bool accept(Token t)
{
    if(tok == t)
    {
        nexttok();
        return true;
    }
    return false;
}

bool expect(Token t)
{
    if(accept(t)) return true;

    error("unexpected token");
    return false;
}

void expression();

void factor()
{
    if(accept(IDENTIFIER))
    {
        std::cout << " " << get_str() << " ";
    }
    else if(accept(NUMBER))
    {
        std::cout << " " << get_str() << " ";
    }
    else if(accept(OPEN))
    {
        std::cout << " ( ";
        expression();
        expect(CLOSE);
        std::cout << " ) ";
    }
    else
    {
        error("syntax error");
        nexttok();
    }
}

void term()
{
    factor();
    while(tok == MUL || tok == DIV)
    {
        if(tok == MUL) std::cout << " * ";
        else std::cout << " / ";
        nexttok();
        factor();
    }
}

void expression()
{
    if(tok == ADD || tok == SUB)
    {
        if(tok == ADD) std::cout << " + ";
        else std::cout << " - ";
        nexttok();
    }
    term();
    while(tok == ADD || tok == SUB)
    {
        if(tok == ADD) std::cout << " + ";
        else std::cout << " - ";
        nexttok();
        term();
    }
}

void parse()
{
    nexttok();
    expression();
    expect(T_EOF);
}