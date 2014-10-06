//
//  ox/oxparser.h
//
//  Created by Michael Henderson on 10/3/14.
//  Copyright (c) 2014 Michael D Henderson. All rights reserved.
//

#ifndef __ox__oxparser__
#define __ox__oxparser__

#include "oxbuf.h"
#include "oxtype.h"

oxcell     *oxexpr_read(oxbuf *ib);
oxcell     *oxexpr_read_tail(oxbuf *ib);
oxtoken    *oxtok_read(oxbuf *ib);
const char *oxtok_toktype(oxtoken *t);

#endif /* defined(__ox__oxparser__) */
