#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include "lexer.h"


Token *tokens = NULL;
Token *lastToken = NULL;
int line = 1;
const char *pCrtCh = NULL;


#define SAFEALLOC(var, Type)                          \
    if ((var = (Type *)malloc(sizeof(Type))) == NULL) \
    err("not enough memory")


void err(const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    fprintf(stderr, "error: ");
    vfprintf(stderr, fmt, va);
    fputc('\n', stderr);
    va_end(va);
    exit(-1);
}


void tkerr(const Token *tk, const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    fprintf(stderr, "error in line %d: ", tk->line);
    vfprintf(stderr, fmt, va);
    fputc('\n', stderr);
    va_end(va);
    exit(-1);
}


Token *addTk(int code)
{
    Token *tk;
    SAFEALLOC(tk, Token);
    tk->code = code;
    tk->line = line;
    tk->next = NULL;
    if (lastToken)
    {
        lastToken->next = tk;
    }
    else
    {
        tokens = tk;
    }
    lastToken = tk;
    printf("Added token: %d at line %d\n", tk->code, tk->line); 
    return tk;
}


char *createString(const char *start, const char *end)
{
    size_t len = end - start;
    char *str = (char *)malloc(len + 1);
    if (!str)
        err("not enough memory for string");
    memcpy(str, start, len);
    str[len] = '\0';
    return str;
}


int getNextToken() {
    int state = 0, nCh;
    char ch;
    const char *pStartCh;
    Token *tk;

    while (1) {
        ch = *pCrtCh;
        //printf("Processing character: '%c' (code %d) at line %d\n", ch, ch, line);

        switch (state) {
            case 0:
                if (isalpha(ch) || ch == '_') {
                    pStartCh = pCrtCh++;
                    state = 1;
                } else if (ch == ' ' || ch == '\r' || ch == '\t') {
                    pCrtCh++;
                } else if (ch == '\n') {
                    line++; 
                    pCrtCh++;
                } else if (ch >= '1' && ch <= '9') {
                    pStartCh = pCrtCh++;
                    state = 3; 
                } else if (ch == '0') {
                    pStartCh = pCrtCh++;
                    state = 5; 
                } else if (ch == '\'') {
                    pStartCh = pCrtCh++;
                    state = 13; 
                } else if (ch == '"') {
                    pStartCh = pCrtCh++;
                    state = 18; 
                } else if (ch == '+') {
                    pCrtCh++; 
                    return addTk(ADD)->code;
                } else if (ch == '-') {
                    pCrtCh++; 
                    return addTk(SUB)->code;
                } else if (ch == '*') {
                    pCrtCh++; 
                    return addTk(MUL)->code;
                } else if (ch == '/') {
                    pCrtCh++; 
                    return addTk(DIV)->code;
                } else if (ch == ';') {
                    pCrtCh++; 
                    return addTk(SEMICOLON)->code;
                } else if (ch == ',') {
                    pCrtCh++; 
                    return addTk(COMMA)->code;
                } else if (ch == '(') {
                    pCrtCh++; 
                    return addTk(LPAR)->code;
                } else if (ch == ')') {
                    pCrtCh++; 
                    return addTk(RPAR)->code;
                } else if (ch == '{') {
                    pCrtCh++; 
                    return addTk(LACC)->code;
                } else if (ch == '}') {
                    pCrtCh++; 
                    return addTk(RACC)->code;
                } else if (ch == '[') {
                    pCrtCh++; 
                    return addTk(LBRACKET)->code;
                } else if (ch == ']') {
                    pCrtCh++; 
                    return addTk(RBRACKET)->code;
                } else if (ch == '=') {
                    pCrtCh++;
                    return addTk(ASSIGN)->code; 
                } else if (ch == 0) { 
                    addTk(END);
                    return END;
                } else {
                    tkerr(addTk(END), "invalid character");
                }
                break;

            case 1: 
                if (isalnum(ch) || ch == '_') {
                    pCrtCh++;
                } else {
                    state = 2; 
                }
                break;

            case 2: 
                nCh = pCrtCh - pStartCh;
                if (nCh == 5 && !memcmp(pStartCh, "break", 5)) {
                    tk = addTk(BREAK);
                } else if (nCh == 4 && !memcmp(pStartCh, "char", 4)) {
                    tk = addTk(CHAR);
                } else if (nCh == 2 && !memcmp(pStartCh, "if", 2)) {
                    tk = addTk(IF);
                } else if (nCh == 3 && !memcmp(pStartCh, "for", 3)) {
                    tk = addTk(FOR);
                } else if (nCh == 6 && !memcmp(pStartCh, "return", 6)) {
                    tk = addTk(RETURN);
                } else if (nCh == 7 && !memcmp(pStartCh, "struct", 6)) {
                    tk = addTk(STRUCT);
                } else if (nCh == 4 && !memcmp(pStartCh, "void", 4)) {
                    tk = addTk(VOID);
                } else if (nCh == 5 && !memcmp(pStartCh, "while", 5)) {
                    tk = addTk(WHILE);
                } else if (nCh == 4 && !memcmp(pStartCh, "else", 4)) {
                    tk = addTk(ELSE);
                } else if (nCh == 5 && !memcmp(pStartCh, "double", 6)) {
                    tk = addTk(DOUBLE);
                } else if (nCh == 3 && !memcmp(pStartCh, "int", 3)) {
                    tk = addTk(INT);
                } else if (nCh == 0) { 
                    state = -1; 
                } else{
                    tk = addTk(ID);
                    tk->text = createString(pStartCh, pCrtCh);
                }
                return tk->code;

            case 3: 
                if (ch >= '0' && ch <= '9') {
                    pCrtCh++;
                } else if (ch == '.') {
                    pCrtCh++; 
                    state = 7; 
                } else if (ch == 'e' || ch == 'E') {
                    pCrtCh++; 
                    state = 9; 
                } else {
                    state = 4; 
                }
                break;

            case 4: 
                tk = addTk(CT_INT);
                tk->text = createString(pStartCh, pCrtCh);
                return CT_INT;

            case 5: 
                if (ch >= '0' && ch <= '7') {
                    pCrtCh++;  
                    state = 4; 
                } else if (ch == 'x' || ch == 'X') {
                    pCrtCh++; 
                    ch = *pCrtCh; 
                    if ((ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F')) {
                        pCrtCh++; 
                        state = 6; 
                    } else {
                        tkerr(addTk(END), "expected a hexadecimal digit after '0x'");
                    }
                } else if (ch == ' ' || ch == '\n' || ch == '\t' || ch == '\r' || ch == 0) {
                    state = 4; 
                } else if (ch == '.') {
                    pCrtCh++; 
                    state = 7; 
                } else if (ch == 'e' || ch == 'E') {
                    pCrtCh++; 
                    state = 9; 
                } else if (ch == '8' || ch == '9') {
                    pCrtCh++; 
                    state = 3; 
                } 
                else {
                    tkerr(addTk(END), "invalid character in octal/hex literal");
                }
                break;

            case 6: 
                if ((ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F')) {
                    pCrtCh++; 
                    state = 6;
                } else {
                    state = 4;
                    tk = addTk(CT_INT);  
                    tk->text = createString(pStartCh, pCrtCh);
                    return CT_INT; 
                }
            break;

            case 7:
                if (ch >= '0' && ch <= '9') {
                    pCrtCh++;
                    state = 8;
                } else {
                    tkerr(addTk(END), "Invalid character in fractional part");
                }
            break;
        
            case 8: 
                if (ch >= '0' && ch <= '9') {
                    pCrtCh++;
                } else if (ch == 'e' || ch == 'E') { 
                    pCrtCh++;
                    state = 9;
                } else {
                    state = 12;
                    tk = addTk(CT_REAL); 
                    tk->text = createString(pStartCh, pCrtCh);
                    return CT_REAL;
                }
            break;
        
            case 9: 
                if (ch == '+' || ch == '-') {
                    pCrtCh++; 
                } else if (ch >= '0' && ch <= '9') {
                    pCrtCh++;
                    state = 10; 
                } else {
                    tkerr(addTk(END), "Invalid or missing digits in exponent");
                }
            break;
            
            case 10:
                if (ch >= '0' && ch <= '9') {
                    pCrtCh++;
                } else {
                    tk = addTk(CT_REAL); 
                    tk->text = createString(pStartCh, pCrtCh);
                    return CT_REAL;
                }
            break;
            

            case 11: 
                if (ch >= '0' && ch <= '9') {
                    pCrtCh++;
                    state = 12;
                } else {
                    tk = addTk(CT_REAL);  
                    tk->text = createString(pStartCh, pCrtCh);
                    return CT_REAL; 
                }
            break;

            case 12: 
                    tk = addTk(CT_REAL);  
                    tk->text = createString(pStartCh, pCrtCh);
                    return CT_REAL; 

            case 13:
                if (ch == '\\') {
                    pCrtCh++;
                    state = 14; 
                } else if (ch != '\'' && ch > 31 && ch < 127) {
                    pCrtCh++;
                    state = 15; 
                } else {
                    tkerr(addTk(END), "Invalid character literal");
                }
            break;

            case 14:
             if (strchr("abfnrtv'?\"\\0", ch)) {
                    pCrtCh++;
                    state = 15; 
                }else {
                    tkerr(addTk(END), "Invalid escape sequence");
                }
            break;

            case 15:
                if (ch == '\'') {
                    pCrtCh++;
                    tk = addTk(CT_CHAR);
                    tk->text = createString(pStartCh, pCrtCh);
                    return CT_CHAR;
                } else {
                    tkerr(addTk(END), "Unterminated character literal");
                }
            break;

            case 16:
                tk = addTk(CT_CHAR); 
                tk->text = createString(pStartCh, pCrtCh);
                return CT_CHAR;

            case 17:
                if (strchr("abfnrtv'?\"\\0", ch)) {
                    pCrtCh++;
                    state = 15; 
                }else {
                    tkerr(addTk(END), "Invalid escape sequence");
                }
            break;

            case 18:
                if (ch == '\\') {
                    pCrtCh++;
                    state = 14;
                } else if (ch != '"' && ch > 31 && ch < 127) {
                    pCrtCh++;
                } else if (ch == '"') {
                    pCrtCh++;
                    state = 20; 
                } else {
                    tkerr(addTk(END), "Unterminated string literal");
                }
            break;

            
            case 19:
                if (ch == '"') {
                    pCrtCh++;
                    state = 20; 
                } else {
                    pCrtCh++;
                    state = 18;
                }
            break;

            case 20:
                tk = addTk(CT_STRING);
                tk->text = createString(pStartCh, pCrtCh);
                return CT_STRING;



            
        }
    }
}


void showTokens() {
    for (Token *tk = tokens; tk; tk = tk->next) {
        printf("Token code: ");
        switch (tk->code) {
            case ID: 
                printf("ID");
                if (tk->text) 
                    printf(" (%s)", tk->text);
            break;
            case CT_INT:
                printf("CT_INT");
            break;
            case CT_REAL:
                printf("CT_REAL");
            break;
            case CT_CHAR:
                printf("CT_CHAR");
            break;
            case CT_STRING:
                printf("CT_STRING");
            break;
            case BREAK:
                printf("BREAK");
            break;
            case CHAR:
                printf("CHAR");
            break;
            case END:
                printf("END");
            break;
            case SEMICOLON:
                printf("SEMICOLON");
            break;
            case COMMA:
                printf("COMMA");
            break; 
            case LPAR:
                printf("LPAR");
            break;
            case RPAR:
                printf("RPAR");
            break;
            case LBRACKET:
                printf("LBRACKET");
            break;
            case RBRACKET:
                printf("RBRACKET");
            break;
            case LACC:
                printf("LACC");
            break;
            case RACC:  
                printf("RACC");
            break;
            case ADD:
                printf("ADD");
            break;
            case SUB:
                printf("SUB");
            break;
            case MUL:
                printf("MUL");
            break;  
            case DIV:   
                printf("DIV");
            break;
            case DOT:
                printf("DOT");
            break;
            case OR:            
                printf("OR");
            break;
            case EQUAL:
                printf("EQUAL");
            break; 
            case INT:
                printf("INT");
            break; 
            case ASSIGN:
                printf("ASSIGN");
            break;
            case FOR:
                printf("FOR");
            break;  
            case IF:
                printf("IF");       
            break;
            case RETURN:
                printf("RETURN");
            break;
            case STRUCT:
                printf("STRUCT");   
            break;
            case VOID:
                printf("VOID");
            break;
            case WHILE:
                printf("WHILE");
            break;
            case AND:
                printf("AND");
            break;
            

            default:
                printf("UNKNOWN");
        }
        printf(" at line %d\n", tk->line);       
    }
}



void done()
{
    Token *tk;
    while (tokens)
    {
        tk = tokens;
        tokens = tokens->next;
        if (tk->code == ID)
            free(tk->text);
        free(tk);
    }
}


char *readFileContent(const char *fileName)
{
    FILE *file = fopen(fileName, "r");
    if (!file)
    {
        err("Could not open file %s", fileName);
    }
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *content = (char *)malloc(fileSize + 1);
    if (!content)
    {
        fclose(file);
        err("Not enough memory to read file");
    }

    fread(content, 1, fileSize, file);
    content[fileSize] = '\0';

    fclose(file);
    return content;
}


void setInput(const char *input)
{
    pCrtCh = input;
}
