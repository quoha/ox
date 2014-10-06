//
//  ox/oxutil.h
//
//  Created by Michael Henderson on 10/6/14.
//  Copyright (c) 2014 Michael D Henderson. All rights reserved.
//

#ifndef __ox__oxutil__
#define __ox__oxutil__

#include "oxalloc.h"

unsigned char *strrcpy(unsigned char *src, size_t length);

oxcell       *car(oxcell *c);
oxcell       *cdr(oxcell *c);
int           isatom(oxcell *c);
int           iscons(oxcell *c);
int           islist(oxcell *c);
int           isnil(oxcell *c);
int           issymbol(oxcell *c);
int           istext(oxcell *c);

int           oxcell_eq_text(oxcell *c1, oxcell *c2);
oxcell       *oxcell_get_car(oxcell *c);
oxcell       *oxcell_get_cdr(oxcell *c);
long          oxcell_get_integer(oxcell *c);
double        oxcell_get_real(oxcell *c);
oxcell       *oxcell_get_name(oxcell *c);
const char   *oxcell_get_text(oxcell *c);
oxcell       *oxcell_get_value(oxcell *c);
oxcell       *oxcell_set_car(oxcell *c, oxcell *t);
oxcell       *oxcell_set_cdr(oxcell *c, oxcell *t);
oxcell       *oxsym_lookup(oxcell *name, oxcell *env);

#endif /* defined(__ox__oxutil__) */