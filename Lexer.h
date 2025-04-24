#ifndef LEXER_H
#define LEXER_H

// Token codes
enum {
    ID, CT_INT, CT_REAL, CT_CHAR, CT_STRING, COMMA, SEMICOLON, LPAR, RPAR, LBRACKET, RBRACKET,
    LACC, RACC, ADD, SUB, MUL, DIV, DOT, AND, OR, EQUAL, ASSIGN, NOTEQ, NOT, LESSEQ, LESS,
    GREATEREQ, GREATER, SPACE, BREAK, CHAR, END, DOUBLE, ELSE, FOR, IF, INT, RETURN, STRUCT,
    VOID, WHILE
};


// Token structure
typedef struct _Token {
    int code;
    union {
        char *text;
        long int i;
        double r;
    };
    int line;
    struct _Token *next;
} Token;

// Functions
Token *addTk(int code);
char *createString(const char *start, const char *end);
int getNextToken();
void showTokens();
void done();
char *readFileContent(const char *fileName);
void setInput(const char *input);

#endif // LEXER_H
