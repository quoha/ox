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
    q->left = q->right = q->e = q->t = 0;
    q->numberOfEntries = 0;
    return q;
}

// potentially create a deep copy of a queue.
//
// the 'fcopy' function is responsible for creating a deep copy
// of the data. if function is not supplied, then will do a shallow
// copy of the queue by sharing the data between the two queues.
//
// only the caller knows if that is desirable.
//
oxdeq *oxdeq_copy(oxdeq *q, void *(*fcopy)(void *data)) {
    oxdeq *c = oxdeq_alloc();
    if (q && q->numberOfEntries) {
        q->e = q->left;
        while (q->e) {
            void *copyOfData = fcopy ? fcopy(q->e->data) : q->e->data;
            oxdeq_push_right(c, copyOfData);
            q->e = q->e->right;
        }
    }
    return c;
}

ssize_t oxdeq_number_of_entries(oxdeq *q) {
    return q ? q->numberOfEntries : 0;
}

void *oxdeq_peek_left(oxdeq *q) {
    return (q && q->left) ? q->left->data : 0;
}

void *oxdeq_peek_right(oxdeq *q) {
    return (q && q->right) ? q->right->data : 0;
}

void *oxdeq_pop_left(oxdeq *q) {
    void *v = (q && q->left) ? q->left->data : 0;
    if (v) {
        q->e = q->left;
        if (!q->left->right) {
            q->left  = q->right = 0;
        } else {
            q->left  = q->left->right;
        }
        q->numberOfEntries--;
        free(q->e);
        q->e = 0;
    }
    return v;
}

void *oxdeq_pop_right(oxdeq *q) {
    void *v = (q && q->right) ? q->right->data : 0;
    if (v) {
        q->e = q->right;
        if (!q->right->left) {
            q->left  = q->right = 0;
        } else {
            q->right = q->right->left;
        }
        q->numberOfEntries--;
        free(q->e);
        q->e = 0;
    }
    return v;
}

oxdeq *oxdeq_push_left(oxdeq *q, void *data) {
    if (q && data) {
        q->e = malloc(sizeof(*(q->e)));
        if (!q->e) {
            perror(__FUNCTION__);
            exit(2);
        }
        q->e->data  = data;
        q->e->left  = 0;
        q->e->right = q->left;
        if (!q->left) {
            q->left = q->right      = q->e;
        } else {
            q->left = q->left->left = q->e;
        }
        q->numberOfEntries++;
        q->e = 0;
    }
    return q;
}

oxdeq *oxdeq_push_right(oxdeq *q, void *data) {
    if (q && data) {
        q->e = malloc(sizeof(*(q->e)));
        if (!q->e) {
            perror(__FUNCTION__);
            exit(2);
        }
        q->e->data  = data;
        q->e->left  = q->right;
        q->e->right = 0;
        if (!q->right) {
            q->left  = q->right        = q->e;
        } else {
            q->right = q->right->right = q->e;
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
        q->e = q->left;
        q->t = q->right;

        while (q->e != q->t) {
            void *tmp  = q->e->data;
            q->e->data = q->t->data;
            q->t->data = tmp;
            
            if (q->e->right == q->t) {
                // the two ends have met, so we're done
                //
                break;
            }

            // haven't met, so move the ends towards the middle
            //
            q->e = q->e->right;
            q->t = q->t->left;
        }
        q->e = q->t = 0;
    }
    return q;
}
