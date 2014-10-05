//
//  ox/oxparser.h
//
//  Created by Michael Henderson on 10/3/14.
//  Copyright (c) 2014 Michael D Henderson. All rights reserved.
//

#ifndef __ox__oxparser__
#define __ox__oxparser__

#include <stdlib.h>

#include "oxbuf.h"

typedef struct oxtoken {
    union {
        long   integer;
        char  *name;
        double real;
        char  *text;
    } value;
    enum {oxTokEOF, oxTokOpenParen, oxTokCloseParen, oxTokInteger, oxTokName, oxTokReal, oxTokText} kind;
    int line;
    unsigned char data[1];
} oxtoken;

enum atomType {eAtomFunc, eAtomInteger, eAtomReal, eAtomText};

typedef struct oxatom {
    enum atomType    kind;   // kind for atoms
    struct oxcell *(*func)(struct oxcell *, struct oxcell *);
    long             integer;// value for integer
    double           real;   // value for real
    char            *text;   // text for strings
} oxatom;

typedef struct oxcell {
    enum { octAtom, octCons, octSymbol } kind;

    oxatom *atom;

    struct {
        struct oxcell *car;
        struct oxcell *cdr;
    } cons;

    struct {
        struct oxcell *name;
        struct oxcell *value;
    } symbol;
} oxcell;

oxcell *oxexpr_read(struct oxbuf *ib);
oxcell *oxexpr_read_tail(struct oxbuf *ib);
void    oxexpr_print(struct oxcell *c);

oxcell       *car(oxcell *c);
oxcell       *cdr(oxcell *c);
int           isatom(oxcell *c);
int           iscons(oxcell *c);
int           isnil(oxcell *c);
int           issymbol(oxcell *c);
int           istext(oxcell *c);

oxatom *oxatom_alloc_integer(long integer);
oxatom *oxatom_alloc_real(double real);
oxatom *oxatom_alloc_text(const char *text, size_t length);

oxcell     *oxcell_get_car(oxcell *c);
oxcell     *oxcell_get_cdr(oxcell *c);
long        oxcell_get_integer(oxcell *c);
double      oxcell_get_real(oxcell *c);
oxcell     *oxcell_get_name(oxcell *c);
const char *oxcell_get_text(oxcell *c);
oxcell     *oxcell_get_value(oxcell *c);

oxcell *oxcell_set_car(oxcell *c, oxcell *t);
oxcell *oxcell_set_cdr(oxcell *c, oxcell *t);

oxcell *oxcell_alloc(oxcell *car, oxcell *cdr);
oxcell *oxcell_alloc_cons(oxcell *car, oxcell *cdr);
oxcell *oxcell_alloc_cstring(const char *cstring);
oxcell *oxcell_alloc_integer(long integer);
oxcell *oxcell_alloc_real(double real);
oxcell *oxcell_alloc_text(const char *text, size_t len);
oxcell *oxcell_alloc_symbol(oxcell *name, oxcell *value);

oxtoken    *oxtok_read(struct oxbuf *ib);
const char *oxtok_toktype(oxtoken *t);

#endif /* defined(__ox__oxparser__) */
