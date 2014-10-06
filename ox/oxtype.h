//
//  ox/oxtype.h
//
//  Created by Michael Henderson on 10/3/14.
//  Copyright (c) 2014 Michael D Henderson. All rights reserved.
//

#ifndef __ox__oxtype__
#define __ox__oxtype__

enum atomType {eAtomFunc, eAtomInteger, eAtomReal, eAtomText, eAtomTime};

typedef struct oxatom {
    enum atomType    kind;      // kind for atoms
    struct oxcell *(*func)(struct oxcell *, struct oxcell *);
    long             integer;   // value for integer
    double           real;      // value for real
    char            *text;      // text for strings
    unsigned long    timestamp; // time values
} oxatom;

typedef struct oxcell {
    enum { octAtom, octCons } kind;
    
    oxatom   *atom;
    int       isSymbol;
    
    struct {
        struct oxcell *car;
        struct oxcell *cdr;
    } cons;
} oxcell;

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


#endif /* defined(__ox__oxtype__) */
