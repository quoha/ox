//
//  ox/oxprim.c
//
//  Created by Michael Henderson on 10/6/14.
//  Copyright (c) 2014 Michael D Henderson. All rights reserved.
//
#if 0
#include "oxtype.h"
#include "oxalloc.h"
#include "oxinit.h"
#include "oxprim.h"
#include "oxutil.h"

oxcell *oxf_glyph_add(oxcell *args, oxcell *env) {
    if (args) {
        oxcell *op1 = oxcell_get_first(args);
        oxcell *op2 = oxcell_get_first(oxcell_get_rest(args));
        if (isNumber(op1) && isNumber(op2)) {
            if (op1->atom->kind == eAtomInteger && op2->atom->kind == eAtomInteger) {
                return oxcell_alloc_integer(op1->atom->integer + op2->atom->integer);
            }
            if (op1->atom->kind == eAtomReal && op2->atom->kind == eAtomInteger) {
                return oxcell_alloc_real(op1->atom->real + op2->atom->integer);
            }
            if (op1->atom->kind == eAtomInteger && op2->atom->kind == eAtomReal) {
                return oxcell_alloc_real(op1->atom->integer + op2->atom->real);
            }
            if (op1->atom->kind == eAtomReal && op2->atom->kind == eAtomReal) {
                return oxcell_alloc_real(op1->atom->real + op2->atom->real);
            }
        }
    }
    return oxnil;
}

oxcell *oxf_glyph_subtract(oxcell *args, oxcell *env) {
    if (args) {
        oxcell *op1 = oxcell_get_first(args);
        oxcell *op2 = oxcell_get_first(oxcell_get_rest(args));
        if (isNumber(op1) && isNumber(op2)) {
            if (op1->atom->kind == eAtomInteger && op2->atom->kind == eAtomInteger) {
                return oxcell_alloc_integer(op1->atom->integer - op2->atom->integer);
            }
            if (op1->atom->kind == eAtomReal && op2->atom->kind == eAtomInteger) {
                return oxcell_alloc_real(op1->atom->real - op2->atom->integer);
            }
            if (op1->atom->kind == eAtomInteger && op2->atom->kind == eAtomReal) {
                return oxcell_alloc_real(op1->atom->integer - op2->atom->real);
            }
            if (op1->atom->kind == eAtomReal && op2->atom->kind == eAtomReal) {
                return oxcell_alloc_real(op1->atom->real - op2->atom->real);
            }
        }
    }
    return oxnil;
}

oxcell *oxf_first(oxcell *args, oxcell *env) {
    if (args) {
        if (args->kind == octList) {
            return args->first ? args->first : oxnil;
        }
    }
    return oxnil;
}

oxcell *oxf_list(oxcell *args, oxcell *env) {
    oxcell *first = oxf_first(args, env);
    oxcell *rest  = oxf_rest(oxf_rest(args, env), env);
    oxcell *c    = oxcell_alloc();
    oxcell_set_first(c, first);
    oxcell_set_rest(c, rest);
    return c;
}

oxcell *oxf_lambda(oxcell *args, oxcell *env) {
    return oxnil;
}

oxcell *oxf_noop(oxcell *args, oxcell *env) {
    return oxnil;
}

oxcell *oxf_quote(oxcell *args, oxcell *env) {
    return oxnil;
}

oxcell *oxf_rest(oxcell *args, oxcell *env) {
    if (args) {
        if (args->kind == octList) {
            return args->rest ? args->rest : oxnil;
        }
    }
    return oxnil;
}

#endif