//
//  ox/oxbuf.h
//
//  Created by Michael Henderson on 10/2/14.
//  Copyright (c) 2014 Michael D Henderson. All rights reserved.
//

#ifndef __ox__oxbuf__
#define __ox__oxbuf__

#include <stdio.h>

// this structure should hold the entire file
//
typedef struct oxbuf {
    unsigned char *curr;
    unsigned char *endOfData;
    int           lineNumber;
    unsigned char data[1];
} oxbuf;

typedef struct oxwhtlst {
    unsigned char map[256];
} oxwhtlst;


oxbuf *oxbuf_delete_ff(oxbuf *s);
oxbuf *oxbuf_force_unix_eol(oxbuf *s);
oxbuf *oxbuf_from_cstring(const char *s);
oxbuf *oxbuf_from_file(const char *fileName);
oxbuf *oxbuf_from_oxbuf(oxbuf *s);
oxbuf *oxbuf_keep_whitelist(oxbuf *s, oxwhtlst *w);
oxbuf *oxbuf_remove_carriage_returns(oxbuf *s);
oxbuf *oxbuf_remove_non_ascii(oxbuf *s);
oxbuf *oxbuf_replace_char(oxbuf *s, oxwhtlst *w);
oxbuf *oxbuf_rtrim_lines(oxbuf *s);
oxbuf *oxbuf_trim_trailing_map(oxbuf *s, oxwhtlst *w);

#endif /* defined(__ox__oxbuf__) */
