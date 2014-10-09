//
//  ox/oxprint.c
//
//  Created by Michael Henderson on 10/6/14.
//  Copyright (c) 2014 Michael D Henderson. All rights reserved.
//
#if 0
#include "oxtype.h"
#include "oxprint.h"
#include "oxutil.h"

#include <stdio.h>

void oxcell_dump(oxcell *c, int level) {
    level++;
    if (!c) {
        printf(".dump: null%*cnull\n", level, ' ');
        return;
    }

    if (isnil(c->first) && isnil(c->rest)) {
        printf(".dump: .... %p %2d %2d, %-11s %-11s %p\n", c, c->kind, c->isSymbol, "nil", "nil", c->atom);
    } else if (isnil(c->first)) {
        printf(".dump: .... %p %2d %2d, %-11s %11p %p\n", c, c->kind, c->isSymbol, "nil", c->rest, c->atom);
    } else if (isnil(c->first)) {
        printf(".dump: .... %p %2d %2d, %11p %-11s %p\n", c, c->kind, c->isSymbol, c->first, "nil", c->atom);
    } else {
        printf(".dump: .... %p %2d %2d, %11p %11p %p\n", c, c->kind, c->isSymbol, c->first, c->rest, c->atom);
    }

    if (isnil(c)) {
        printf(".dump: nil  %*c()\n", level, ' ');
        return;
    }
    
    if (isatom(c)) {
        printf(".dump: atom %*c()\n", level, ' ');
        switch (c->atom->kind) {
            case eAtomFunc:
                printf(".dump: atom %*c#func# %p\n", level, ' ', c->atom->func);
                return;
            case eAtomInteger:
                printf(".dump: atom %*cint    %ld\n", level, ' ', oxcell_get_integer(c));
                return;
            case eAtomReal:
                printf(".dump: atom %*creal   %g\n", level, ' ', oxcell_get_real(c));
                return;
            case eAtomText:
                printf(".dump: atom %*ctext   %s\n", level, ' ', oxcell_get_text(c));
                return;
            case eAtomTime:
                printf(".dump: atom %*c#time#\n", level, ' ');
                return;
        }
    }

    printf(".dump: %-4s %*c(\n", issymbol(c) ? "sym" : "list", level, ' ');

    oxcell_dump(oxcell_get_first(c), level);
    c = oxcell_get_rest(c);
    while (!isnil(c) && islist(c)) {
        oxcell_dump(oxcell_get_first(c), level);
        c = oxcell_get_rest(c);
    }
    printf(".dump: %-4s %*c)\n", issymbol(c) ? "sym" : "list", level, ' ');

    return;
}

void oxcell_print(oxcell *c) {
    if (!c) {
        // do nothing?
    } else if (isnil(c)) {
        printf("()");
    } else if (isatom(c)) {
        switch (c->atom->kind) {
            case eAtomFunc:
                printf("#CFUNC");
                return;
            case eAtomInteger:
                printf("%ld", oxcell_get_integer(c));
                return;
            case eAtomReal:
                printf("%g", oxcell_get_real(c));
                return;
            case eAtomText:
                printf("\"%s\"", oxcell_get_text(c));
                return;
            case eAtomTime:
                printf("*time*");
                return;
        }
    } else if (issymbol(c)) {
        printf("%s", oxcell_get_text(oxcell_get_name(c)));
        return;
    } else if (islist(c)) {
        printf("(");
        oxcell_print(oxcell_get_first(c));
        c = oxcell_get_rest(c);
        while (!isnil(c) && islist(c)) {
            printf(" ");
            oxcell_print(oxcell_get_first(c));
            c = oxcell_get_rest(c);
        }
        printf( ")");
        return;
    }
    printf("error: internal error - cell not atom/list/symbol\n");
    exit(2);
}

#endif