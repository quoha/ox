//
//  ox/oxdeq.c
//
//  Created by Michael Henderson on 10/2/14.
//  Copyright (c) 2014 Michael D Henderson. All rights reserved.
//

#include "oxdeq.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

oxdeq *oxdeq_alloc(void) {
    oxdeq *q = malloc(sizeof(*q));
    if (!q) {
        perror(__FUNCTION__);
        exit(2);
    }
    q->front = q->back = q->e = q->t = 0;
    q->numberOfEntries = 0;
    return q;
}

ssize_t oxdeq_number_of_entries(oxdeq *q) {
    return q ? q->numberOfEntries : 0;
}

void *oxdeq_peek_back(oxdeq *q) {
    return (q && q->back) ? q->back->data : 0;
}

void *oxdeq_peek_front(oxdeq *q) {
    return (q && q->front) ? q->front->data : 0;
}

void *oxdeq_pop_back(oxdeq *q) {
    void *v = (q && q->back) ? q->back->data : 0;
    if (v) {
        q->e    = q->back;
        q->back = q->back->prev;
        if (q->back) {
            q->back->next = 0;
        } else {
            q->front = q->back = 0;
        }
        q->numberOfEntries--;
        free(q->e);
        q->e = 0;
    }
    return v;
}

void *oxdeq_pop_front(oxdeq *q) {
    void *v = (q && q->front) ? q->front->data : 0;
    if (v) {
        q->e     = q->front;
        q->front = q->front->next;
        if (q->front) {
            q->front->prev = 0;
        } else {
            q->front = q->back = 0;
        }
        q->numberOfEntries--;
        free(q->e);
        q->e = 0;
    }
    return v;
}

oxdeq *oxdeq_push_back(oxdeq *q, void *data) {
    if (q && data) {
        q->e = malloc(sizeof(*(q->e)));
        if (!q->e) {
            perror(__FUNCTION__);
            exit(2);
        }
        q->e->data = data;
        q->e->data = data;
        q->e->prev = q->back;
        q->e->next = 0;
        if (!q->back) {
            q->front = q->back = q->e;
        } else {
            q->back->next = q->e;
            q->back       = q->e;
        }
        q->numberOfEntries++;
        q->e = 0;
    }
    return q;
}

oxdeq *oxdeq_push_front(oxdeq *q, void *data) {
    if (q && data) {
        q->e = malloc(sizeof(*(q->e)));
        if (!q->e) {
            perror(__FUNCTION__);
            exit(2);
        }
        q->e->data = data;
        q->e->prev = 0;
        q->e->next = q->front;
        if (!q->front) {
            q->front = q->back = q->e;
        } else {
            q->front->prev = q->e;
            q->front       = q->e;
        }
        q->numberOfEntries++;
        q->e = 0;
    }
    return q;
}

// reverse the queue by swapping the data in all of the entries.
// start at the front and the back and work until we meet in the
// middle.
//
oxdeq *oxdeq_reverse(oxdeq *q) {
    if (q) {
        q->e = q->front;
        q->t = q->back;

        while (q->e != q->t) {
            void *tmp  = q->e->data;
            q->e->data = q->t->data;
            q->t->data = tmp;
            
            if (q->e->next == q->t) {
                // the two ends have met, so we're done
                //
                break;
            }
            q->e       = q->e->next;
            q->t       = q->t->prev;
        }
    }
    return q;
}
