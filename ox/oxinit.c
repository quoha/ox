//
//  oxinit.c
//  ox
//
//  Created by Michael Henderson on 10/6/14.
//  Copyright (c) 2014 Michael D Henderson. All rights reserved.
//

#include "oxtype.h"
#include "oxalloc.h"
#include "oxinit.h"
#include "oxprim.h"

oxcell *nill;
oxcell *myEnv;

int oxinit(void) {
    if (!nill) {
        nill = oxcell_alloc();
        nill->cons.car = nill;
        nill->cons.cdr = nill;
    }
    
    // add the built ins to the environment
    //
    myEnv = oxcell_alloc_cons(oxcell_alloc_symbol(oxcell_alloc_cstring("car"   ), oxcell_alloc_func(oxf_car   )), nill );
    myEnv = oxcell_alloc_cons(oxcell_alloc_symbol(oxcell_alloc_cstring("cdr"   ), oxcell_alloc_func(oxf_cdr   )), myEnv);
    myEnv = oxcell_alloc_cons(oxcell_alloc_symbol(oxcell_alloc_cstring("cons"  ), oxcell_alloc_func(oxf_cons  )), myEnv);
    myEnv = oxcell_alloc_cons(oxcell_alloc_symbol(oxcell_alloc_cstring("lambda"), oxcell_alloc_func(oxf_lambda)), myEnv);
    myEnv = oxcell_alloc_cons(oxcell_alloc_symbol(oxcell_alloc_cstring("noop"  ), oxcell_alloc_func(oxf_noop  )), myEnv);
    myEnv = oxcell_alloc_cons(oxcell_alloc_symbol(oxcell_alloc_cstring("quote" ), oxcell_alloc_func(oxf_quote )), myEnv);
    
    return -1;
}

