#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include "basis.h"

int unit;
int decl_struct;
int decl_var;
int type_base;
int array_decl;
int type_name;
int decl_func;
int func_arg;
int stm;
int stm_compound;
int expr;
int expr_assign;
int expr_or;
int expr_or1;
int expr_and;
int expr_and1;
int expr_eq;
int expr_eq1;
int expr_rel;
int expr_rel1;
int expr_add;
int expr_mul;
int expr_cast;
int expr_unary;
int expr_postfix;
int expr_postfix1;
int expr_primary;


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

// exprPrimary: ID ( LPAR ( expr ( COMMA expr )* )? RPAR )?
int exprPrimary() {
    Token *startTk = crtTk;

    if (consume(ID)) {
        if (consume(LPAR)) {
            if (exprPrimary()) {
                while (consume(COMMA)) {
                    if (!exprPrimary()) tkerr(crtTk, "missing expression after comma");
                }
            }
            if (!consume(RPAR)) tkerr(crtTk, "missing ) after function call");
        }
        return 1;
    }

        // Case: CT_INT
        if (consume(CT_INT)) return 1;

        // Case: CT_REAL
        if (consume(CT_REAL)) return 1;
    
        // Case: CT_CHAR
        if (consume(CT_CHAR)) return 1;
    
        // Case: CT_STRING
        if (consume(CT_STRING)) return 1;
    
        // Case: LPAR expr RPAR
        if (consume(LPAR)) {
            if (exprPrimary()) {
                if (consume(RPAR)) return 1;
                else tkerr(crtTk, "missing ) after ( expr");
            } else tkerr(crtTk, "invalid expression after (");
        }
    
        crtTk = startTk;  // backtrack on failure
        return 0;
}

// typeBase: INT | DOUBLE | CHAR | STRUCT ID ;
int typeBase() {
    Token *startTk = crtTk;
    if (consume(INT)) return 1;
    if (consume(CHAR)) return 1;
    if (consume(STRUCT)) {
        if (consume (ID)) {
            decl_struct = 1;
            return 1;
        } else {
            tkerr(crtTk, "invalid type declaration");
        }
    }
    crtTk = startTk;  // backtrack on failure
    return 0;
}

// declVar:  typeBase ID arrayDecl? ( COMMA ID arrayDecl? )* SEMICOLON ;
int declVar() {
    Token *startTk = crtTk;

    if (typeBase()) {
        if (consume(ID)) {
            array_decl = 0;

            while (consume(COMMA)) 
            {
                if (!consume(ID)) {
                    tkerr(crtTk, "missing ID after comma");
                    array_decl = 1;
                }
            }

            if (!consume(SEMICOLON)) {
                tkerr(crtTk, "missing ; after variable declaration");
                decl_var = 1;
                return 1;
            } else {
                tkerr(crtTk, "missing ID after type");
            }
            
        }
    }
    crtTk = startTk;
    return 0; 
}

// declStruct: STRUCT ID LACC declVar* RACC SEMICOLON ;
int declStruct() {
    Token *startTk = crtTk;

    if (consume(STRUCT)) {
        if (consume(ID)) {
            if (consume(LACC)) {
                while (declVar()) {
                    // Keep consuming declVars
                }

                if (consume(RACC)) {
                    if (consume(SEMICOLON)) {
                        decl_struct = 1;
                        return 1;
                    } else {
                        tkerr(crtTk, "missing ; after struct declaration");
                    }
                } else {
                    tkerr(crtTk, "missing } after struct body");
                }
            } else {
                tkerr(crtTk, "missing { to start struct body");
            }
        } else {
            tkerr(crtTk, "missing struct name");
        }
    }

    crtTk = startTk;
    return 0;
}








void runSyntacticalFromTokens() {
    crtTk = tokens;



    showTokens();
    done();
}
