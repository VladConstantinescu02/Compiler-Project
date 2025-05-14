#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include "basis.h"

int unit_flag;
int decl_struct_flag;
int decl_var_flag;
int type_base_flag;
int array_decl_flag;
int type_name_flag;
int decl_func_flag;
int func_arg_flag;
int stm_flag;
int stm_compound_flag;
int expr_flag;
int expr_assign_flag;
int expr_or_flag;
int expr_or1_flag;
int expr_and_flag;
int expr_and1_flag;
int expr_eq_flag;
int expr_rel_flag;
int expr_add_flag;
int expr_mul_flag;
int expr_cast_flag;
int expr_unary_flag;
int expr_postfix_flag;
int expr_primary_flag;

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

//expr: exprAssign ;
int expr() {
    Token *startTk = crtTk;
    if (exprAssign()) {
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

        
        if (consume(CT_INT)) {
            return 1;
        }
        
        if (consume(CT_REAL)) {
            return 1;
        }
    
        if (consume(CT_CHAR)) {
            return 1;
        }
    
        if (consume(CT_STRING)) {
            return 1;
        }
    
        if (consume(LPAR)) {
            if (exprPrimary()) {
                if (consume(RPAR)) return 1;
                else tkerr(crtTk, "missing ) after ( expr");
            } else tkerr(crtTk, "invalid expression after (");
        }
    
        crtTk = startTk;  
        return 0;
}

/*exprPostfix: exprPostfix LBRACKET expr RBRACKET
           | exprPostfix DOT ID 
           | exprPrimary ;
*/
int exprPostfix() {
    Token *startTk = crtTk;

    if (!exprPrimary()) {
        crtTk = startTk;
        return 0;
    }


    while (1) {
        if (consume(LBRACKET)) {
            if (!expr()) tkerr(crtTk, "missing expression inside [ ]");
            if (!consume(RBRACKET)) tkerr(crtTk, "missing ]");
        } else if (consume(DOT)) {
            if (!consume(ID)) tkerr(crtTk, "missing ID after .");
        } else {
            break;  
        }
    }

    expr_postfix_flag = 1;
    return 1;
}

//exprUnary: ( SUB | NOT ) exprUnary | exprPostfix ;
int exprUnary() {
    Token *startTk = crtTk;

    if (consume(SUB) || consume(NOT)) {
        if(!exprUnary())
        tkerr(crtTk,"Missing operator after unary operator");
        expr_unary_flag = 1;
        return 1;
    }

    if(exprPostfix()) {
        expr_unary_flag = 1;
        return 1;
    }

    crtTk = startTk;
    return 0; 
}

//exprCast: LPAR typeName RPAR exprCast | exprUnary ;
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
            expr_cast_flag = 1;
            return 1;  
        }
    }

    crtTk = startTk;
    return 0; 
}

    
//exprMul: exprMul ( MUL | DIV ) exprCast | exprCast ;
int exprMul() {
    Token *startTk = crtTk;

    if (!exprCast()) {
        crtTk = startTk;
        return 0;
    }

    while (consume(MUL) || consume(DIV)) {
        if (!exprCast()) {
            tkerr(crtTk, "missing right-hand side after * or /");
        }
    }

    expr_mul_flag = 1;
    return 1;

    crtTk = startTk;
    return 0;
}

//exprAdd: exprAdd ( ADD | SUB ) exprMul | exprMul ;
int exprAdd() {
    Token *startTk = crtTk;

    if (!exprMul()) {
        crtTk = startTk;
        return 0;
    }

    while (consume(ADD) || consume(SUB)) {
        if (!exprMul()) {
            tkerr(crtTk, "missing right-hand side after + or -");
        }
    }

    expr_add_flag = 1;
    return 1;

    crtTk = startTk;
    return 0;
}

//exprRel: exprRel ( LESS | LESSEQ | GREATER | GREATEREQ ) exprAdd | exprAdd ;
int exprRel() {
    Token *startTk = crtTk;

    if (!exprAdd()) {
        crtTk = startTk;
        return 0;
    }

    while (consume(LESS) || consume(LESSEQ) || consume(GREATER) || consume(GREATEREQ)) {
        if (!exprAdd()) {
            tkerr(crtTk, "missing right-hand side after <, <=, >, or >=");
        }
    }

    expr_rel_flag = 1;
    return 1;

    crtTk = startTk;
    return 0;
}

//exprEq: exprEq ( EQUAL | NOTEQ ) exprRel | exprRel ;
int exprEq() {
    Token *startTk = crtTk;

    if (!exprRel()) {
        crtTk = startTk;
        return 0;
    }

    while (consume(EQUAL) || consume(NOTEQ)) {
        if (!exprRel()) {
            tkerr(crtTk, "missing right-hand side after == or !=");
        }
    }

    expr_eq_flag = 1;
    return 1;

    crtTk = startTk;
    return 0;
}

//exprOr: exprOr OR exprAnd | exprAnd ;
int exprOr() {
    Token *startTk = crtTk;

    if (!exprAnd()) {
        crtTk = startTk;
        return 0;
    }

    while (consume(OR)) {
        if (!exprAnd()) {
            tkerr(crtTk, "missing right-hand side after ||");
        }
    }

    expr_or_flag = 1;
    return 1;

    crtTk = startTk;
    return 0;
}


//exprAnd: exprAnd AND exprEq | exprEq ;
int exprAnd() {
    Token *startTk = crtTk;

    if (!exprEq()) {
        crtTk = startTk;
        return 0;
    }

    while (consume(AND)) {
        if (!exprEq()) {
            tkerr(crtTk, "missing right-hand side after &&");
        }
    }

    expr_and_flag = 1;
    return 1;

    crtTk = startTk;
    return 0;
}

//exprAssign: exprUnary ASSIGN exprAssign | exprOr ;
int exprAssign() {
    Token *startTk = crtTk;

    if(!exprUnary()) {
        crtTk = startTk;
        return 0;
    }

    while (consume(ASSIGN)) {
        if (!exprAssign()) {
            tkerr(crtTk, "missing right-hand side after =");
        }
    }

    crtTk = startTk;
    return 0;
}    




// typeBase: INT | DOUBLE | CHAR | STRUCT ID ;
int typeBase() {
    Token *startTk = crtTk;
    if (consume(INT)) return 1;
    if (consume(CHAR)) return 1;
    if (consume(STRUCT)) {
        if (consume (ID)) {
            decl_struct_flag = 1;
            return 1;
        } else {
            tkerr(crtTk, "invalid type declaration");
        }
    }
    crtTk = startTk;  
    return 0;
}

//typeName: typeBase arrayDecl? 
int typeName() {
    Token *startTk = crtTk;

    if (typeBase()) {
        arrayDecl();
        type_name_flag = 1;
        return 1;
    } else {
        tkerr(crtTk, "missing or invalid type name");
    }

    crtTk = startTk;  
    return 0;
}



//arrayDecl: LBRACKET expr? RBRACKET ;
int arrayDecl() {
    Token *startTk = crtTk;

    if (consume(LBRACKET)) {
        expr();  
        if (consume(RBRACKET)) {
            array_decl_flag = 1;
            return 1;
        } else {
            tkerr(crtTk, "missing ] in array declaration");
        }
    }

    crtTk = startTk;
    return 0;
}

//funcArg: typeBase ID arrayDecl? ;
int funcArg() {
    Token *startTk = crtTk;

    if (typeBase()) {
        if (consume(ID)) {
            arrayDecl();
            func_arg_flag = 1;
            return 1;
        } else {
             tkerr(crtTk, "missing ID in function argument");
        }
    }

    crtTk = startTk;
    return 0;
}

/*stm: stmCompound 
           | IF LPAR expr RPAR stm ( ELSE stm )?
           | WHILE LPAR expr RPAR stm
           | FOR LPAR expr? SEMICOLON expr? SEMICOLON expr? RPAR stm
           | BREAK SEMICOLON
           | RETURN expr? SEMICOLON
           | expr? SEMICOLON ;*/
int stm() {
    Token *startTk = crtTk;

    if (stmCompound()) {
        stm_flag = 1;
        return 1;
    }

    if (consume(IF)) {
        if(!consume(LPAR)) {
            tkerr(crtTk, "missing ( after if");
        }
        if (!expr()) {
            tkerr(crtTk, "missing expression after ( in if");
        }
        if (!consume(RPAR)) {
            tkerr(crtTk, "missing ) after expression in if");
        }
        if (!stm()) {
            tkerr(crtTk, "missing statement after if");
        }
        if (consume(ELSE)) {
            if (!stm()) {
                tkerr(crtTk, "missing statement after else");
            }
        }
        return 1;
    }

    if (consume(WHILE)) {
        if (!consume(LPAR)) {
            tkerr(crtTk, "missing ( after while");
        }
        if (!expr()) {
            tkerr(crtTk, "missing expression after ( in while");
        }
        if (!consume(RPAR)) {
            tkerr(crtTk, "missing ) after expression in while");
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
            tkerr(crtTk, "missing ; after for expression");
        }
        expr();
        if (!consume(SEMICOLON)) {
            tkerr(crtTk, "missing ; after for expression");
        }
        expr();
        if (!consume(RPAR)) {
            tkerr(crtTk, "missing ) after for expression");
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
    } else if (consume(SEMICOLON)) {
        tkerr(crtTk, "Empty statement");
        return 1;
    }


    crtTk = startTk;
    return 0;
}

//stmCompound: LACC ( declVar | stm )* RACC ;
int stmCompound() {
    Token *startTk = crtTk;

    if (!consume(LACC)) {  
        crtTk = startTk;
        return 0;
    }

    while (1) {
        if (declVar()) {
            decl_var_flag = 1;
        } else if (stm()) {
            stm_flag = 1;
        } else {
            break;  
        }
    }

    if (!consume(RACC)) {  
        tkerr(crtTk, "missing } after compound statement");
        crtTk = startTk;
        return 0;
    }

    stm_compound_flag = 1;
    return 1;
}


// declVar:  typeBase ID arrayDecl? ( COMMA ID arrayDecl? )* SEMICOLON ;
int declVar() {
    Token *startTk = crtTk;

    if (typeBase()) {
        if (consume(ID)) {
            arrayDecl();

            while (consume(COMMA)) 
            {
                if (!consume(ID)) {
                    tkerr(crtTk, "missing ID after comma");
                    array_decl_flag = 1;
                }
            }

            if (!consume(SEMICOLON)) {
                tkerr(crtTk, "missing ; after variable declaration");
                decl_var_flag = 1;
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
                while (1) {
                    if (!declVar()) {}
                    else break;
                }

                if (consume(RACC)) {
                    if (consume(SEMICOLON)) {
                        decl_struct_flag = 1;
                        return 1;
                    } else {
                        tkerr(crtTk, "missing ; after struct declaration");
                    }
                } else {
                    tkerr(crtTk, "missing } after struct declaration");
                }
            } else {
                tkerr(crtTk, "missing { after struct declaration");
            }
        } else {
            tkerr(crtTk, "missing ID after struct");
        }
    }

    crtTk = startTk;
    return 0;
}

/*declFunc: ( typeBase MUL? | VOID ) ID 
                        LPAR ( funcArg ( COMMA funcArg )* )? RPAR 
                        stmCompound ;*/

int declFunc() {
    Token *startTk = crtTk;

    if (typeBase() || consume(VOID)) { 
        consume(MUL); 
    } else {
        tkerr(crtTk, "missing type or void in function declaration");
        crtTk = startTk;
        return 0;
    }

    if (!consume(ID)) {
        tkerr(crtTk, "missing ID in function declaration");
        crtTk = startTk;
        return 0;
    }

    if (!consume(LPAR)) {
        tkerr(crtTk, "missing ( in function declaration");
        crtTk = startTk;
        return 0;
    }

    if (funcArg()) {
        while (consume(COMMA)) {
            if (!funcArg()) {
                tkerr(crtTk, "missing function argument after comma");
            }
        }
    }

    if (!consume(RPAR)) {
        tkerr(crtTk, "missing ) in function declaration");
        crtTk = startTk;
        return 0;
    }

    if (!stmCompound()) {
        tkerr(crtTk, "missing function body");
        crtTk = startTk;
        return 0;
    }

    decl_func_flag = 1;
    return 1;
}


//unit: ( declStruct | declFunc | declVar )* END ;
int unit() {
    Token *startTk = crtTk;

    while (1) {
        if (declStruct()) {
            decl_struct_flag = 1;
        } else if (declFunc()) {
            decl_func_flag = 1;
        } else if (declVar()) {
            decl_var_flag = 1;
        } else {
            break;
        }
    }

    if (!consume(END)) {
        tkerr(crtTk, "missing END");
        crtTk = startTk;
        return 0;
    }

    unit_flag = 1;
    return 1;

    crtTk = startTk;
    return 0;
}


void runSyntacticalFromTokens() {
    crtTk = tokens; 

    if (unit()) {
        printf("Parsing completed successfully.\n");
    } else {
        tkerr(crtTk, "Syntax error during parsing.");
    }

    showTokens();
    done();
}
