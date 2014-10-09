//
//  ox/oxalloc.c
//
//  Created by Michael Henderson on 10/6/14.
//  Copyright (c) 2014 Michael D Henderson. All rights reserved.
//
#if 0
#include "oxalloc.h"
#include "oxinit.h"
#include "oxutil.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

oxcell *oxnil = 0;
oxcell *oxenv = 0;

oxcell *oxcell_alloc(void) {
    oxcell *c = malloc(sizeof(*c));
    if (!c) {
        perror(__FUNCTION__);
        exit(2);
    }
    
    c->kind         = octAtom;
    c->isSymbol     = 0;
    c->plist        = 0;
    c->first        = 0;
    c->rest         = 0;
    c->atom         = 0;
    
    return c;
}

oxcell *oxcell_alloc_cstring(const char *cstring) {
    return oxcell_alloc_text(cstring, strlen(cstring));
}

oxcell *oxcell_alloc_env(void) {
    if (!oxenv) {
        oxenv = oxcell_alloc_nil();
    }
    return oxenv;
}

oxcell *oxcell_alloc_func(oxcell *(func)(oxcell *arg, oxcell *env)) {
    oxcell *c           = oxcell_alloc();
    c->kind             = octAtom;
    c->isSymbol         = 0;
    c->plist            = oxnil;
    c->first            = 0;
    c->rest             = 0;
    
    c->atom = malloc(sizeof(*(c->atom)));
    if (!c->atom) {
        perror(__FUNCTION__);
        exit(2);
    }
    memset(c->atom, 0, sizeof(*(c->atom)));
    
    c->atom->kind      = eAtomFunc;
    c->atom->func      = func;
    c->atom->integer   = 0;
    c->atom->real      = 0.0;
    c->atom->text      = 0;
    c->atom->timestamp = 0;
    
    return c;
}

oxcell *oxcell_alloc_integer(long integer) {
    oxcell *c           = oxcell_alloc();
    c->kind             = octAtom;
    c->isSymbol         = 0;
    c->plist            = oxnil;
    c->first            = 0;
    c->rest             = 0;

    c->atom = malloc(sizeof(*(c->atom)));
    if (!c->atom) {
        perror(__FUNCTION__);
        exit(2);
    }
    memset(c->atom, 0, sizeof(*(c->atom)));
    
    c->atom->kind      = eAtomInteger;
    c->atom->func      = 0;
    c->atom->integer   = integer;
    c->atom->real      = 0.0;
    c->atom->text      = 0;
    c->atom->timestamp = 0;
    
    return c;
}

oxcell *oxcell_alloc_list(oxcell *first, oxcell *rest) {
    oxcell *c           = oxcell_alloc();
    c->kind             = octList;
    c->isSymbol         = 0;
    c->plist            = oxnil;
    c->first            = first;
    c->rest             = rest;
    c->atom             = 0;
    return c;
}

oxcell *oxcell_alloc_nil(void) {
    if (!oxnil) {
        oxnil           = oxcell_alloc();
        oxnil->kind     = octAtom;
        oxnil->isSymbol = 0;
        oxnil->plist    = oxnil;
        oxnil->atom     = 0;
        oxnil->first    = oxnil;
        oxnil->rest     = oxnil;
    }
    return oxnil;
}

oxcell *oxcell_alloc_real(double real) {
    oxcell *c           = oxcell_alloc();
    c->kind             = octAtom;
    c->isSymbol         = 0;
    c->first            = 0;
    c->rest             = 0;

    c->atom = malloc(sizeof(*(c->atom)));
    if (!c->atom) {
        perror(__FUNCTION__);
        exit(2);
    }
    memset(c->atom, 0, sizeof(*(c->atom)));
    
    c->atom->kind      = eAtomReal;
    c->atom->plist     = oxnil;
    c->atom->func      = 0;
    c->atom->real      = real;
    c->atom->real      = 0.0;
    c->atom->text      = 0;
    c->atom->timestamp = 0;
    
    return c;
}

oxcell *oxcell_alloc_symbol(oxcell *name, oxcell *value) {
    oxcell *c           = oxcell_alloc();
    c->kind             = octAtom;
    c->isSymbol         = -1;
    c->first            = oxnil;
    c->rest             = oxnil;
    c->atom = malloc(sizeof(*(c->atom)));
    if (!c->atom) {
        perror(__FUNCTION__);
        exit(2);
    }
    memset(c->atom, 0, sizeof(*(c->atom)));

    c->atom->kind      = eAtomReal;
    c->atom->plist     = oxnil;
    c->atom->func      = 0;
    c->atom->real      = real;
    c->atom->real      = 0.0;
    c->atom->text      = 0;
    c->atom->timestamp = 0;

    return c;
}

oxcell *oxcell_alloc_text(const char *text, size_t length) {
    oxcell *c           = oxcell_alloc();
    c->kind             = octAtom;
    c->isSymbol         = 0;
    c->first            = 0;
    c->rest             = 0;
    
    c->atom = malloc(sizeof(*(c->atom)));
    if (!c->atom) {
        perror(__FUNCTION__);
        exit(2);
    }
    memset(c->atom, 0, sizeof(*(c->atom)));
    
    c->atom->kind      = eAtomText;
    c->atom->func      = 0;
    c->atom->integer   = 0;
    c->atom->real      = 0.0;
    c->atom->text      = malloc(length + 1);
    c->atom->timestamp = 0;
    
    if (!c->atom->text) {
        perror(__FUNCTION__);
        exit(2);
    }
    memcpy(c->atom->text, text, length);
    c->atom->text[length] = 0;
    
    return c;
}

oxcell *oxcell_alloc_timestamp(unsigned long timestamp) {
    oxcell *c           = oxcell_alloc();
    c->kind             = octAtom;
    c->isSymbol         = 0;
    c->isUndefinedValue = 0;
    c->first            = 0;
    c->rest             = 0;
    
    c->atom = malloc(sizeof(*(c->atom)));
    if (!c->atom) {
        perror(__FUNCTION__);
        exit(2);
    }
    memset(c->atom, 0, sizeof(*(c->atom)));
    
    c->atom->kind      = eAtomTime;
    c->atom->func      = 0;
    c->atom->integer   = 0;
    c->atom->real      = 0.0;
    c->atom->text      = 0;
    c->atom->timestamp = timestamp;
    
    return c;
}
#endif