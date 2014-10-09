//
//  ox/oxcell.h
//
//  Created by Michael Henderson on 10/7/14.
//  Copyright (c) 2014 Michael D Henderson. All rights reserved.
//

#ifndef __ox__oxcell__
#define __ox__oxcell__

#include <stdlib.h>

typedef struct oxcell    oxcell;
typedef struct oxchmap   oxchmap;
typedef struct oxtext    oxtext;
typedef        oxcell *(*oxfunc)(oxcell *args, oxcell *env);

// why must everyone write their own string?
//
struct oxtext {
    unsigned char *curr;
    unsigned char *endOfData;
    unsigned long  hashValue;
    size_t         length;
    int            lineNumber;
    unsigned char data[1];
};

// character map for whitelisting and such
//
struct oxchmap {
    unsigned char map[256];
};

// the official list of common data types
//
typedef enum { oxcBoolean, oxcChMap, oxcFunc, oxcInteger, oxcList, oxcName, oxcReal, oxcSymbol, oxcText, oxcTimestamp } oxcellKind;

// simple structure to hold common data types
//
struct oxcell {
    oxcellKind kind;
    union {
        struct {
            oxcell          *first;
            oxcell          *rest;
        } list;
        union {
            int              boolean;
            oxchmap          chmap;
            unsigned long    dttm;
            oxfunc           func;
            long             integer;
            oxtext          *name;
            double           real;
            struct {
                oxtext      *name;
                oxcell      *value;
            } symbol;
            oxtext          *text;
        } atom;
    } u;
};

#define OXISLIST(c)    ((c)->kind == oxcList)
#define OXISATOM(c)    (!(OXISLIST(c)))

#define OXCFIRST(c)    ((c)->u.list.first)
#define OXCREST(c)     ((c)->u.list.rest)
#define OXCBOOLEAN(c)  ((c)->u.atom.boolean)
#define OXCHMAP(c)     ((c)->u.atom.chmap)
#define OXCDTTM(c)     ((c)->u.atom.dttm)
#define OXCFUNC(c)     ((c)->u.atom.func)
#define OXCINTEGER(c)  ((c)->u.atom.integer)
#define OXCNAME(c)     ((c)->u.atom.name)
#define OXCREAL(c)     ((c)->u.atom.real)
#define OXCTEXT(c)     ((c)->u.atom.text)
#define OXCSYMBOL(c)   ((c)->u.atom.symbol)

extern oxcell *oxenv;
extern oxcell *oxnil;
extern oxcell *oxtrue;

oxcell *oxcell_factory(oxcellKind kind, ...);

oxcell *oxcell_get_env(void);
oxcell *oxcell_get_nil(void);
oxcell *oxcell_get_true(void);

oxtext *oxtext_from_cstring(const char *cstring);
oxtext *oxtext_from_memory(unsigned char *memory, size_t length);

#endif /* defined(__ox__oxcell__) */
