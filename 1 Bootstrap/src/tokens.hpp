#pragma once

#include <string>

enum Token
{
    IDENTIFIER, NUMBER, STRING,
    IF, WHILE, BREAK, CONTINUE, RETURN, ASSIGN,
    ADD, SUB, MUL, DIV,
    ADDADD, SUBSUB,
    ADDEQUAL, SUBEQUAL, MULEQUAL, DIVEQUAL,
    GT, GE, LT, LE, EQ, NEQ,
    OPEN, CLOSE, OPENB, CLOSEB, OPENSQ, CLOSESQ,
    DOT, COMMA, COLON, SEMICOLON,
    T_EOF, // The value of T_EOF is equal to the total number of tokens
    T_INVALID,
};

static std::string TokenNames[]
{
    "Identifier", "Number", "String",
    "if", "while", "break", "continue", "return", "=",
    "+", "-", "*", "/",
    "++", "--",
    "+=", "-=", "*=", "/=",
    ">", ">=", "<", "<=", "==", "!=",
    "(", ")", "{", "}", "[", "]",
    ".", ",", ":", ";",
    "EOF",
    "Invalid",
};