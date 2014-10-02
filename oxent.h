//
//  ox/oxent.h
//
//  Created by Michael Henderson on 10/2/14.
//  Copyright (c) 2014 Michael D Henderson. All rights reserved.
//

#ifndef __ox__oxent__
#define __ox__oxent__

typedef struct oxent {
    struct oxent *prev;
    struct oxent *next;
    struct oxval *value;
} oxent;

oxent *oxent_new(void);

#endif /* defined(__ox__oxent__) */
