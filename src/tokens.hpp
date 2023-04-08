#pragma once

#include <string>

enum Token
{
    IDENTIFIER, NUMBER, STRING,
    USING,
    IF, WHILE, ELSE, BREAK, CONTINUE, RETURN, ASSIGN,
    ADD, SUB, MUL, DIV,
    AND, OR, XOR, MOD,
    ADDADD, SUBSUB,
    ADDEQUAL, SUBEQUAL, MULEQUAL, DIVEQUAL,
    ANDEQUAL, OREQUAL, XOREQUAL, MODEQUAL,
    GT, GE, LT, LE, EQ, NEQ, LOGAND, LOGOR,
    OPEN, CLOSE, OPENB, CLOSEB, OPENSQ, CLOSESQ,
    DOT, COMMA, COLON, SEMICOLON,
    NAMESPACE, GROUP, ENUM,
    PUBLIC, PRIVATE,
    COMMENT_LINE, COMMENT_BLOCK_OPEN, COMMENT_BLOCK_CLOSE,
    T_EOF, // The value of T_EOF is equal to the total number of tokens
    T_INVALID,
};

static std::string TokenNames[]
{
    "Identifier", "Number", "String",
    "using",
    "if", "while", "else", "break", "continue", "return", "=",
    "+", "-", "*", "/",
    "&", "|", "^", "%",
    "++", "--",
    "+=", "-=", "*=", "/=",
    "&=", "|=", "^=", "%=",
    ">", ">=", "<", "<=", "==", "!=", "&&", "||",
    "(", ")", "{", "}", "[", "]",
    ".", ",", ":", ";",
    "namespace", "group", "enum",
    "public", "private",
    "//", "/*", "*/",
    "EOF",
    "Invalid",
};