//
//  ox/oxval.h
//
//  Created by Michael Henderson on 10/2/14.
//  Copyright (c) 2014 Michael D Henderson. All rights reserved.
//

#ifndef __ox__oxval__
#define __ox__oxval__

#include <stdlib.h>

#include "oxdeq.h"

typedef struct oxval {
    union {
        int           boolean;
        struct oxdeq *queue;
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
    enum { oxtBool, oxtNumber, oxtQueue, oxtStack, oxtSymbol, oxtText } kind;
    unsigned char data[1];
} oxval;

oxval *oxval_alloc(char type, char isNull, ...);

#endif /* defined(__ox__oxval__) */
