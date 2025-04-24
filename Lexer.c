#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include "lexer.h"

// Global variables
Token *tokens = NULL;
Token *lastToken = NULL;
int line = 1;
const char *pCrtCh = NULL;

// Safe memory allocation
#define SAFEALLOC(var, Type)                          \
    if ((var = (Type *)malloc(sizeof(Type))) == NULL) \
    err("not enough memory")

// Error function
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

// Token error function
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

// Add token to the list
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
    return tk;
}

// Create a string from pStartCh to pCrtCh (excluding)
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

// Get the next token
int getNextToken() {
    int state = 0, nCh;
    char ch;
    const char *pStartCh;
    Token *tk;

    while (1) {
        ch = *pCrtCh;
        printf("Processing character: '%c' (code %d) at line %d\n", ch, ch, line);

        switch (state) {
            case 0:
                if (isalpha(ch) || ch == '_') {
                    pStartCh = pCrtCh++;
                    state = 1;
                } else if (ch == ' ' || ch == '\r' || ch == '\t') {
                    pCrtCh++; // Skip whitespace
                } else if (ch == '\n') {
                    line++;  // New line
                    pCrtCh++; // Skip newline character
                } else if (ch >= '1' && ch <= '9') {
                    pCrtCh++;
                    state = 3;
                } else if (ch == '0') {
                    pCrtCh++;
                    state = 5; // Handle zero specifically
                }          else if (ch == 0) {  // End of file reached
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
                    state = 2;  // Move to next state after processing identifier
                }
                break;

            case 2:
                nCh = pCrtCh - pStartCh;
                if (nCh == 5 && !memcmp(pStartCh, "break", 5)) {
                    tk = addTk(BREAK);
                } else if (nCh == 4 && !memcmp(pStartCh, "char", 4)) {
                    tk = addTk(CHAR);
                } else {
                    tk = addTk(ID);
                    tk->text = createString(pStartCh, pCrtCh);
                }
                return tk->code;

                case 3:  // Integer literal processing
                if (ch >= '0' && ch <= '9') {
                    pCrtCh++;  // Keep processing digits
                } else {
                    state = 4;
                }
                break;

            case 4:
                addTk(CT_INT);  // Complete integer token
                return CT_INT;

            case 5:
                if (ch >= 0 && ch <= 7) {
                    pCrtCh++;  // Keep processing octal digits
                    state = 4;
                } else if (ch == 'x' || ch == 'X') {
                    pCrtCh++;
                    state = 6;  // Move to hexadecimal state
                } else {
                    tkerr(addTk(END), "invalid character in octal literal");

                }
            break;

            case 6:  // Hexadecimal literal processing
            if (ch >= '0' || ch <= '9' || (ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F')) {
                pCrtCh++;
                state = 4;  // Move to hexadecimal state
            } else {
                tkerr(addTk(END), "invalid character in octal literal");

            }
            break;
        }
    }
}



// Show all tokens
void showTokens()
{
    for (Token *tk = tokens; tk; tk = tk->next)
    {
        printf("%d", tk->code);
        if (tk->code == ID)
            printf(":%s", tk->text);
        printf(" ");
    }
    printf("\n");
}

// Free all tokens
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

// Read file content into memory
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

// Set input for lexer
void setInput(const char *input)
{
    pCrtCh = input;
}
