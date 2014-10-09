//
//  ox/oxval.h
//
//  Created by Michael Henderson on 10/2/14.
//  Copyright (c) 2014 Michael D Henderson. All rights reserved.
//

#ifndef __ox__oxval__
#define __ox__oxval__

#include "local.h"

#include <stdlib.h>

// simple structure to hold common data types
//
typedef struct oxcell {
    enum { oxcBool, oxcFunc, oxcInteger, oxcList, oxcName, oxcReal, oxcSymbol, oxcText, oxcTimestamp } kind;
    union {
        struct {
            struct oxcell *first;
            struct oxcell *rest;
        } list;
        union {
            int boolean;
            struct oxcell *(*func)(struct oxcell *args, struct oxcell *env);
            char *name;
            union {
                long   integer;
                double real;
            } number;
            struct {
                char          *name;
                struct oxcell *value;
            } symbol;
            struct {
                char   *value;
                int     isNull;
                size_t  length;
            } text;
            unsigned long timestamp;
        } atom;
    } u;
} oxcell;

oxval *oxval_alloc(char type, char isNull, ...);


#endif /* defined(__ox__oxval__) */
