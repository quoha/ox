//
//  ox/oxdeq.h
//
//  Created by Michael Henderson on 10/2/14.
//  Copyright (c) 2014 Michael D Henderson. All rights reserved.
//

#ifndef __ox__oxdeq__
#define __ox__oxdeq__

#include <stdio.h>
#include <stdlib.h>

typedef struct oxdeq {
    struct oxqent {
        struct oxqent *prev;
        struct oxqent *next;
        void          *data;
    } *front, *back, *e, *t;
    size_t numberOfEntries;
} oxdeq;

oxdeq  *oxdeq_alloc(void);
ssize_t oxdeq_number_of_entries(oxdeq *q);
void   *oxdeq_peek_back(oxdeq *q);
void   *oxdeq_peek_front(oxdeq *q);
void   *oxdeq_pop_back(oxdeq *q);
void   *oxdeq_pop_front(oxdeq *q);
oxdeq  *oxdeq_push_back(oxdeq *q, void *data);
oxdeq  *oxdeq_push_front(oxdeq *q, void *data);
oxdeq  *oxdeq_reverse(oxdeq *q);

#endif /* defined(__ox__oxdeq__) */
