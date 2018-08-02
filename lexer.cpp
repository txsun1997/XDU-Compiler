#include "lexer.h"

FILE *fout = fopen("output_lex.txt","w");

// initialization: set line number and open the source file.
lexer::lexer(const char *filename)
{
    line = 1;
    fin = fopen(filename, "r");
    if(!fin){
        //printf("Open file failed!\n");
        fputs("Open file failed!\n",fout);
    }
}

// close: close the source file.
lexer::~lexer()
{
    if(fin) fclose(fin);
}

// match token in symbol table.
token lexer::matchToken(const char *sym)
{
    for(int i=0; i<SYMNO; i++){
        if(!strcmp(symTab[i].str, sym))
            return symTab[i];
    }
    // Not found.
    token err = {ERRTOKEN, "", 0.0, NULL};
    return err;
}

//
token lexer::getToken()
{
    token tk = {NONTOKEN, "", 0.0, NULL};
    memset(buffer, 0, MAX);
    int ch;
    while(1){
        ch = toupper(getc(fin));
        if(ch == EOF)
            return tk;
        if(ch == '\n')
            line++;
        if(!isspace(ch))
            break;
    }
    buffer[strlen(buffer)] = ch;
    buffer[strlen(buffer)+1] = 0;
    if(ch == '"'){
        buffer[0] = 0;
        while(1){
            ch = getc(fin);
            buffer[strlen(buffer)] = ch;
            buffer[strlen(buffer)+1] = 0;
            if(ch == '"'){
                buffer[strlen(buffer)-1] = 0;
                break;
            }
        }
        token _tk = {STR, "", 0.0, NULL};
        strcpy(_tk.str, buffer);
        return _tk;
    }
    if(isalpha(ch)){                // the first of token is an alpha.
        while(1){
            ch = toupper(getc(fin));
            if(isalnum(ch)){        // if ch is alpha or number.
                buffer[strlen(buffer)] = ch;
                buffer[strlen(buffer)+1] = 0;
            }
            else
                break;
        }
        if(ch != EOF)
            ungetc(ch, fin);
        // Now we get a complete token
        tk = matchToken(buffer);
        strcpy(tk.str, buffer);       // for wrong token.
        return tk;
    }
    else if(isdigit(ch)){           // the first of token is a digit, which means constant.
        int flag = 0;
        while(1){
            ch = getc(fin);
            if(isdigit(ch) || ch=='.'){
                if(ch=='.'){
                    if(flag==1){    // >=2 '.'s
                        while(isdigit(ch) || ch=='.'){
                            buffer[strlen(buffer)] = ch;
                            buffer[strlen(buffer)+1] = 0;
                            ch = getc(fin);
                        }
                        if(ch != EOF)
                            ungetc(ch, fin);
                        tk.type = ERRTOKEN;
                        strcpy(tk.str, buffer);
                        return tk;
                    }
                    else{
                        flag = 1;
                        buffer[strlen(buffer)] = ch;
                        buffer[strlen(buffer)+1] = 0;
                    }
                }
                else{
                    buffer[strlen(buffer)] = ch;
                    buffer[strlen(buffer)+1] = 0;
                }
            }
            else
                break;
        }
        if(ch != EOF)
            ungetc(ch, fin);
        tk.type = CONST_ID;
        tk.val = atof(buffer);
        return tk;
    }
    else{                           // the first of token is not a alpha or a digit.
        switch(ch)
        {
            case '+': tk.type = PLUS; break;
            case '-':
                ch = toupper(getc(fin));
                if(ch == '-'){      // COMMENT
                    while(ch!='\n' && ch!=EOF)
                        ch = toupper(getc(fin));
                    if(ch != EOF)
                        ungetc(ch, fin);
                    return getToken();
                }
                else{
                    if(ch != EOF)
                        ungetc(ch, fin);
                    tk.type = MINUS;
                    break;
                }
            case '*':
                ch = toupper(getc(fin));
                if(ch == '*'){
                    tk.type = POW;
                    break;
                }
                else{
                    if(ch != EOF)
                        ungetc(ch, fin);
                    tk.type = MUL;
                    break;
                }
            case '/':
                ch = toupper(getc(fin));
                if(ch == '/'){      // COMMENT
                    while(ch!='\n' && ch!=EOF){
                        ch = toupper(getc(fin));
                    }
                    if(ch != EOF)
                        ungetc(ch, fin);
                    return getToken();
                }
                else{
                    if(ch != EOF)
                        ungetc(ch, fin);
                    tk.type = DIV;
                    break;
                }
            case '(': tk.type = _LP; break;
            case ')': tk.type = RP; break;
            case ';': tk.type = SEMICOLON; break;
            case ',': tk.type = COMMA; break;
            default : tk.type = ERRTOKEN; break;
        }
    }
    return tk;
}

void lexer::analyze()
{
    token tk;
    int i = 0;
    while(1){
        tk = getToken();
        if(tk.type != NONTOKEN)
            token_list[i++] = tk;
        else
            break;
    }
    size = i;
}

void lexer::print()
{
    //printf("  Lexical Analysis:\n");
    fputs("Lexical Analysis:\n", fout);
    //printf("---------------------------------------------------\n");
    fputs("---------------------------------------------------\n", fout);
    //printf("  Type       String         Value       FuncPtr\n");
    fputs("  Type       String         Value       FuncPtr\n", fout);
    //printf("---------------------------------------------------\n");
    fputs("---------------------------------------------------\n", fout);
    for(int i=0; i<size ;i++){
        fprintf(fout, "%6d %12s %14f %12x\n",
               token_list[i].type, token_list[i].str, token_list[i].val, token_list[i].FuncPtr);
    }
    //printf("---------------------------------------------------\n\n");
    fputs("---------------------------------------------------\n", fout);
}



