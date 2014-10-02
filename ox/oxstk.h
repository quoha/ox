//
//  ox/oxstk.h
//
//  Created by Michael Henderson on 10/2/14.
//  Copyright (c) 2014 Michael D Henderson. All rights reserved.
//

#ifndef __ox__oxstk__
#define __ox__oxstk__

#include "oxent.h"

typedef struct oxstk {
    struct oxent *top;
    struct oxent *bottom;
    int           height;
} oxstk;

oxstk *oxstk_new(void);

#endif /* defined(__ox__oxstk__) */
