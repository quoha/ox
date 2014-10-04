//
//  ox/oxtype.h
//
//  Created by Michael Henderson on 10/3/14.
//  Copyright (c) 2014 Michael D Henderson. All rights reserved.
//

#ifndef __ox__oxtype__
#define __ox__oxtype__

#include <stdlib.h>

// obviously not worried about code size or speed at the moment
//
typedef struct oxatom {
    enum { oxaBoolean, oxaInteger, oxaReal, oxaSymbol, oxaText } kind;
    int          isNull;
    int          boolean;
    struct {
        int          integer;
        double       real;
    }            number;
    struct {
        struct oxatom *name;
        struct oxcell *plist;
    }            symbol;
    struct {
        char        *value;
        ssize_t      length;
    }            text;
    unsigned char data[1];
} oxatom;

// if atom is set, then this is an atom
// if first is set, then this is a list
// if a list and rest is NULL, then this is the end of the list
//
typedef struct oxcell {
    struct oxatom *atom;
    struct oxatom *first;
    struct oxcell *rest;
} oxcell;

int         OXISATOM(oxcell *c);
int         OXISBOOLEAN(oxatom *a);
int         OXISINTEGER(oxatom *a);
int         OXISNUMBER(oxatom *a);
int         OXISREAL(oxatom *a);
int         OXISSYMBOL(oxatom *a);
int         OXISTEXT(oxatom *a);

int         OXASINTEGER(oxatom *a);
void        OSASNUMBER(oxatom *a);
double      OXASREAL(oxatom *a);
const char *OXASTEXT(oxatom *a);

#endif /* defined(__ox__oxtype__) */
