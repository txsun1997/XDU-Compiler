#ifndef LEXER_H
#define LEXER_H

#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#define LENGTH 600      // max # of tokens
#define MAX    30      // max # of single token
#define SYMNO  30       // # of symbols in table

typedef double (* FuncPtr)(double);

enum token_type
{
    ERRTOKEN = -1, FOR, T, FROM, TO, STEP, DRAW, SCALE, //-1~6
    ROT, ORIGIN, IS, PLUS, MINUS, MUL, DIV, POW, _LP,   //7~15
    RP, SEMICOLON, COMMA, FUNC, CONST_ID, NONTOKEN,     //16~21
    LABEL, STR, _COLOR, _YELLOW, _BLUE, _RED, _GREEN,   //22~28
    CLEAR, BACKGROUND, _WHITE, _BLACK, AXISX, AXISY     //29~34
};

struct token
{
    token_type type;
    char       str[MAX];
    double     val;
    double     (*FuncPtr)(double);
};

static token symTab[] =
{
    {CONST_ID, "PI",    3.1415926,  NULL},
    {CONST_ID, "E",     2.7182818,  NULL},
    {FOR,      "FOR",   0.0,        NULL},
    {T,        "T",     0.0,        NULL},
    {FROM,     "FROM",  0.0,        NULL},
    {TO,       "TO",    0.0,        NULL},
    {STEP,     "STEP",  0.0,        NULL},
    {DRAW,     "DRAW",  0.0,        NULL},
    {SCALE,    "SCALE", 0.0,        NULL},
    {ROT,      "ROT",   0.0,        NULL},
    {ORIGIN,   "ORIGIN",0.0,        NULL},
    {_COLOR,   "COLOR", 0.0,        NULL}, //COLOR
    {_YELLOW,  "YELLOW",0.0,        NULL}, //YELLOW
    {_BLUE,    "BLUE",  0.0,        NULL}, //BLUE
    {_RED,     "RED",   0.0,        NULL}, //RED
    {_GREEN,   "GREEN", 0.0,        NULL}, //GREEN
    {_WHITE,   "WHITE", 0.0,        NULL}, //WHITE
    {_BLACK,   "BLACK", 0.0,        NULL}, //BLACK
    {LABEL,    "LABEL", 0.0,        NULL}, //LABEL
    {CLEAR,    "CLEAR", 0.0,        NULL}, //CLEAR
    {BACKGROUND,"BACKGROUND", 0.0,  NULL}, //BACKGROUND
    {AXISX,    "AXISX", 0.0,        NULL}, //AXIS-X
    {AXISY,    "AXISY", 0.0,        NULL}, //AXIS-Y
    {IS,       "IS",    0.0,        NULL},
    {FUNC,     "SIN",   0.0,        sin},
    {FUNC,     "COS",   0.0,        cos},
    {FUNC,     "TAN",   0.0,        tan},
    {FUNC,     "LN",    0.0,        log},
    {FUNC,     "EXP",   0.0,        exp},
    {FUNC,     "SQRT",  0.0,        sqrt}
};

class lexer
{
    FILE *fin;
    int line;
    char buffer[MAX];
    token token_list[LENGTH];
    int size;
public:
    lexer(const char *filename);
    ~lexer();
    token matchToken(const char *sym);
    token getToken();
    void analyze();
    void print();
    int getLine() {return line;};
    int getSize() {return size;};
};

#endif // LEXER_H
