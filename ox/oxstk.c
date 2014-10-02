//
//  ox/oxstk.c
//
//  Created by Michael Henderson on 10/2/14.
//  Copyright (c) 2014 Michael D Henderson. All rights reserved.
//

#include "oxstk.h"
#include "oxval.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

typedef struct oxent {
    struct oxent *prev;
    struct oxent *next;
    struct oxval *value;
} oxent;

static oxent *oxent_new(void);

oxstk *oxstk_push(oxstk *s, char type, ...);
oxval *oxstk_pop(oxstk *s);

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

oxval *oxstk_pop(oxstk *s) {
    oxval *v = 0;
    if (s && s->top) {
        oxent *e = s->top;
        
        s->top = s->top->next;
        if (s->top) {
            s->top->prev = 0;
        } else {
            s->bottom = 0;
        }
        s->height--;

        v = e->value;

        free(e);
    }
    return v;
}

oxstk *oxstk_push(oxstk *s, char type, ...) {
    if (s) {
        oxent *e = malloc(sizeof(*e));
        if (!e) {
            perror(__FUNCTION__);
            exit(2);
        }
        e->value = 0;
        e->prev  = NULL;
        e->next  = s->top;
        if (e->next) {
            e->next->prev = e;
        }
        if (!s->top) {
            s->bottom = e;
        }
        s->top = e;
        s->height++;
    }
    return s;
}

static oxent *oxent_new(void) {
    oxent *e = malloc(sizeof(*e));
    if (!e) {
        perror(__FUNCTION__);
        exit(2);
    }
    e->prev = e->next = 0;
    e->value = 0;
    return e;
}
