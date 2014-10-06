//
//  ox/oxprim.c
//
//  Created by Michael Henderson on 10/6/14.
//  Copyright (c) 2014 Michael D Henderson. All rights reserved.
//

#include "oxtype.h"
#include "oxalloc.h"
#include "oxinit.h"
#include "oxprim.h"
#include "oxutil.h"


oxcell *oxf_car(oxcell *args, oxcell *env) {
    if (args) {
        if (args->kind == octCons) {
            return args->cons.car;
        }
    }
    return 0;
}

oxcell *oxf_cdr(oxcell *args, oxcell *env) {
    if (args) {
        if (args->kind == octCons) {
            return args->cons.cdr;
        }
    }
    return 0;
}

oxcell *oxf_cons(oxcell *args, oxcell *env) {
    oxcell *car = oxf_car(args, env);
    oxcell *cdr = oxf_car(oxf_cdr(args, env), env);
    oxcell *c   = oxcell_alloc();
    oxcell_set_car(c, car);
    oxcell_set_cdr(c, cdr);
    return c;
}

oxcell *oxf_lambda(oxcell *args, oxcell *env) {
    return nill;
}

oxcell *oxf_noop(oxcell *args, oxcell *env) {
    return nill;
}

oxcell *oxf_quote(oxcell *args, oxcell *env) {
    return nill;
}

