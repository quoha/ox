//
//  ox/oxparser.h
//
//  Created by Michael Henderson on 10/3/14.
//  Copyright (c) 2014 Michael D Henderson. All rights reserved.
//

#ifndef __ox__oxparser__
#define __ox__oxparser__

typedef struct oxtoken {
    union {
        long   integer;
        char  *name;
        double real;
        char  *text;
    } value;
    enum {oxTokEOF, oxTokOpenParen, oxTokCloseParen, oxTokInteger, oxTokReal, oxTokSymbol, oxTokText} kind;
    int line;
    unsigned char data[1];
} oxtoken;

#include "oxbuf.h"

oxtoken    *oxtok_read(oxbuf *ib);
const char *oxtok_toktype(oxtoken *t);

#endif /* defined(__ox__oxparser__) */
