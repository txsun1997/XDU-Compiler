#include "parser.h"

int flag = 0;           //Handle errCode 7
FILE *fout_p = fopen("output_parser.txt","w");

void Parser::fetchToken()
{
    tk = lex->getToken();
    if(tk.type == ERRTOKEN){
        err[errNo].errCode = 1;
        err[errNo].lineNo = lex->getLine();
        strcpy(err[errNo].object, tk.str);
        errNo++;
    }
}

void Parser::matchToken(enum token_type tk_t)
{
    if(tk.type != tk_t){
        if(tk_t == SEMICOLON){
            err[errNo].errCode = 4;
            err[errNo].lineNo = lex->getLine();
            strcpy(err[errNo].object, tk.str);
            errNo++;
            return ;        //Avoid fetchToken again!
        }
        else if(tk_t == COMMA){
            err[errNo].errCode = 7;
            err[errNo].lineNo = lex->getLine();
            //strcpy(err[errNo].object, tk.str);
            errNo++;
            flag = 1;
            while(tk.type != SEMICOLON){
                fetchToken();
            }
            return ;        //Avoid fetchToken again!
        }
        else{
            err[errNo].errCode = 2;
            err[errNo].lineNo = lex->getLine();
            strcpy(err[errNo].object, tk.str);
            errNo++;
        }
    }
    else
        if(strcmp(tk.str,""))
            fprintf(fout_p, "\t'%s' Matched.\n", tk.str);
    fetchToken();
}

void Parser::printSyntaxTree(struct eNode *root, int indent)
{
    int i;
    for(i=1; i<=indent; i++)
        fputs("\t", fout_p);
    switch(root->op)
    {
        case PLUS:      fprintf(fout_p, "  %s\n","+"); break;
        case MINUS:     fprintf(fout_p, "  %s\n","-"); break;
        case MUL:       fprintf(fout_p, "  %s\n","*"); break;
        case DIV:       fprintf(fout_p, "  %s\n","/"); break;
        case POW:       fprintf(fout_p, "  %s\n","**");break;
        case FUNC:      fprintf(fout_p, "  %x\n",root->Content.caseFunc.MathFuncPtr); break;
        case CONST_ID:  fprintf(fout_p, "  %f\n",root->Content.caseConst); break;
        case T:         fputs("  T\n", fout_p); break;
        default:        fputs("  Error Tree Node!\n", fout_p); return;
    }
    if(root->op == CONST_ID || root->op == T)
        return ;
    if(root->op == FUNC)
        printSyntaxTree(root->Content.caseFunc.child, indent+1);
    else{
        printSyntaxTree(root->Content.caseOp.left, indent+1);
        printSyntaxTree(root->Content.caseOp.right,indent+1);
    }
}

struct eNode* Parser::makeTree(enum token_type opcode, ...)
{
    struct eNode *ePtr = new(struct eNode);
    ePtr->op = opcode;
    va_list argPtr;
    va_start (argPtr, opcode);
    switch(opcode)
    {
    case ERRTOKEN:
        ePtr->Content.caseConst = -1.0;
    case CONST_ID:
        ePtr->Content.caseConst = (double)va_arg(argPtr, double);
        break;
    case T:
        ePtr->Content.caseParmPtr = &para.para_t;
        break;
    case FUNC:
        ePtr->Content.caseFunc.MathFuncPtr = (FuncPtr)va_arg(argPtr, FuncPtr);
        ePtr->Content.caseFunc.child = (struct eNode *)va_arg(argPtr, struct eNode *);
        break;
    default:  // Normal operation
        ePtr->Content.caseOp.left = (struct eNode *)va_arg(argPtr, struct eNode *);
        ePtr->Content.caseOp.right = (struct eNode *)va_arg(argPtr, struct eNode *);
        break;
    }
    va_end(argPtr);
    return ePtr;
}

struct eNode* Parser::Atom()
{
    struct eNode *address, *tmp;
    struct token t = tk;
    switch(tk.type)
    {
    case CONST_ID:
        matchToken(CONST_ID);
        address = makeTree(CONST_ID, t.val);
        break;
    case T:
        matchToken(T);
        address = makeTree(T);
        break;
    case FUNC:
        matchToken(FUNC);
        matchToken(_LP);
        tmp = Expression();
        address = makeTree(FUNC, t.FuncPtr, tmp);
        matchToken(RP);
        break;
    case _LP:
        matchToken(_LP);
        address = Expression();
        matchToken(RP);
        break;
    default:
        err[errNo].errCode = 3;
        err[errNo].lineNo = lex->getLine();
        strcpy(err[errNo].object, tk.str);
        errNo++;
        address = makeTree(ERRTOKEN);
        fetchToken();
        break;
    }
    return address;
}

struct eNode* Parser::Component()
{
    struct eNode *left, *right;
    left = Atom();
    if(tk.type == POW){
        matchToken(POW);
        right = Component();
        left = makeTree(POW, left, right);
    }
    return left;
}

struct eNode* Parser::Factor()           // positive,negative
{
    struct eNode *left, *right;
    if(tk.type == PLUS){
        matchToken(PLUS);
        right = Factor();
    }
    else if(tk.type == MINUS){
        matchToken(MINUS);
        right = Factor();
        left = new eNode;
        left->op = CONST_ID;
        left->Content.caseConst = 0.0;
        right = makeTree(MINUS, left, right);
    }
    else{
        right = Component();
    }
    return right;
}

struct eNode* Parser::Term()             // MUL,DIV
{
    struct eNode *left, *right;
    token_type token_tmp;
    left = Factor();
    while(tk.type==MUL || tk.type==DIV){
        token_tmp = tk.type;
        matchToken(token_tmp);
        right = Factor();
        left = makeTree(token_tmp, left, right);
    }
    return left;
}

struct eNode* Parser::Expression()       // PLUS,MINUS
{
    //printf("\t\tBegin: Expression\n");
    struct eNode *left, *right;
    token_type token_tmp;
    left = Term();
    while(tk.type==PLUS || tk.type==MINUS){
        token_tmp = tk.type;
        matchToken(token_tmp);
        right = Term();
        left = makeTree(token_tmp, left, right);
    }
    //printf("\t\tEnd: Expression\n");
    return left;
}

void Parser::plot(double start, double end, double step, struct eNode *x, struct eNode *y)
{
    for(para.para_t = start; para.para_t<=end; para.para_t = para.para_t+step){
        double x_v = calExpr(x);
        double y_v = calExpr(y);  //adapt axis-y
        //scale
        x_v *= para.scale_x;
        y_v *= para.scale_y;
        //rot
        double x_tmp = x_v;
        x_v = x_v*cos(para.rot_angle) + y_v*sin(para.rot_angle);
        y_v = y_v*cos(para.rot_angle) - x_tmp*sin(para.rot_angle);
        //origin
        x_v += para.origin_x;
        y_v += para.origin_y;
        //printf("\t(%f,%f)\n",x_v, y_v);
        putpixel(x_v,Y_RANGE-y_v,clr);
    }
}

void Parser::ForStatement()
{
    fputs("      Begin: ForStatement\n", fout_p);
    struct eNode *start, *end, *step, *x, *y;
    double start_v, end_v, step_v;
    matchToken(FOR);
    matchToken(T);
    matchToken(FROM);
    start = Expression();
    start_v = calExpr(start);
    fputs("\tFrom:\n", fout_p);
    printSyntaxTree(start, 1);

    matchToken(TO);
    end = Expression();
    end_v = calExpr(end);
    fputs("\tTo:\n", fout_p);
    printSyntaxTree(end, 1);

    matchToken(STEP);
    step = Expression();
    step_v = calExpr(step);
    fputs("\tStep:\n", fout_p);
    printSyntaxTree(step, 1);

    matchToken(DRAW);
    matchToken(_LP);
    x = Expression();
    fputs("\taxis-X:\n", fout_p);
    printSyntaxTree(x, 1);

    matchToken(COMMA);
    if(flag){
        flag = 0;
        return ;
    }
    y = Expression();
    fputs("\taxis-Y:\n", fout_p);
    printSyntaxTree(y, 1);

    matchToken(RP);
    fputs("      End: ForStatement\n", fout_p);
    fputs("----------- Start plotting -----------\n", fout_p);
    plot(start_v, end_v, step_v, x, y);
}

void Parser::LabelStatement()
{
    setcolor(BLACK);
    struct eNode *x_ptr, *y_ptr;
    fputs("      Begin: LabelStatement\n", fout_p);
    setfont(&font);
    matchToken(LABEL);
    matchToken(_LP);
    x_ptr = Expression();
    double l_x = calExpr(x_ptr);

    matchToken(COMMA);
    if(flag){
        flag = 0;
        return ;
    }
    y_ptr = Expression();
    double l_y = calExpr(y_ptr);

    matchToken(COMMA);
    if(flag){
        flag = 0;
        return ;
    }
    char content[50];
    strcpy(content, tk.str);

    fetchToken();

    if(tk.type == COMMA){
        matchToken(COMMA);
        LOGFONT tmpF;
        getfont(&tmpF);
        strcpy(tmpF.lfFaceName, tk.str);
        //fprintf(fout_p, "\n%s\n",tk.str);
        setfont(&tmpF);

        fetchToken();
    }

    outtextxy(l_x, Y_RANGE-l_y, content);
    //matchToken(STR);
    matchToken(RP);
}

void Parser::AxisxStatement()
{
    setcolor(BLACK);
    struct eNode *len_ptr, *n_ptr;
    fputs("      Begin: AxisxStatement\n", fout_p);
    matchToken(AXISX);
    matchToken(_LP);
    len_ptr = Expression();
    double len = calExpr(len_ptr);     //length of axis-x

    matchToken(COMMA);
    if(flag){
        flag = 0;
        return ;
    }
    n_ptr = Expression();
    double n = calExpr(n_ptr);     //# of scale

    matchToken(COMMA);             //now tk represents literal
    if(flag){
        flag = 0;
        return ;
    }

    axis.ox = para.origin_x;
    axis.oy = para.origin_y;
    axis.xc = n;
    axis.xlen = len*para.scale_x;
    double dd = axis.xlen/50;

    LOGFONT tmpF;
    getfont(&tmpF);
    tmpF.lfWeight = 600;
    tmpF.lfItalic = TRUE;
    strcpy(tmpF.lfFaceName, "Times New Roman");
    setfont(&tmpF);

    line(axis.ox, Y_RANGE-axis.oy, axis.ox+axis.xlen, Y_RANGE-axis.oy);
    outtextxy(axis.ox+axis.xlen - 10, Y_RANGE-axis.oy + 5, tk.str);

    if(axis.xc != 0){               //draw scales
        for(int i=axis.ox; i<axis.ox+axis.xlen; i=i+double(axis.xlen/n)){
            line(i, Y_RANGE-axis.oy, i, Y_RANGE-axis.oy-dd);
        }
    }

    fetchToken();
    //matchToken(STR);
    matchToken(RP);
}

void Parser::AxisyStatement()
{
    setcolor(BLACK);
    struct eNode *len_ptr, *n_ptr;
    fputs("      Begin: AxisyStatement\n", fout_p);
    matchToken(AXISY);
    matchToken(_LP);
    len_ptr = Expression();
    double len = calExpr(len_ptr);     //length of axis-y

    matchToken(COMMA);
    if(flag){
        flag = 0;
        return ;
    }
    n_ptr = Expression();
    double n = calExpr(n_ptr);     //# of scale

    matchToken(COMMA);             //now tk represents literal
    if(flag){
        flag = 0;
        return ;
    }

    axis.yc = n;
    axis.ylen = len*para.scale_y;
    double dd = axis.ylen/50;

    LOGFONT tmpF;
    getfont(&tmpF);
    tmpF.lfWeight = 600;
    tmpF.lfItalic = TRUE;
    strcpy(tmpF.lfFaceName, "Times New Roman");
    setfont(&tmpF);

    line(axis.ox, Y_RANGE-axis.oy, axis.ox, Y_RANGE-axis.oy-axis.ylen);
    outtextxy(axis.ox - 20, Y_RANGE-axis.oy-axis.ylen, tk.str);

    if(axis.yc != 0){               //draw scales
        for(int i=axis.oy; i<axis.oy+axis.ylen; i=i+double(axis.ylen/n)){
            line(axis.ox, Y_RANGE-i, axis.ox+dd, Y_RANGE-i);
        }
    }

    fetchToken();
    //matchToken(STR);
    matchToken(RP);
}

void Parser::BkgStatement()
{
    fputs("      Begin: BackgroundStatement\n", fout_p);
    matchToken(BACKGROUND);
    matchToken(_COLOR);
    matchToken(IS);
    switch(tk.type)
    {
    case _WHITE:
        setbkcolor(WHITE); break;
    case _BLACK:
        setbkcolor(BLACK); break;
    default:
        err[errNo].errCode = 6;
        err[errNo].lineNo = lex->getLine();
        strcpy(err[errNo].object, tk.str);
        errNo++;
        break;
    }
    fprintf(fout_p, "\tBackground Color : %s\n", tk.str);
    fetchToken();
    //Equivalent to manual matchToken()

    fputs("      End: Background Statement\n", fout_p);
}

void Parser::ColorStatement()
{
    fputs("      Begin: ColorStatement\n", fout_p);
    matchToken(_COLOR);
    matchToken(IS);
    switch(tk.type)
    {
    case _BLUE:
        clr = BLUE;   break;
    case _RED:
        clr = RED;    break;
    case _YELLOW:
        clr = YELLOW; break;
    case _GREEN:
        clr = GREEN;  break;
    default:
        err[errNo].errCode = 6;
        err[errNo].lineNo = lex->getLine();
        strcpy(err[errNo].object, tk.str);
        errNo++;
        break;
    }
    fprintf(fout_p, "\tColor : %s\n", tk.str);
    fetchToken();
    //Equivalent to manual matchToken()

    fputs("      End: ColorStatement\n", fout_p);
}

void Parser::RotStatement()
{
    struct eNode *angle;
    fputs("      Begin: RotStatement\n", fout_p);
    matchToken(ROT);
    matchToken(IS);
    angle = Expression();
    fputs("\tRotation angle:\n", fout_p);
    printSyntaxTree(angle, 1);
    para.rot_angle = calExpr(angle);
    fprintf(fout_p, "\t\tValue of Tree:%f\n",para.rot_angle);

    fputs("      End: RotStatement\n", fout_p);
}

void Parser::ScaleStatement()
{
    fputs("      Begin: ScaleStatement\n", fout_p);
    struct eNode *x_scale, *y_scale;
    matchToken(SCALE);
    matchToken(IS);
    matchToken(_LP);
    x_scale = Expression();
    fputs("\tScale-X:\n", fout_p);
    printSyntaxTree(x_scale, 1);
    para.scale_x = calExpr(x_scale);
    fprintf(fout_p, "\t\tValue of Tree:%f\n",para.scale_x);

    matchToken(COMMA);
    if(flag){
        flag = 0;
        return ;
    }
    y_scale = Expression();
    fputs("\tScale-Y:\n", fout_p);
    printSyntaxTree(y_scale, 1);
    para.scale_y = calExpr(y_scale);
    fprintf(fout_p, "\t\tValue of Tree:%f\n",para.scale_y);

    matchToken(RP);
    fputs("      End: ScaleStatement\n", fout_p);
}

void Parser::OriginStatement()
{
    struct eNode *x_ptr, *y_ptr;
    fputs("      Begin: OriginStatement\n", fout_p);
    matchToken(ORIGIN);
    matchToken(IS);
    matchToken(_LP);
    x_ptr = Expression();
    fputs("\tOrigin-X:\n", fout_p);
    printSyntaxTree(x_ptr, 1);
    para.origin_x = calExpr(x_ptr);
    fprintf(fout_p, "\t\tValue of Tree:%f\n",para.origin_x);

    matchToken(COMMA);
    if(flag){
        flag = 0;
        return ;
    }
    y_ptr = Expression();
    fputs("\tOrigin-Y:\n", fout_p);
    printSyntaxTree(y_ptr, 1);
    para.origin_y = calExpr(y_ptr);
    fprintf(fout_p, "\t\tValue of Tree:%f\n",para.origin_y);

    matchToken(RP);
    fputs("      End: OriginStatement\n", fout_p);
}

void Parser::ClearStatement()
{
    fputs("      ClearStatement\n", fout_p);
    matchToken(CLEAR);
    cleardevice();
    para = {0, 0, 0, 1, 1, 0};
    axis = {0, 0, 0, 0, 0, 0};
    clr = RED;
}

void Parser::Statement()
{
    fputs("    Begin: Statement\n", fout_p);
    switch(tk.type)
    {
        case ORIGIN: OriginStatement(); break;
        case SCALE:  ScaleStatement();  break;
        case ROT:    RotStatement();    break;
        case FOR:    ForStatement();    break;
        case _COLOR: ColorStatement();  break;
        case LABEL:  LabelStatement();  break;
        case CLEAR:  ClearStatement();  break;
        case BACKGROUND:BkgStatement(); break;
        case AXISX:  AxisxStatement();  break;
        case AXISY:  AxisyStatement();  break;
        default:
            err[errNo].errCode = 8;
            err[errNo].lineNo = lex->getLine();
            //strcpy(err[errNo].object, tk.str);
            errNo++;
            //Avoid repeat error.
            while(tk.type!=SEMICOLON){
                fetchToken();
            }
            break;
    }
    fputs("    End: Statement\n", fout_p);
}

void Parser::Program()
{
    fputs("  Begin: Program\n", fout_p);
    while(tk.type != NONTOKEN)
    {
        Statement();
        matchToken(SEMICOLON);
    }
    fputs("  End: Program\n", fout_p);
}

Parser::Parser(const char *filename)
{
    fputs("Parser Initializing...\n", fout_p);
    lex = new lexer(filename);
    para = {0, 0, 0, 1, 1, 0};
    axis = {0, 0, 0, 0, 0, 0};
    errNo = 0;
    clr = RED;
    getfont(&font);
    strcpy(font.lfFaceName, "Times New Roman");
    setfont(&font);
}

Parser::~Parser()
{
    delete lex;
}

void Parser::Analyze()
{
    fputs("---------------------------------------------------\n", fout_p);
    fputs("Begin: Parser\n", fout_p);
    setinitmode(1, 200, 100);
    initgraph(X_RANGE, Y_RANGE);
    setbkcolor(WHITE);
    fetchToken();
    Program();
    fputs("End: Paser\n", fout_p);
    fputs("---------------------------------------------------\n", fout_p);
    printError();
    getch();
    closegraph();
}

void Parser::printError()
{
    int errOut = 0;
    int tmp = 0;
    for(int i=0; i<errNo; i++){
        if(tmp != err[i].lineNo){
            errOut++;
            tmp = err[i].lineNo;
        }
    }
    fprintf(fout_p, "Analyze finished: %d error(s).\n",errOut);
    fputs("---------------------------------------------------", fout_p);
    tmp = 0;
    for(int i=0; i<errNo; i++){
        if(tmp == err[i].lineNo){   //errors in the same line
            switch(err[i].errCode)
            {
                case 1:fprintf(fout_p, "Error token '%s'. ",err[i].object); break;
                case 2:fprintf(fout_p, "Unexpected token '%s'. ",err[i].object); break;
                case 3:fprintf(fout_p, "'%s': Undefined token in expression. ",err[i].object); break;
                case 4:fprintf(fout_p, "Expect ';' before '%s'. ",err[i].object); break;
                case 5:fprintf(fout_p, "Divided by 0. "); break;
                case 6:fprintf(fout_p, "Color '%s' is not supported yet. ",err[i].object); break;
                case 7:fprintf(fout_p, "Expect more parameter(s). "); break;
                case 8:fprintf(fout_p, "Wrong Statement. "); break;
                default:fprintf(fout_p, "Error unknown. "); break;
            }
        }
        else{
            fprintf(fout_p, "\nerror in line %d: ",err[i].lineNo);
            switch(err[i].errCode)
            {
                case 1:fprintf(fout_p, "Error token '%s'. ",err[i].object); break;
                case 2:fprintf(fout_p, "Unexpected token '%s'. ",err[i].object); break;
                case 3:fprintf(fout_p, "'%s': Undefined token in expression. ",err[i].object); break;
                case 4:fprintf(fout_p, "Expect ';' before '%s'. ",err[i].object); break;
                case 5:fprintf(fout_p, "Divided by 0. "); break;
                case 6:fprintf(fout_p, "Color '%s' is not supported yet. ",err[i].object); break;
                case 7:fprintf(fout_p, "Expect more parameter(s). "); break;
                case 8:fprintf(fout_p, "Wrong Statement. "); break;
                default:fprintf(fout_p, "Error unknown. "); break;
            }
            tmp = err[i].lineNo;
        }
    }
    fputs("\n", fout_p);
}

double Parser::calExpr(struct eNode *root)
{
    if(!root)
        return 0;
    switch(root->op)
    {
    case PLUS:
        return calExpr(root->Content.caseOp.left) + calExpr(root->Content.caseOp.right);
    case MINUS:
        return calExpr(root->Content.caseOp.left) - calExpr(root->Content.caseOp.right);
    case MUL:
        return calExpr(root->Content.caseOp.left) * calExpr(root->Content.caseOp.right);
    case DIV:
        if(calExpr(root->Content.caseOp.right) == 0){
            err[errNo].errCode = 5;
            err[errNo].lineNo = lex->getLine();
            errNo++;
            return 0;
        }
        else
            return calExpr(root->Content.caseOp.left) / calExpr(root->Content.caseOp.right);
    case POW:
        return pow(calExpr(root->Content.caseOp.left), calExpr(root->Content.caseOp.right));
    case FUNC:
        return (* root->Content.caseFunc.MathFuncPtr)(calExpr(root->Content.caseFunc.child));
    case CONST_ID:
        return root->Content.caseConst;
    case T:
        return *(root->Content.caseParmPtr);
    default:
        return 0;
    }
}
