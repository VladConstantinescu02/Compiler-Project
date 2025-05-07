#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include "basis.h"


Token *crtTk;
Token *consumedTk;

int expr();
int exprAssign();
int exprOr();
int exprAnd();
int exprEq();
int exprRel();
int exprAdd();
int exprMul();
int exprUnary();
int exprPostfix();
int exprPrimary();
int typeBase();
int arrayDecl();
int typeName();
int varDecl();
int fnParam();
int fnParamList();
int fnDecl();
int stm();
int stmCompound();
int stmIf();
int stmWhile();
int stmFor();
int stmReturn();
int declStruct();
int decl();
int unit();



// --- Consume Function ---
int consume(int code) {
    if (crtTk && crtTk->code == code) {
        consumedTk = crtTk;
        crtTk = crtTk->next;
        return 1;
    }
    return 0;
}

// --- Grammar Rule: expr ::= ID | CT_INT ---
int expr() {
    if (consume(ID)) return 1;
    if (consume(CT_INT)) return 1;
    return 0;
}

int runParser(const char *filename) {
    // Load source and run lexer
    const char *input = readFileContent(filename);
    setInput(input);

    while (getNextToken() != END); // tokenize all input
    crtTk = tokens;                // set parser to start

    // Call start rule
    if (unit()) {
        printf("Syntax OK\n");
    } else {
        tkerr(crtTk, "Syntax error in program.");
    }

    // Debug print (optional)
    showTokens();

    // Cleanup
    done();
    return 0;
}
