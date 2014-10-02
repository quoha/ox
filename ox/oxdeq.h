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

// simple double ended queue. has no understanding of the
// data that it is storing. supports push/pop/peek operations
// to the front and back of the queue. supports reversing the
// data (which is odd because you could just push/pop from the
// other end of the queue to do the same thing).
//
typedef struct oxdeq {
    struct oxqent {
        struct oxqent *left;
        struct oxqent *right;
        void          *data;
    } *left, *right, *e, *t;
    size_t numberOfEntries;
} oxdeq;

oxdeq  *oxdeq_alloc(void);
oxdeq  *oxdeq_copy(oxdeq *q, void *(*fcopy)(void *data));
ssize_t oxdeq_number_of_entries(oxdeq *q);
void   *oxdeq_peek_left(oxdeq *q);
void   *oxdeq_peek_right(oxdeq *q);
void   *oxdeq_pop_left(oxdeq *q);
void   *oxdeq_pop_right(oxdeq *q);
oxdeq  *oxdeq_push_left(oxdeq *q, void *data);
oxdeq  *oxdeq_push_right(oxdeq *q, void *data);
oxdeq  *oxdeq_reverse(oxdeq *q);

// convenience macros to peek/push/pop like a stack
//
#define oxdeq_peek(q)    oxdeq_peek_left((q))
#define oxdeq_pop(q)     oxdeq_pop_left((q))
#define oxdeq_push(q, d) oxdeq_push_left((q), (d))

#endif /* defined(__ox__oxdeq__) */
