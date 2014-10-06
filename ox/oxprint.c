//
//  ox/oxprint.c
//
//  Created by Michael Henderson on 10/6/14.
//  Copyright (c) 2014 Michael D Henderson. All rights reserved.
//

#include "oxtype.h"
#include "oxprint.h"
#include "oxutil.h"

#include <stdio.h>

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
    } else if (iscons(c)) {
        printf("(");
        oxcell_print(car(c));
        c = cdr(c);
        while (!isnil(c) && iscons(c)) {
            printf(" ");
            oxcell_print(car(c));
            c = cdr(c);
        }
        printf( ")");
        return;
    }
    printf("error: internal error - cell not atom/list/symbol\n");
    exit(2);
}

