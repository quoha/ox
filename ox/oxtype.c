//
//  ox/oxtype.c
//
//  Created by Michael Henderson on 10/3/14.
//  Copyright (c) 2014 Michael D Henderson. All rights reserved.
//

#include "oxtype.h"

#include <stdio.h>

#define ABEND(x) do { printf(x); printf("\t%s %s %d\n", __FILE__, __FUNCTION__, __LINE__); exit(2); } while (0)

int OXISATOM(oxatom *a) {
    return (a && (a->kind == oxaBoolean || a->kind == oxaInteger || a->kind == oxaReal || a->kind == oxaText)) ? -1 : 0;
}

int OXASBOOLEAN(oxatom *a) {
    if (!(a && a->kind == oxaBoolean)) {
        ABEND("error:\tinternal error - reading a non-boolean cell\n");
    }
    return a->boolean ? -1 : 0;
}

int OXASINTEGER(oxatom *a) {
    if (!(a && a->kind == oxaInteger)) {
        ABEND("error:\tinternal error - reading a non-integer cell\n");
    } else if (a && a->kind == oxaInteger && a->isNull) {
        ABEND("error:\tinternal error - fetching null integer\n");
    }
    return a->number.integer;
}

void OXASNUMBER(oxatom *a) {
    ABEND("error:\tinternal error - should never call OXASNUMBER\n");
}

double OXASREAL(oxatom *a) {
    if (!(a && a->kind == oxaReal)) {
        ABEND("error:\tinternal error - reading a non-real cell\n");
    } else if (a && a->kind == oxaInteger && a->isNull) {
        ABEND("error:\tinternal error - fetching null real\n");
    }
    return a->number.real;
}

const char *OXASTEXT(oxatom *a) {
    if (!(a && a->kind == oxaText)) {
        ABEND("error:\tinternal error - reading a non-text cell\n");
    } else if (a && a->kind == oxaText && a->isNull) {
        ABEND("error:\tinternal error - fetching null text\n");
    }
    return a->text.value;
}

int OXISBOOLEAN(oxatom *a) {
    return (a && a->kind == oxaBoolean) ? -1 : 0;
}

int OXISINTEGER(oxatom *a) {
    return (a && a->kind == oxaInteger) ? -1 : 0;
}

int OXISNUMBER(oxatom *a) {
    return (OXISINTEGER(a) || OXISREAL(a)) ? -1 : 0;
}

int OXISREAL(oxatom *a) {
    return (a && a->kind == oxaReal) ? -1 : 0;
}

int OXISSYMBOL(oxatom *a) {
    return (a && a->kind == oxaSymbol) ? -1 : 0;
}

int OXISTEXT(oxatom *a) {
    return (a && a->kind == oxaText) ? -1 : 0;
}
