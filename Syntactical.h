#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include "basis.h"


int expr();
int exprAssign();
int exprOr();
int exprAnd();
int exprRel();
int exprAdd();
int exprMul();
int exprCast();
int exprUnary();
int exprPostfix();
int exprPrimary();
int typeName();
int arrayDecl();
int typeBase();
int funcArg();
int declVar();
int declStruct();
int declFunc();
int stmCompound();
int stm();
int unit();

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

int exprPrimary() {
    Token *startTk = crtTk;

    if (consume(ID)) {
        if (consume(LPAR)) {
            if (!consume(RPAR)) {
                if (!expr()) {
                    tkerr(crtTk, "missing expression in function call");
                }

                while (consume(COMMA)) {
                    if (!expr()) tkerr(crtTk, "missing expression after comma");
                }
                if (!consume(RPAR)) {
                    tkerr(crtTk, "missing ) after function call");
                }
            }
        }
        return 1;
    }

    if (consume(CT_INT) || consume(CT_REAL) || consume(CT_CHAR) || consume(CT_STRING)) {
        return 1;
    }

    if (consume(LPAR)) {
        if (!expr()) {
            tkerr(crtTk, "missing expression inside ( )"); 
        }
        if (!consume(RPAR)) { 
            tkerr(crtTk, "missing ) after expression");
        }
        return 1;
    }

    crtTk = startTk;
    return 0;
}

int exprPostfix() {
    if (!exprPrimary()) {
        return 0;
    }
    while (1) {
        if (consume(LBRACKET)) {
            if (!expr()) {
                tkerr(crtTk, "missing expression in exprPostFix"); 
            }
            if (!consume(RBRACKET)) {
                tkerr(crtTk, "missing ] after array index"); 
            }
        } else if (consume(DOT)) {
            if (!consume(ID)) tkerr(crtTk, "missing field name after .");
        } else break;
    }
    return 1;
}

int exprUnary() {
    if (consume(SUB) || consume(NOT)) {
        if (!exprUnary()) {
            tkerr(crtTk, "missing operand for unary operator");
        }
        return 1;
    }
    return exprPostfix();
}

int exprCast() {
    Token *startTk = crtTk;
    if (consume(LPAR)) {
        if (typeName()) {
            if (!consume(RPAR)) {
                tkerr(crtTk, "missing ) after type name"); 
            }
            if (!exprCast()) {
                tkerr(crtTk, "missing expression after cast"); 
            }
            return 1;
        }
        crtTk = startTk;
    }
    return exprUnary();
}

int exprMul() {
    if (!exprCast()) {
        return 0; 
    }

    while (consume(MUL) || consume(DIV)) {
        if (!exprCast()) {
             tkerr(crtTk, "missing right-hand operand for * or /");
        }
    }
    return 1;
}

int exprAdd() {
    if (!exprMul()) {
        return 0;
    }

    while (consume(ADD) || consume(SUB)) {
        if (!exprMul()) {
            tkerr(crtTk, "missing right-hand operand for + or -");
        }
    }
    return 1;
}

int exprRel() {
    if (!exprAdd()) {
        return 0;
    }

    while (consume(LESS) || consume(LESSEQ) || consume(GREATER) || consume(GREATEREQ)) {
        if (!exprAdd()) tkerr(crtTk, "missing right-hand operand for relational operator");
    }
    return 1;
}

int exprEq() {
    if (!exprRel()) {
        return 0;
    }

    while (consume(EQUAL) || consume(NOTEQ)) {
        if (!exprRel()) {
            tkerr(crtTk, "missing right-hand operand for == or !=");
        }
    }
    return 1;
}

int exprAnd() {
    if (!exprEq()) {
        return 0;
    }

    while (consume(AND)) {
        if (!exprEq()) tkerr(crtTk, "missing right-hand operand for &&");
    }
    return 1;
}

int exprOr() {
    if (!exprAnd()) {
        return 0;
    }

    while (consume(OR)) {
        if (!exprAnd()) {
            tkerr(crtTk, "missing right-hand operand for ||");
        }
    }
    return 1;
}

int exprAssign() {
    Token *startTk = crtTk;

    if (exprUnary()) {
        if (consume(ASSIGN)) {
            if (!exprAssign()) {
                tkerr(crtTk, "Expected expression after =");
                crtTk = startTk;
                return 0;
            }
            return 1;
        }
        crtTk = startTk;
    }

    if (exprOr()) {
        return 1;
    }

    crtTk = startTk;
    return 0;
}

int expr() {
    return exprAssign();
}

int stm() {
    if (stmCompound()) {
         return 1;
    }

    if (consume(IF)) {
        if (!consume(LPAR)) {
            tkerr(crtTk, "missing ( after if");
        }
        if (!expr()) {
            tkerr(crtTk, "missing condition in if");
        }
        if (!consume(RPAR)) {
            tkerr(crtTk, "missing ) after if condition");
        }
        if (!stm()) {
            tkerr(crtTk, "missing statement after if");
        }
        if (consume(ELSE) && !stm()) {
            tkerr(crtTk, "missing statement after else");
        }
        return 1;
    }
    if (consume(WHILE)) {
        if (!consume(LPAR)) {
            tkerr(crtTk, "missing ( after while");
        }
        if (!expr()) {
            tkerr(crtTk, "missing condition in while");
        }
        if (!consume(RPAR)) {
            tkerr(crtTk, "missing ) after while condition");
        }
        if (!stm()) {
            tkerr(crtTk, "missing statement after while");
        }
        return 1;
    }
    if (consume(FOR)) {
    if (!consume(LPAR)) {
        tkerr(crtTk, "missing ( after for");
    }
    expr();
    if (!consume(SEMICOLON)) {
        tkerr(crtTk, "missing ; after initialization in for");
    }
    expr(); 
    if (!consume(SEMICOLON)) {
        tkerr(crtTk, "missing ; after condition in for");
    }
    expr(); 
    if (!consume(RPAR)) {
        tkerr(crtTk, "missing ) in for");
    }
    if (!stm()) {
        tkerr(crtTk, "missing statement after for");
    }
    return 1;
}

    if (consume(BREAK)) {
        if (!consume(SEMICOLON)) {
            tkerr(crtTk, "missing ; after break");
        }
        return 1;
    }
    if (consume(RETURN)) {
        expr();
        if (!consume(SEMICOLON)) {
            tkerr(crtTk, "missing ; after return");
        }
        return 1;
    }
    if (expr()) {
        if (!consume(SEMICOLON)) {
            tkerr(crtTk, "missing ; after expression");
        }
        return 1;
    }
    if (consume(SEMICOLON)) return 1;

    return 0;
}

int stmCompound() {
    if (!consume(LACC)) {
        return 0;
    }
    while (declVar() || stm());
    if (!consume(RACC)) tkerr(crtTk, "missing } after compound statement");
    return 1;
}

int declVar() {
    Token *startTk = crtTk;

    if (!typeBase()) {
        return 0;
    }

    if (!consume(ID)) {
        tkerr(crtTk, "missing ID after type");
    }
    arrayDecl();
    while (consume(COMMA)) {
        if (!consume(ID)) {
            tkerr(crtTk, "missing ID after ,");
        }
        arrayDecl();
    }
    if (!consume(SEMICOLON)) {
        tkerr(crtTk, "missing ; after variable declaration");
    }
    return 1;
}

int declStruct() {
    if (!consume(STRUCT)) {
        return 0;
    }

    if (!consume(ID)) {
        tkerr(crtTk, "missing struct name");
    }
    if (!consume(LACC)) {
        tkerr(crtTk, "missing { after struct name");
    }

    while (declVar());
    if (!consume(RACC)) tkerr(crtTk, "missing } in struct");
    if (!consume(SEMICOLON)) tkerr(crtTk, "missing ; after struct");
    return 1;
}

int declFunc() {
    if (!(typeBase() || consume(VOID))) {
        return 0;
    }
    consume(MUL);

    if (!consume(ID)) {
        tkerr(crtTk, "missing function name");
    }

    if (!consume(LPAR)) {
        tkerr(crtTk, "missing ( in function");
    }

    if (funcArg()) {
        while (consume(COMMA)) {
            if (!funcArg()) {
                tkerr(crtTk, "missing function argument");
            }
        }
    }
    if (!consume(RPAR)) {
        tkerr(crtTk, "missing ) in function");
    }
    if (!stmCompound()) {
        tkerr(crtTk, "missing function body");
    }
    return 1;
}

int typeBase() {
    if (consume(INT) || consume(DOUBLE) || consume(CHAR)) {
         return 1;
    }
    
    if (consume(STRUCT)) {
        if (!consume(ID)) {
            tkerr(crtTk, "missing ID after struct");
        }
        return 1;
    }
    return 0;
}

int arrayDecl() {
    if (consume(LBRACKET)) {
        expr();
        if (!consume(RBRACKET)) {
            tkerr(crtTk, "missing ] in array declaration");
        }
        return 1;
    }
    return 0;
}

int typeName() {
    if (!typeBase()) {
        return 0;
    }
    arrayDecl();
    return 1;
}

int funcArg() {
    if (!typeBase()) {
        return 0;
    }

    if (!consume(ID)) {
        tkerr(crtTk, "missing parameter name");
    }
    arrayDecl();
    return 1;
}

int unit() {
    while (declStruct() || declFunc() || declVar());
    if (!consume(END)) tkerr(crtTk, "missing END token");
    return 1;
}

void runSyntacticalFromTokens() {
crtTk = tokens;


if (unit()) {
    printf("Parsing completed successfully.\n\n");
} else {
    tkerr(crtTk, "Syntax error during parsing.");
}

showTokens();
done();

}
