#include "tokens.hpp"
#include <iostream>
#include <fstream>
#include <cassert>
#include <string>

#define PATTERN_RESET 0b11111111111111111111111111111111111
#define NUM_PATTERNS 35

#define PATTERN_KEYWORD(KW, T) \
    void match_ ## KW () \
    { \
        const char keyword[] = #KW; \
        if(count >= sizeof(keyword)) \
        { \
            toggle_pattern(T); \
            return; \
        } \
        if(src[pos] != keyword[count]) \
        { \
            toggle_pattern(T); \
            return; \
        } \
        if(count + 2 < sizeof(keyword) && src[pos + 1] != keyword[count + 1]) \
        { \
            toggle_pattern(T); \
            return; \
        } \
        lastMatch = T; \
    }

#define PATTERN_SYM(N, S, T) \
    void match_ ## N () \
    { \
        const char sym[] = #S; \
        if(count >= sizeof(sym)) \
        { \
            toggle_pattern(T); \
            return; \
        } \
        if(src[pos] != sym[count]) \
        { \
            toggle_pattern(T); \
            return; \
        } \
        if(count + 2 < sizeof(sym) && src[pos + 1] != sym[count + 1]) \
        { \
            toggle_pattern(T); \
            return; \
        } \
        lastMatch = T; \
    }

#define PATTERN_SYMS(N, S, T) \
    void match_ ## N () \
    { \
        const char sym[] = S; \
        if(count >= sizeof(sym)) \
        { \
            toggle_pattern(T); \
            return; \
        } \
        if(src[pos] != sym[count]) \
        { \
            toggle_pattern(T); \
            return; \
        } \
        if(count + 2 < sizeof(sym) && src[pos + 1] != sym[count + 1]) \
        { \
            toggle_pattern(T); \
            return; \
        } \
        lastMatch = T; \
    }

std::string str_ident, str_identCurr;
std::string src;
size_t startPos = 0;
size_t prevPos = 0;
size_t startLine = 1;
size_t startCol = 1;
size_t prevLine = 1;
size_t prevCol = 1;
size_t pos = 0;
size_t count = 0;
size_t line = 1, col = 1;
uint64_t valid_patterns = PATTERN_RESET;
Token lastMatch = T_INVALID;

bool flags[64] = {false};

void load_source(const std::string& file, const size_t& _pos)
{
    // TODO:
    std::ifstream in(file);
    src = "";
    std::string line = "";
    while(std::getline(in, line)) src += line + "\n";
    in.close();
    
    pos = _pos;
    startPos = pos;
}

void reset_valid()
{
    valid_patterns = PATTERN_RESET;
    for(int i = 0; i < 64; i++) flags[i] = false;
}

void toggle_pattern(const int pattern)
{
    assert(pattern < NUM_PATTERNS);
    valid_patterns ^= (uint64_t(1) << uint64_t(pattern));
    assert(valid_patterns <= PATTERN_RESET);
}

bool pattern_enabled(const int pattern)
{
    assert(pattern < NUM_PATTERNS);
    return bool(valid_patterns & (uint64_t(1) << uint64_t(pattern)));
}

bool isDigit(char c)
{
    return (c >= '0' && c <= '9');
}

bool isAlpha(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

bool isAlnum(char c)
{
    return isDigit(c) || isAlpha(c);
}

bool isSpace(char c)
{
    return (c == ' ') || (c == '\t') || (c == '\r') || (c == '\n');
}

void match_identifier()
{
    if(count == 0)
    {
        if(!isAlpha(src[pos]) && src[pos] != '_')
        {
            toggle_pattern(IDENTIFIER);
            return;
        }
    }
    else if(!isAlnum(src[pos]) && src[pos] != '_')
    {
        toggle_pattern(IDENTIFIER);
        return;
    }

    lastMatch = IDENTIFIER;
}

void match_number()
{
    if(count == 0)
    {
        if(!isDigit(src[pos]))
        {
            toggle_pattern(NUMBER);
            return;
        }
    }
    else if(!isDigit(src[pos]) && src[pos] != '.')
    {
        toggle_pattern(NUMBER);
        return;
    }
    if(src[pos] == '.')
    {
        if(flags[NUMBER])
        {
            toggle_pattern(NUMBER);
            return;
        }
        flags[NUMBER] = true;
    }

    lastMatch = NUMBER;
}

void match_string()
{
    if(flags[STRING])
    {
        toggle_pattern(STRING);
        return;
    }
    if(count == 0)
    {
        if(src[pos] != '"')
        {
            toggle_pattern(STRING);
            return;
        }
    }
    else if(src[pos] == '"')
    {
        flags[STRING] = true;
    }

    lastMatch = STRING;
}

PATTERN_KEYWORD(if, IF)
PATTERN_KEYWORD(while, WHILE)
PATTERN_KEYWORD(break, BREAK)
PATTERN_KEYWORD(continue, CONTINUE)
PATTERN_KEYWORD(return, RETURN)
PATTERN_SYM(assign, =, ASSIGN);

PATTERN_SYM(add, +, ADD)
PATTERN_SYM(sub, -, SUB)
PATTERN_SYM(mul, *, MUL)
PATTERN_SYM(div, /, DIV)

PATTERN_SYM(addadd, ++, ADDADD)
PATTERN_SYM(subsub, --, SUBSUB)

PATTERN_SYM(addequal, +=, ADDEQUAL)
PATTERN_SYM(subequal, -=, SUBEQUAL)
PATTERN_SYM(mulequal, *=, MULEQUAL)
PATTERN_SYM(divequal, /=, DIVEQUAL)

PATTERN_SYM(gt, >, GT)
PATTERN_SYM(ge, >=, GE)
PATTERN_SYM(lt, <, LT)
PATTERN_SYM(le, <=, LE)
PATTERN_SYM(eq, ==, EQ)
PATTERN_SYM(neq, !=, NEQ)

PATTERN_SYMS(open, "(", OPEN);
PATTERN_SYMS(close, ")", CLOSE);
PATTERN_SYMS(openb, "{", OPENB);
PATTERN_SYMS(closeb, "}", CLOSEB);
PATTERN_SYMS(opensq, "[", OPENSQ);
PATTERN_SYMS(closesq, "]", CLOSESQ);

PATTERN_SYMS(dot, ".", DOT)
PATTERN_SYMS(comma, ",", COMMA)
PATTERN_SYMS(colon, ":", COLON)
PATTERN_SYMS(semicolon, ";", SEMICOLON)


Token lex()
{
    reset_valid();
    while(isSpace(src[pos]))
    {
        col++;
        if(src[pos] == '\n')
        {
            line++;
            col = 1;
        }
        pos++;
    }
    if(pos >= src.size())
    {
        str_ident = str_identCurr;
        str_identCurr = "EOF";
        return T_EOF;
    }
    startPos = pos;
    while(true)
    {
        if(pos >= src.size())
        {
            reset_valid();
            str_ident = str_identCurr;
            str_identCurr = src.substr(startPos, count);
            while(isSpace(src[pos]))
            {
                col++;
                if(src[pos] == '\n')
                {
                    line++;
                    col = 1;
                }
                pos++;
            }
            prevLine = startLine;
            prevCol = startCol;
            startLine = line;
            startCol = col;
            prevPos = startPos;
            startPos = pos;
            count = 0;
            Token t = lastMatch;
            lastMatch = T_INVALID;
            return t;
        }

        if(pattern_enabled(IDENTIFIER))     match_identifier();
        if(pattern_enabled(NUMBER))         match_number();
        if(pattern_enabled(STRING))         match_string();
        if(pattern_enabled(IF))             match_if();
        if(pattern_enabled(WHILE))          match_while();
        if(pattern_enabled(BREAK))          match_break();
        if(pattern_enabled(CONTINUE))       match_continue();
        if(pattern_enabled(RETURN))         match_return();
        if(pattern_enabled(ASSIGN))         match_assign();
        if(pattern_enabled(ADD))            match_add();
        if(pattern_enabled(SUB))            match_sub();
        if(pattern_enabled(MUL))            match_mul();
        if(pattern_enabled(DIV))            match_div();
        if(pattern_enabled(ADDADD))         match_addadd();
        if(pattern_enabled(SUBSUB))         match_subsub();
        if(pattern_enabled(ADDEQUAL))       match_addequal();
        if(pattern_enabled(SUBEQUAL))       match_subequal();
        if(pattern_enabled(MULEQUAL))       match_mulequal();
        if(pattern_enabled(DIVEQUAL))       match_divequal();
        if(pattern_enabled(GT))             match_gt();
        if(pattern_enabled(GE))             match_ge();
        if(pattern_enabled(LT))             match_lt();
        if(pattern_enabled(LE))             match_le();
        if(pattern_enabled(EQ))             match_eq();
        if(pattern_enabled(NEQ))            match_neq();
        if(pattern_enabled(OPEN))           match_open();
        if(pattern_enabled(CLOSE))          match_close();
        if(pattern_enabled(OPENB))          match_openb();
        if(pattern_enabled(CLOSEB))         match_closeb();
        if(pattern_enabled(OPENSQ))         match_opensq();
        if(pattern_enabled(CLOSESQ))        match_closesq();
        if(pattern_enabled(DOT))            match_dot();
        if(pattern_enabled(COMMA))          match_comma();
        if(pattern_enabled(COLON))          match_colon();
        if(pattern_enabled(SEMICOLON))      match_semicolon();

        // std::cout << "(";
        // for(uint64_t i = NUM_PATTERNS - 1; i < NUM_PATTERNS; i--)
        // {
        //     std::cout << ((valid_patterns >> i) & 1);
        // }
        // std::cout << ")";

        if(valid_patterns == 0)
        {
            reset_valid();
            str_ident = str_identCurr;
            str_identCurr = src.substr(startPos, count);
            while(isSpace(src[pos]))
            {
                col++;
                if(src[pos] == '\n')
                {
                    line++;
                    col = 1;
                }
                pos++;
            }
            prevLine = startLine;
            prevCol = startCol;
            startLine = line;
            startCol = col;
            prevPos = startPos;
            startPos = pos;
            count = 0;
            Token t = lastMatch;
            lastMatch = T_INVALID;
            return t;
        }
        pos++;
        col++;
        count++;
    }
}

std::string get_id()
{
    return str_ident;
}

void error(const std::string& msg)
{
    std::cerr << msg << " at line " << prevLine << ", column " << prevCol << "\n";
}
