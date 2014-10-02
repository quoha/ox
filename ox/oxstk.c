//
//  ox/oxstk.c
//
//  Created by Michael Henderson on 10/2/14.
//  Copyright (c) 2014 Michael D Henderson. All rights reserved.
//

#include "oxstk.h"

#include <stdio.h>
#include <stdlib.h>

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

