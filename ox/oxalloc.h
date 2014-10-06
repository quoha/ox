//
//  ox/oxalloc.h
//
//  Created by Michael Henderson on 10/6/14.
//  Copyright (c) 2014 Michael D Henderson. All rights reserved.
//

#ifndef __ox__oxalloc__
#define __ox__oxalloc__

#define ABEND(x) do { printf(x); printf("\t%s %s %d\n", __FILE__, __FUNCTION__, __LINE__); exit(2); } while (0)

#include "oxtype.h"
#include <stdlib.h>

oxcell *oxcell_alloc(void);
oxcell *oxcell_alloc_cons(oxcell *car, oxcell *cdr);
oxcell *oxcell_alloc_cstring(const char *cstring);
oxcell *oxcell_alloc_func(oxcell *(*func)(oxcell *arg, oxcell *env));
oxcell *oxcell_alloc_integer(long integer);
oxcell *oxcell_alloc_real(double real);
oxcell *oxcell_alloc_text(const char *text, size_t len);
oxcell *oxcell_alloc_symbol(oxcell *name, oxcell *value);

#endif /* defined(__ox__oxalloc__) */
