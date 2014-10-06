//
//  ox/oxalloc.c
//
//  Created by Michael Henderson on 10/6/14.
//  Copyright (c) 2014 Michael D Henderson. All rights reserved.
//

#include "oxalloc.h"
#include "oxinit.h"
#include "oxutil.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

oxcell *oxcell_alloc(void) {
    oxcell *c = malloc(sizeof(*c));
    if (!c) {
        perror(__FUNCTION__);
        exit(2);
    }
    
    c->kind         = octCons;
    c->atom         = 0;
    c->isSymbol     = 0;
    c->cons.car     = 0;
    c->cons.cdr     = 0;
    
    return c;
}

oxcell *oxcell_alloc_cons(oxcell *car, oxcell *cdr) {
    oxcell *c = oxcell_alloc();
    oxcell_set_car(c, car);
    oxcell_set_cdr(c, cdr);
    return c;
}

oxcell *oxcell_alloc_cstring(const char *cstring) {
    return oxcell_alloc_text(cstring, strlen(cstring));
}

oxcell *oxcell_alloc_func(oxcell *(func)(oxcell *arg, oxcell *env)) {
    oxcell *c           = oxcell_alloc();
    c->kind             = octAtom;
    
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

oxcell *oxcell_alloc_real(double real) {
    oxcell *c           = oxcell_alloc();
    c->kind             = octAtom;
    
    c->atom = malloc(sizeof(*(c->atom)));
    if (!c->atom) {
        perror(__FUNCTION__);
        exit(2);
    }
    memset(c->atom, 0, sizeof(*(c->atom)));
    
    c->atom->kind      = eAtomReal;
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

oxcell *oxcell_alloc_symbol(oxcell *name, oxcell *value) {
    oxcell *c   = oxcell_alloc();
    c->isSymbol = -1;
    oxcell_set_car(c, name);
    oxcell_set_cdr(c, value);
    return c;
}

