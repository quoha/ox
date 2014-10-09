//
//  ox/oxprim.h
//
//  Created by Michael Henderson on 10/6/14.
//  Copyright (c) 2014 Michael D Henderson. All rights reserved.
//

#ifndef __ox__oxprim__
#define __ox__oxprim__

#include "oxtype.h"

oxcell *oxf_first(oxcell *args, oxcell *env);
oxcell *oxf_lambda(oxcell *args, oxcell *env);
oxcell *oxf_list(oxcell *args, oxcell *env);
oxcell *oxf_noop(oxcell *args, oxcell *env);
oxcell *oxf_quote(oxcell *args, oxcell *env);
oxcell *oxf_rest(oxcell *args, oxcell *env);

oxcell *oxf_glyph_add(oxcell *args, oxcell *env);
oxcell *oxf_glyph_subtract(oxcell *args, oxcell *env);

#endif /* defined(__ox__oxprim__) */
