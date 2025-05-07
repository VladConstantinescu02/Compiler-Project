#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include "basis.h"

Token *crtTk;
Token *consumedTk;

int consume(int code) {
    if (crtTk && crtTk->code == code) {
        consumedTk = crtTk;
        crtTk = crtTk->next;
        return 1;
    }
    return 0;
}

int expr() {
    Token *startTk = crtTk;

    if (consume(ID)) {
        return 1;
    }
    if (consume(CT_INT)) {
        return 1;
    }
    if (consume(LPAR)) {
        if (expr()) {
            if (consume(RPAR)) {
                return 1;
            } else {
                tkerr(crtTk, "missing )");
            }
        } else {
            tkerr(crtTk, "invalid expression after (");
        }
        crtTk = startTk; 
    }
    return 0;
}


void runSyntacticalFromTokens() {
    crtTk = tokens;

    if (expr()) {
        printf("Syntax OK\n");
    } else {
        tkerr(crtTk, "Syntax error in input");
    }

    showTokens();
    done();
}
