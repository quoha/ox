//
//  ox/main.c
//
//  Created by Michael Henderson on 10/1/14.
//  Copyright (c) 2014 Michael D Henderson. All rights reserved.
//

#include "local.h"

#include <stdio.h>
#include <string.h>

int main(int argc, const char * argv[]) {
    oxwhtlst w;
    memset(&w, -1, sizeof(w));

    oxbuf *a = oxbuf_from_cstring("hello, world  \nthis is a test\n");

    oxdeq *q = oxdeq_alloc();
    oxdeq_reverse(q);

    printf("%s\n", a->data);

    return 0;
}
