//
//  oxinit.c
//  ox
//
//  Created by Michael Henderson on 10/6/14.
//  Copyright (c) 2014 Michael D Henderson. All rights reserved.
//
#if 0
#include "oxtype.h"
#include "oxalloc.h"
#include "oxinit.h"
#include "oxprim.h"

int oxinit(void) {
    if (!oxnil) {
        oxnil = oxcell_alloc_nil();
    }
    if (!oxenv) {
        oxenv = oxcell_alloc_env();
    }

    // add the built ins to the environment
    //
    oxenv = oxcell_alloc_list(oxcell_alloc_symbol(oxcell_alloc_cstring("first" ), oxcell_alloc_func(oxf_first )), oxenv);
    oxenv = oxcell_alloc_list(oxcell_alloc_symbol(oxcell_alloc_cstring("rest"  ), oxcell_alloc_func(oxf_rest  )), oxenv);
    oxenv = oxcell_alloc_list(oxcell_alloc_symbol(oxcell_alloc_cstring("list"  ), oxcell_alloc_func(oxf_list  )), oxenv);
    oxenv = oxcell_alloc_list(oxcell_alloc_symbol(oxcell_alloc_cstring("lambda"), oxcell_alloc_func(oxf_lambda)), oxenv);
    oxenv = oxcell_alloc_list(oxcell_alloc_symbol(oxcell_alloc_cstring("noop"  ), oxcell_alloc_func(oxf_noop  )), oxenv);
    oxenv = oxcell_alloc_list(oxcell_alloc_symbol(oxcell_alloc_cstring("quote" ), oxcell_alloc_func(oxf_quote )), oxenv);

    oxenv = oxcell_alloc_list(oxcell_alloc_symbol(oxcell_alloc_cstring("+"), oxcell_alloc_func(oxf_glyph_add     )), oxenv);
    oxenv = oxcell_alloc_list(oxcell_alloc_symbol(oxcell_alloc_cstring("-"), oxcell_alloc_func(oxf_glyph_subtract)), oxenv);

    return -1;
}

#endif