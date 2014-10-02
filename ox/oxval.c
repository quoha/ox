//
//  ox/oxval.c
//
//  Created by Michael Henderson on 10/2/14.
//  Copyright (c) 2014 Michael D Henderson. All rights reserved.
//

#include "oxval.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

static oxval *oxval_alloc_boolean(int value);
static oxval *oxval_alloc_integer(int value, int isNull);
static oxval *oxval_alloc_queue(oxdeq *queue);
static oxval *oxval_alloc_real(double value, int isNull);
static oxval *oxval_alloc_symbol(const char *name, oxval *value);
static oxval *oxval_alloc_text(const char *value);

//
// oxval_alloc(flag, isNullFlag, value)
//
// flag value__ is_null______________________
//    b boolean not applicable
//    d real    per the isNull flag
//    i integer per the isNull flag
//    q queue   not applicable
//    s symbol  not applicable
//    t text    if ptr is null or isNull flag
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
    oxdeq      *queue   = NULL;
    double      real    = 0.0;
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
        case 'q':
            queue = va_arg(ap, oxdeq *);
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
            queue   = NULL;
            real    = 0.0;
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
        case 'q':
            return oxval_alloc_queue(queue);
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

static oxval *oxval_alloc_boolean(int value) {
    oxval *v = malloc(sizeof(*v));
    if (!v) {
        perror(__FUNCTION__);
        exit(2);
    }
    v->kind      = oxtBool;
    v->u.boolean = value ? -1 : 0;
    return v;
}

static oxval *oxval_alloc_integer(int value, int isNull) {
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

static oxval *oxval_alloc_queue(oxdeq *queue) {
    oxval *v = malloc(sizeof(*v));
    if (!v) {
        perror(__FUNCTION__);
        exit(2);
    }
    v->kind      = oxtQueue;
    if (queue) {
        v->u.queue   = oxdeq_copy(queue, 0);
    } else {
        v->u.queue   = oxdeq_alloc();
    }
    if (!v->u.queue) {
        perror(__FUNCTION__);
        exit(2);
    }
    return v;
}

static oxval *oxval_alloc_real(double value, int isNull) {
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

static oxval *oxval_alloc_symbol(const char *name, oxval *value) {
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

static oxval *oxval_alloc_text(const char *value) {
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
