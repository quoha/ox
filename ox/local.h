//
//  ox/local.h
//
//  Created by Michael Henderson on 10/1/14.
//  Copyright (c) 2014 Michael D Henderson. All rights reserved.
//

#ifndef ox_local_h
#define ox_local_h

#include <stdlib.h>

typedef struct oxbuf    oxbuf;
typedef struct oxent    oxent;
typedef struct oxstk    oxstk;
typedef struct oxval    oxval;
typedef struct oxwhtlst oxwhtlst;

#include "oxbuf.h"
#include "oxent.h"

struct oxstk {
    struct oxent *top;
    struct oxent *bottom;
    int           height;
};

struct oxval {
    union {
        int           boolean;
        struct oxstk *stack;
        struct {
            union {
                int    integer;
                double real;
            } value;
            int    isNull;
            enum { oxtInteger , oxtReal } kind;
        } number;
        struct {
            char   *value;
            ssize_t length;
            int     isNull;
        } text;
        struct {
            char         *name;
            struct oxval *value;
        } symbol;
    } u;
    enum { oxtBool, oxtNumber, oxtStack, oxtSymbol, oxtText } kind;
    unsigned char data[1];
};

#endif
