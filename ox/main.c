//
//  ox/main.c
//
//  Created by Michael Henderson on 10/1/14.
//  Copyright (c) 2014 Michael D Henderson. All rights reserved.
//

#include "local.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

oxval *oxval_alloc(char type, char isNull, ...);

oxstk *oxstk_new(void) {
    oxstk *s = malloc(sizeof(*s));
    if (!s) {
        perror(__FUNCTION__);
        exit(2);
    }
    s->top = s->bottom = 0;
    s->height = 0;
    return s;
}

oxval *oxval_alloc_boolean(int value) {
    oxval *v = malloc(sizeof(*v));
    if (!v) {
        perror(__FUNCTION__);
        exit(2);
    }
    v->kind      = oxtBool;
    v->u.boolean = value ? -1 : 0;
    return v;
}

oxval *oxval_alloc_integer(int value, int isNull) {
    oxval *v = malloc(sizeof(*v));
    if (!v) {
        perror(__FUNCTION__);
        exit(2);
    }
    v->kind                   = oxtNumber;
    v->u.number.kind          = oxtInteger;
    v->u.number.isNull        = isNull ? -1 : 0;
    v->u.number.value.integer = value;
    return v;
}

oxval *oxval_alloc_real(double value, int isNull) {
    oxval *v = malloc(sizeof(*v));
    if (!v) {
        perror(__FUNCTION__);
        exit(2);
    }
    v->kind                = oxtNumber;
    v->u.number.kind       = oxtReal;
    v->u.number.isNull     = isNull ? -1 : 0;
    v->u.number.value.real = value;
    return v;
}

oxval *oxval_alloc_stack(oxstk *stack) {
    oxval *v = malloc(sizeof(*v));
    if (!v) {
        perror(__FUNCTION__);
        exit(2);
    }
    v->kind      = oxtStack;
    v->u.stack   = oxstk_new();
    if (!v->u.stack) {
        perror(__FUNCTION__);
        exit(2);
    }
    if (stack) {
        // TODO: deep copy of stack here
    }
    return v;
}

oxval *oxval_alloc_symbol(const char *name, oxval *value) {
    ssize_t len = name ? strlen(name) : 0;
    oxval *v = malloc(sizeof(*v) + len);
    if (!v) {
        perror(__FUNCTION__);
        exit(2);
    }
    v->kind                = oxtSymbol;
    v->u.symbol.name       = (char *)(v->data);;
    v->u.symbol.value      = value;
    strcpy(v->u.symbol.name, name ? name : "");
    return v;
}

oxval *oxval_alloc_text(const char *value) {
    ssize_t len = value ? strlen(value) : 0;
    oxval *v = malloc(sizeof(*v) + len);
    if (!v) {
        perror(__FUNCTION__);
        exit(2);
    }
    v->kind                   = oxtText;
    v->u.text.isNull          = value ? 0 : -1;
    v->u.text.length          = len;
    v->u.text.value           = (char *)(v->data);
    strcpy(v->u.text.value, value ? value : "");
    return v;
}

//
// oxval_alloc(flag, value)
//
// flag value__ is_null______________________
//    b boolean not applicable
//    d real    per the isNull flag
//    i integer per the isNull flag
//    k stack   not applicable
//    t text    if ptr is null or isNull flag
//    s symbol  not applicable
//
oxval *oxval_alloc(char type, char isNull, ...) {
    oxval *v = malloc(sizeof(*v));
    if (!v) {
        perror(__FUNCTION__);
        exit(2);
    }

    int         boolean = 0;
    int         integer = 0;
    const char *name    = NULL;
    double      real    = 0.0;
    oxstk      *stack   = NULL;
    const char *text    = NULL;
    oxval      *value   = NULL;
    
    va_list ap;
    va_start(ap, isNull);

    switch (type) {
        case 'b':
            boolean = va_arg(ap, int);
            break;
        case 'd':
            real = va_arg(ap, double);
            break;
        case 'f':
            real = va_arg(ap, double);
            break;
        case 'i':
            integer = va_arg(ap, int);
            break;
        case 'k':
            stack = va_arg(ap, oxstk *);
            break;
        case 's':
            name  = va_arg(ap, const char *);
            value = va_arg(ap, oxval *);
            break;
        case 't':
            text = va_arg(ap, const char *);
            break;
        default:
            boolean = 0;
            integer = 0;
            name    = NULL;
            real    = 0.0;
            stack   = NULL;
            text    = NULL;
            value   = NULL;
            break;
    }
    
    va_end(ap);

    switch (type) {
        case 'b':
            return oxval_alloc_boolean(boolean);
        case 'd':
            return oxval_alloc_real(real, isNull);
        case 'f':
            return oxval_alloc_real(real, isNull);
        case 'i':
            return oxval_alloc_integer(integer, isNull);
        case 'k':
            return oxval_alloc_stack(NULL);
        case 's':
            return oxval_alloc_symbol(name, value);
        case 't':
            return oxval_alloc_text(isNull ? 0 : text);
    }

    fprintf(stderr, "error:\tinvalid type '%c' passed to %s\n", type, __FUNCTION__);
    exit(2);

    // NOT REACHED
    return 0;
}

int main(int argc, const char * argv[]) {
    oxwhtlst w;
    memset(&w, -1, sizeof(w));

    oxbuf *a = oxbuf_from_cstring("hello, world  \nthis is a test\n");

    printf("%s\n", a->data);

    return 0;
}
