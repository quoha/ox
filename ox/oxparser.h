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

enum atomType {eAtomFunc, eAtomInteger, eAtomReal, eAtomText, eAtomName};

typedef struct oxcell {
    enum { octAtom, octList, octName } kind;

    struct oxcell *plist;

    union {
        struct {
            enum atomType kind;   // kind for atoms
            union {
                struct oxcell *(*func)(struct oxcell *, struct oxcell *);
                long             integer;// value for integer
                double           real;   // value for real
                char            *text;   // text for strings
            } u;
        } atom;
        struct {
            struct oxcell *car;
            struct oxcell *cdr;
        } cons;
    }   u;
} oxcell;


oxcell *oxexpr_read(struct oxbuf *ib);
oxcell *oxexpr_read_tail(struct oxbuf *ib);
void    oxexpr_print(struct oxcell *c);

oxcell       *car(oxcell *c);
oxcell       *cdr(oxcell *c);
int           isatom(oxcell *c);
int           iscons(oxcell *c);
int           isname(oxcell *c);
int           isnil(oxcell *c);
int           istext(oxcell *c);
enum atomType type(oxcell *c);

oxcell     *oxcell_get_first(oxcell *c);
oxcell     *oxcell_get_rest(oxcell *c);
long        oxcell_get_integer(oxcell *c);
double      oxcell_get_real(oxcell *c);
const char *oxcell_get_name(oxcell *c);
const char *oxcell_get_text(oxcell *c);

oxcell *oxcell_set_car(oxcell *c, oxcell *t);
oxcell *oxcell_set_cdr(oxcell *c, oxcell *t);

oxcell *oxcell_alloc(oxcell *first, oxcell *second);
oxcell *oxcell_alloc_cons(oxcell *c, oxcell *t);
oxcell *oxcell_alloc_cstring(const char *cstring);
oxcell *oxcell_alloc_integer(long integer);
oxcell *oxcell_alloc_real(double real);
oxcell *oxcell_alloc_text(const char *text, size_t len);

oxtoken    *oxtok_read(struct oxbuf *ib);
const char *oxtok_toktype(oxtoken *t);

#endif /* defined(__ox__oxparser__) */
