//
//  ox/oxeval.c
//
//  Created by Michael Henderson on 10/6/14.
//  Copyright (c) 2014 Michael D Henderson. All rights reserved.
//

#include "oxalloc.h"
#include "oxeval.h"
#include "oxinit.h"
#include "oxutil.h"

#include <stdio.h>

static oxcell *oxeval_atom(oxcell *atom, oxcell *env);

oxcell *oxeval(oxcell *expr, oxcell *env) {
    if (isatom(expr)) {
        return oxeval_atom(expr, env);
    }
    
    oxcell *symbol = oxcell_get_car(expr);
    oxcell *args   = oxcell_get_cdr(expr);
    
    if (issymbol(symbol)) {
        printf(".eval: %s, how cute\n", oxcell_get_text(oxcell_get_name(symbol)));
        // do we need to lookup the symbol?
        //
        if (!symbol->cons.cdr) {
            oxcell *lookup = oxsym_lookup(oxcell_get_name(symbol), env);
            if (!lookup || lookup == nill) {
                printf("error: undefined symbol '%s'\n", oxcell_get_text(oxcell_get_name(symbol)));
                exit(2);
            }
            symbol = lookup;
        }
    } else if (iscons(symbol)) {
        // symbol will be the result of evaluating the list
        //
        symbol = oxeval(symbol, env);
        if (!issymbol(symbol)) {
            // the expression didn't return something that we can execute
            //
        }
    }
    
    // symbol should either be a built-in or a lamba
    //
    oxcell *l = oxcell_get_cdr(symbol);
    if (!isatom(l)) {
        printf("error: can't execute a list\n");
        exit(2);
    }
    
    if (l->atom->kind == eAtomFunc) {
        printf(".info: executing builtin\n");
        return l->atom->func(args, env);
    }
    
    if (1) {
        printf("error: don't know how to execute symbol\n");
        exit(2);
    }
    
    return nill;
}

static oxcell *oxeval_atom(oxcell *c, oxcell *env) {
    if (isnil(c)) {
        return nill;
    } else if (isatom(c)) {
        return c;
    }
    return nill;
}

