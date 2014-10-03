//
//  ox/oxchunk.h
//
//  Created by Michael Henderson on 10/2/14.
//  Copyright (c) 2014 Michael D Henderson. All rights reserved.
//

#ifndef __ox__oxchunk__
#define __ox__oxchunk__

#include "oxbuf.h"

typedef struct oxchunk {
    int           line;
    enum { oxcComment, oxcTemplate, oxcText } kind;
    char data[1];
} oxchunk;

oxchunk *oxchunk_factory(oxbuf *b);
oxchunk *oxchunk_pull_template(oxbuf *b);
oxchunk *oxchunk_pull_template_comment(oxbuf *b);
oxchunk *oxchunk_pull_text(oxbuf *b);

#endif /* defined(__ox__oxchunk__) */
