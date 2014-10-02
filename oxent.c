//
//  ox/oxent.c
//
//  Created by Michael Henderson on 10/2/14.
//  Copyright (c) 2014 Michael D Henderson. All rights reserved.
//

#include "oxent.h"

#include <stdio.h>
#include <stdlib.h>

oxent *oxent_new(void) {
    oxent *e = malloc(sizeof(*e));
    if (!e) {
        perror(__FUNCTION__);
        exit(2);
    }
    e->prev = e->next = 0;
    e->value = 0;
    return e;
}

