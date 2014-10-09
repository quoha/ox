//
//  ox/oxbuf.c
//
//  Created by Michael Henderson on 10/2/14.
//  Copyright (c) 2014 Michael D Henderson. All rights reserved.
//
#if 0
#include "oxbuf.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/stat.h>

// this function loads memory into the buffer with no bounds
// checking.
//
oxbuf *oxbuf_from_buffer(const unsigned char *s, ssize_t length) {
    // create data for the size
    //
    oxbuf *d = malloc(sizeof(*d) + length);
    if (!d) {
        perror(__FUNCTION__);
        exit(2);
    }
    
    d->curr      = d->data;
    d->endOfData = d->data + length;
    d->lineNumber = 0;

    memcpy(d->data, s, length);
    d->data[length] = 0;
    
    return d;
}

// this function loads an entire C-string into a buffer
//
oxbuf *oxbuf_from_cstring(const char *s) {
    return oxbuf_from_buffer((const unsigned char *)(s ? s : ""), s ? strlen(s) : 0);
}

// this function reads the entire file into a buffer
//
oxbuf *oxbuf_from_file(const char *fileName) {
    oxbuf *d = 0;
    if (fileName) {
        // stat the file
        struct stat sb;
        if (stat(fileName, &sb) != 0) {
            perror(fileName);
            exit(2);
        }
        
        // create data for the file size
        d = malloc(sizeof(*d) + sb.st_size);
        if (!d) {
            perror(__FUNCTION__);
            exit(2);
        }
        
        // read in that much data
        FILE *fp = fopen(fileName, "r");
        if (!(fp && fread(d->data, 1, sb.st_size, fp) == sb.st_size)) {
            perror(fileName);
            exit(2);
        }
        fclose(fp);
        
        d->curr = d->data;
        d->endOfData = d->data + sb.st_size;
        d->endOfData[0] = 0;
        d->lineNumber = 0;
    }
    
    return d;
}

oxbuf *oxbuf_from_oxbuf(oxbuf *s) {
    oxbuf *d = 0;
    if (!s) {
        d = oxbuf_from_cstring(0);
    } else {
        size_t len = d->endOfData - d->curr;
        
        // create data for the size
        //
        d = malloc(sizeof(*d) + len);
        if (!d) {
            perror(__FUNCTION__);
            exit(2);
        }
        
        d->curr       = d->data + (s->curr - s->data);
        d->endOfData  = d->data + len;
        d->lineNumber = s->lineNumber;
        
        memcpy(d->data, s->data, s->endOfData - s->data + 1);
    }
    
    return d;
}

//
//
oxbuf *oxbuf_advance_to_string(oxbuf *b, const char *string) {
    if (b && string && *string) {
        size_t len = strlen(string);
        while (b->curr < b->endOfData && strncmp(string, (const char *)(b->curr), len)) {
            if (b->curr[0] == '\n') {
                b->lineNumber++;
            }
            b->curr++;
        }
    }
    return b;
}

// remove all 0xFF bytes from the buffer
//
// note - scans from s->curr forward
// note - will reset s->curr if needed
//
oxbuf *oxbuf_delete_ff(oxbuf *s) {
    if (s) {
        unsigned char *originalEndOfData = s->endOfData;
        
        // find the first 0xFF character in the buffer
        //
        unsigned char *src = s->curr;
        while (src < s->endOfData && *src != 0xFF) {
            src++;
        }
        
        // remove the unwanted data from the buffer by
        // shifting the rest of the buffer forward
        //
        unsigned char *tgt = src;
        while (src < s->endOfData) {
            if (*src != 0xFF) {
                *(tgt++) = *src;
            }
            src++;
        }
        
        // make sure that s->curr and s->endOfData remain valid
        //
        s->endOfData = tgt;
        if (!(s->curr < s->endOfData)) {
            s->curr = s->endOfData;
        }
        
        // as a courtesy, nul out the rest of the original buffer.
        //
        if (tgt < originalEndOfData) {
            memset(tgt, 0, originalEndOfData - tgt);
        }
    }
    
    return s;
}

// change carriage-return + new-line combination
// into just new-line. as a side-effect, removes
// all 0xFF bytes, too. sorry about that.
//
// note - scans from s->curr forward
// note - will reset s->curr if needed
//
oxbuf *oxbuf_force_unix_eol(oxbuf *s) {
    if (s) {
        // replace carriage-return + new-line combination
        // with nul + new-line
        //
        unsigned char *src = s->curr;
        while (src < s->endOfData) {
            if (*src == '\r' && *(src + 1) == '\n') {
                *src = 0xFF;
            }
            src++;
        }
    }
    
    return oxbuf_delete_ff(s);
}

// remove all characters from the buffer that are not
// in the provided whitelist
//
// note - scans from s->curr forward
// note - will reset s->curr if needed
//
oxbuf *oxbuf_keep_whitelist(oxbuf *s, oxwhtlst *w) {
    if (s && w) {
        unsigned char *originalEndOfData = s->endOfData;
        
        // find the first unwanted character in the buffer
        //
        unsigned char *src = s->curr;
        while (src < s->endOfData && w->map[*src]) {
            src++;
        }
        
        // remove the unwanted data from the buffer by
        // shifting the rest of the buffer forward
        //
        unsigned char *tgt = src;
        while (src < s->endOfData) {
            if (w->map[*src]) {
                *(tgt++) = *src;
            }
            src++;
        }
        
        // make sure that s->curr remains valid
        //
        s->endOfData = tgt;
        if (!(s->curr < s->endOfData)) {
            s->curr = s->endOfData;
        }
        
        // as a courtesy, nul out the rest of the original buffer.
        //
        if (tgt < originalEndOfData) {
            memset(tgt, 0, originalEndOfData - tgt);
        }
    }
    
    return s;
}

// remove carriage-return characters from the buffer
//
// note - scans from s->curr forward
// note - will reset s->curr if needed
//
oxbuf *oxbuf_remove_carriage_returns(oxbuf *s) {
    oxwhtlst w;
    memset(&w, -1, sizeof(w));
    w.map['\r'] = 0;
    
    return oxbuf_keep_whitelist(s, &w);
}

// remove non-ascii (aka binary) data from the buffer
//
// note - scans from s->curr forward
// note - will reset s->curr if needed
//
oxbuf *oxbuf_remove_non_ascii(oxbuf *s) {
    int     idx;
    oxwhtlst w;
    for (idx = 0; idx < 256; idx++) {
        w.map[idx] = isascii(idx);
    }
    
    return oxbuf_keep_whitelist(s, &w);
}

// map characters per the map (we're subverting the whitelist
// structure here). the map is expected to hold the translated
// character in it. for example, to change 'a' to 'A', set
// w->map['a'] = 'A'.
//
// note - scans from s->curr forward
// note - will reset s->curr if needed
//
oxbuf *oxbuf_replace_char(oxbuf *s, oxwhtlst *w) {
    if (s && w) {
        unsigned char *p = s->curr;
        while (p < s->endOfData) {
            *p = w->map[*p];
            p++;
        }
    }
    return s;
}

// trim trailing blanks from all lines in the buffer.
//
// will not delete empty lines. does not work on lines that end with
// carriage-return + new-lines. only works for unix style EOL.
//
// note - scans from s->curr forward
// note - will reset s->curr if needed
//
oxbuf *oxbuf_rtrim_lines(oxbuf *s) {
    int     idx;
    oxwhtlst w;
    for (idx = 0; idx < 256; idx++) {
        w.map[idx] = !isblank(idx);
    }
    
    return oxbuf_trim_trailing_map(s, &w);
}

// trim a set of trailing characters from all lines in the buffer.
// (we're subverting the whitelist structure again).
//
// for a traditional "rtrim" functionality, set your whitelist to
// keep everything but spaces and tabs.
//
// will not delete empty lines. does not work on lines that end with
// carriage-return + new-lines. only works for unix style EOL.
//
// note - scans from s->curr forward
// note - will reset s->curr if needed
//
oxbuf *oxbuf_trim_trailing_map(oxbuf *s, oxwhtlst *w) {
    if (s) {
        // skip blank lines
        //
        unsigned char *startOfLine = s->curr;
        while (*startOfLine == '\n' && startOfLine < s->endOfData) {
            startOfLine++;
        }
        
        while (startOfLine < s->endOfData) {
            // find end of the current line
            //
            unsigned char *endOfLine = startOfLine;
            while (endOfLine < s->endOfData && *endOfLine != '\n') {
                endOfLine++;
            }
            
            if (*endOfLine == '\n') {
                // set the trailing characters to 0xFF, which we
                // will remove when we are done with the entire
                // buffer
                //
                unsigned char *p = endOfLine - 1;
                while (p >= startOfLine && !w->map[*p]) {
                    *(p--) = 0xFF;
                }
            }
            
            // skip blank lines
            //
            startOfLine = endOfLine;
            while (*startOfLine == '\n' && startOfLine < s->endOfData) {
                startOfLine++;
            }
        }
    }
    
    return oxbuf_delete_ff(s);
}
#endif