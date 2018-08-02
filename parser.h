#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include <stdarg.h>
#include <graphics.h>

#define X_RANGE 1080
#define Y_RANGE 640
#define MAX_ERR 100

typedef double (* MathFuncPtr) (double);

struct eNode
{
    enum token_type op;
    union
    {
        struct {eNode *left, *right;} caseOp;
        struct {eNode *child; FuncPtr MathFuncPtr;} caseFunc;
        double caseConst;
        double *caseParmPtr;
    }Content;
};

class Parser
{
    lexer *lex;
    token tk;
    int errNo;
    struct error
    {
        int errCode;
        int lineNo;
        char object[MAX];
    } err[MAX_ERR];
    struct para_set
    {
        double para_t;
        double origin_x;
        double origin_y;
        double scale_x;
        double scale_y;
        double rot_angle;
    } para;
    struct axis_data
    {
        double ox;
        double oy;
        double xlen;
        double ylen;
        double xc;
        double yc;
    } axis;
    ege::COLORS clr; //color
    LOGFONT font;
public:
    Parser(const char *filename);
    ~Parser();
    void Analyze();
    void Program();
    void Statement();
    void OriginStatement();
    void ScaleStatement();
    void RotStatement();
    void ForStatement();
    void ColorStatement();
    void LabelStatement();
    void ClearStatement();
    void BkgStatement();
    void AxisxStatement();
    void AxisyStatement();
    struct eNode* Expression();
    struct eNode* Term();
    struct eNode* Factor();
    struct eNode* Component();
    struct eNode* Atom();
    struct eNode* makeTree(enum token_type opcode, ...);
    void printSyntaxTree(struct eNode *root, int indent);
    void matchToken(enum token_type tk_t);
    void fetchToken();
    void printError();
    double calExpr(struct eNode *root);
    void plot(double start, double end, double step, struct eNode *x, struct eNode *y);
};

#endif // PARSER_H
