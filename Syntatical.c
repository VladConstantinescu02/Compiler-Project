#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lexer.h"

typedef struct Token {
    int code;
    const char *text;
    struct Token *next;
} Token;

// Tokenizer variables
const char *pCrtCh;
Token *tokens = NULL, *lastToken = NULL;
int line = 1;

void addToken(int code, const char *start, const char *end) {
    Token *tk = (Token *)malloc(sizeof(Token));
    tk->code = code;
    tk->text = strndup(start, end - start);
    tk->next = NULL;
    if (lastToken) lastToken->next = tk;
    else tokens = tk;
    lastToken = tk;
}

void tokenizer() {
    while (*pCrtCh) {
        if (isalpha(*pCrtCh) || *pCrtCh == '_') {
            const char *start = pCrtCh++;
            while (isalnum(*pCrtCh) || *pCrtCh == '_') pCrtCh++;
            addToken(ID, start, pCrtCh);
        } else if (isdigit(*pCrtCh)) {
            const char *start = pCrtCh++;
            while (isdigit(*pCrtCh)) pCrtCh++;
            addToken(CT_INT, start, pCrtCh);
        } else if (*pCrtCh == '+') {
            addToken(ADD, pCrtCh, ++pCrtCh);
        } else if (*pCrtCh == '-') {
            addToken(SUB, pCrtCh, ++pCrtCh);
        } else if (*pCrtCh == '*') {
            addToken(MUL, pCrtCh, ++pCrtCh);
        } else if (*pCrtCh == '/') {
            addToken(DIV, pCrtCh, ++pCrtCh);
        } else if (*pCrtCh == '(') {
            addToken(LPAR, pCrtCh, ++pCrtCh);
        } else if (*pCrtCh == ')') {
            addToken(RPAR, pCrtCh, ++pCrtCh);
        } else if (isspace(*pCrtCh)) {
            if (*pCrtCh == '\n') line++;
            pCrtCh++;
        } else {
            fprintf(stderr, "Unexpected character '%c' at line %d\n", *pCrtCh, line);
            pCrtCh++;
        }
    }
    addToken(END, pCrtCh, pCrtCh);
}

// Recursive Descent Parser functions
Token *currentToken;

void consume(int code) {
    if (currentToken->code == code) {
        currentToken = currentToken->next;
    } else {
        fprintf(stderr, "Expected token %d, found %d\n", code, currentToken->code);
        exit(1);
    }
}

// Grammar: Expr ::= Term Expr'
void expr();

// Grammar: Term ::= Factor Term'
void term();

// Grammar: Factor ::= ID | CT_INT | '(' Expr ')'
void factor() {
    if (currentToken->code == ID || currentToken->code == CT_INT) {
        consume(currentToken->code);
    } else if (currentToken->code == LPAR) {
        consume(LPAR);
        expr();
        consume(RPAR);
    } else {
        fprintf(stderr, "Unexpected token in factor at line %d\n", line);
        exit(1);
    }
}

// Grammar: Term' ::= ('*' | '/') Factor Term' | ε
void termPrime() {
    if (currentToken->code == MUL || currentToken->code == DIV) {
        consume(currentToken->code);
        factor();
        termPrime();
    }
}

// Grammar: Term ::= Factor Term'
void term() {
    factor();
    termPrime();
}

// Grammar: Expr' ::= ('+' | '-') Term Expr' | ε
void exprPrime() {
    if (currentToken->code == ADD || currentToken->code == SUB) {
        consume(currentToken->code);
        term();
        exprPrime();
    }
}

// Grammar: Expr ::= Term Expr'
void expr() {
    term();
    exprPrime();
}

int main() {
    // Input program for testing
    const char *program = "a + b * (c - d) / e";
    pCrtCh = program;

    // Tokenize input
    tokenizer();
    currentToken = tokens;

    // Parse input
    expr();

    if (currentToken->code == END) {
        printf("Parsing completed successfully.\n");
    } else {
        fprintf(stderr, "Parsing failed. Unexpected token at end.\n");
    }

    return 0;
}
