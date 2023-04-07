#include "tokens.hpp"
#include <iostream>
#include <string>

std::string src = 
"abc+ 1*3.14159 /(7/b12) * -1";
size_t pos = 0;
std::string identifier = "invalid";
Token curr;

void error(const char* msg);

bool op(char* c)
{
    if(*c == '+')
    {
        curr = ADD;
        return true;
    }
    else if(*c == '-')
    {
        curr = SUB;
        return true;
    }
    else if(*c == '*')
    {
        curr = MUL;
        return true;
    }
    else if(*c == '/')
    {
        curr = DIV;
        return true;
    }
    return false;
}

bool brackets(char* c)
{
    if(*c == '(')
    {
        curr = OPEN;
        return true;
    }
    else if(*c == ')')
    {
        curr = CLOSE;
        return true;
    }
    return false;
}

bool number(char* c)
{
    if(*c == '+' || *c == '-')
    {
        if(!isdigit(src[pos])) return false;
    }
    else if(!isdigit(*c)) return false;
    identifier = *c;
    bool hasDot = false;
    while(isdigit(src[pos]) || (src[pos] == '.' && !hasDot))
    {
        identifier += src[pos];
        if(src[pos] == '.') hasDot = true;
        pos++;
    }
    curr = NUMBER;
    return true;
}

bool ident(char* c)
{
    if(!isalpha(*c) && *c != '_') return false;
    identifier = *c;
    while(isalnum(src[pos]) || src[pos] == '_')
    {
        identifier += src[pos];
        pos++;
    }
    curr = IDENTIFIER;
    return true;
}

Token next_tok()
{
    if(pos == src.size()) return T_EOF;
    while(isspace(src[pos])) pos++;
    char* c = &src[pos++];
    if(number(c)) return curr;
    else if(op(c)) return curr;
    else if(brackets(c)) return curr;
    else if(ident(c)) return curr;
    std::cout << "unknown token '" << src[pos - 1] << "'\n";
    return T_EOF;
}

std::string get_str()
{
    return identifier;
}

void error(const char* msg)
{   // TODO: add position information for easier debugging
    std::cout << msg << " at " << pos << ": " << curr << "\n";
}