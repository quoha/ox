//
//  ox/oxbuf.h
//
//  Created by Michael Henderson on 10/2/14.
//  Copyright (c) 2014 Michael D Henderson. All rights reserved.
//

#ifndef __ox__oxbuf__
#define __ox__oxbuf__

#include <stdlib.h>

// hold a buffer in memory
//
typedef struct oxbuf {
    unsigned char *curr;
    unsigned char *endOfData;
    int           lineNumber;
    unsigned char data[1];
} oxbuf;

// provide a character set map for white lists and translating data
//
typedef struct oxwhtlst {
    unsigned char map[256];
} oxwhtlst;

// functions to create a buffer
//
oxbuf *oxbuf_from_buffer(const unsigned char *s, ssize_t length);
oxbuf *oxbuf_from_cstring(const char *s);
oxbuf *oxbuf_from_file(const char *fileName);
oxbuf *oxbuf_from_oxbuf(oxbuf *s);

// functions to transform the buffer
//
oxbuf *oxbuf_delete_ff(oxbuf *s);
oxbuf *oxbuf_force_unix_eol(oxbuf *s);
oxbuf *oxbuf_keep_whitelist(oxbuf *s, oxwhtlst *w);
oxbuf *oxbuf_remove_carriage_returns(oxbuf *s);
oxbuf *oxbuf_remove_non_ascii(oxbuf *s);
oxbuf *oxbuf_replace_char(oxbuf *s, oxwhtlst *w);
oxbuf *oxbuf_rtrim_lines(oxbuf *s);
oxbuf *oxbuf_trim_trailing_map(oxbuf *s, oxwhtlst *w);

// helper functions
//
oxbuf *oxbuf_advance_to_string(oxbuf *b, const char *string);

#endif /* defined(__ox__oxbuf__) */
