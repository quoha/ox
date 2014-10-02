//
//  ox/oxstk.c
//
//  Created by Michael Henderson on 10/2/14.
//  Copyright (c) 2014 Michael D Henderson. All rights reserved.
//

#include "oxstk.h"

#include <stdio.h>
#include <stdlib.h>

static oxent *oxent_new(void);

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
