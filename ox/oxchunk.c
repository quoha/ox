//
//  ox/oxchunk.c
//
//  Created by Michael Henderson on 10/2/14.
//  Copyright (c) 2014 Michael D Henderson. All rights reserved.
//

#include "oxchunk.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

oxchunk *oxchunk_factory(oxbuf *b) {
    // eat chunks while we're pointing at comments
    //
    oxchunk *c = oxchunk_pull_template_comment(b);
    while (c) {
        free(c);
        c = oxchunk_pull_template_comment(b);
    };

    // we're pointing at either text, template, or end of buffer
    //
    c = oxchunk_pull_text(b);
    if (!c) {
        c = oxchunk_pull_template(b);
    }

    return c;
}

// template starts at {{ and continues to }} or end-of-buffer,
// whichever comes first.
//
oxchunk *oxchunk_pull_template(oxbuf *b) {
    oxchunk *c = 0;
    
    // verify that we're looking at a template and not text
    //
    if (b->curr < b->endOfData && (b->curr[0] == '{' && b->curr[1] == '{')) {
        // TODO: buggy - doesn't accept }} inside of templates
        //
        int           foundClose = 0;
        int           line       = b->lineNumber;
        unsigned char *mark      = b->curr;
        oxbuf_advance_to_string(b, "}}");

        if (b->curr + 2 <= b->endOfData) {
            foundClose = 1;
        }

        int idx;
        for (idx = 0; idx < 2; idx++) {
            if (b->curr < b->endOfData) {
                b->curr++;
            }
        }

        c = malloc(sizeof(*c) + (b->curr - mark));
        if (!c) {
            perror(__FUNCTION__);
            exit(2);
        }

        c->kind = oxcTemplate;
        c->line = line;
        if (foundClose) {
            memcpy(c->data, mark + 2, b->curr - mark - 4);
            c->data[b->curr - mark - 4] = 0;
        } else {
            memcpy(c->data, mark + 2, b->curr - mark);
            c->data[b->curr - mark] = 0;
        }
    }
    return c;
}

// the {{/* ... */}} can span multiple lines
//
// the {{/**/}} comment swallows everything from the closing
// curly braces to the end-of-line, including the end--of-line.
//
// both will silently ignore missing terminators
//
oxchunk *oxchunk_pull_template_comment(oxbuf *b) {
    oxchunk *c = 0;
    
    // verify that we're looking at a template comment
    //
    if (b->curr < b->endOfData && (b->curr[0] == '{' && b->curr[1] == '{' && b->curr[2] == '/' && b->curr[3] == '*')) {
        int           line  = b->lineNumber;
        unsigned char *mark = b->curr;

        oxbuf_advance_to_string(b, "*/}}");
        int idx;
        for (idx = 0; idx < 4; idx++) {
            if (b->curr < b->endOfData) {
                b->curr++;
            }
        }

        if (b->curr - mark == 8) {
            // the {{/**/}} comment consumes the remainder of the line, too
            //
            while (b->curr < b->endOfData && b->curr[0] != '\n') {
                b->curr++;
            }
            if (b->curr < b->endOfData) {
                b->curr++;
            }
        }

        c = malloc(sizeof(*c) + (b->curr - mark));
        if (!c) {
            perror(__FUNCTION__);
            exit(2);
        }
        
        c->kind = oxcComment;
        c->line = line;
        memcpy(c->data, mark, (b->curr - mark));
    }

    return c;
}

oxchunk *oxchunk_pull_text(oxbuf *b) {
    oxchunk *c = 0;

    // verify that we're looking at text and not a template. if
    // it is text, then it goes to the start of the next template.
    //
    if (b->curr < b->endOfData && !(b->curr[0] == '{' && b->curr[1] == '{')) {
        int            line = b->lineNumber;
        unsigned char *mark = b->curr;
        oxbuf_advance_to_string(b, "{{");

        c = malloc(sizeof(*c) + (b->curr - mark));
        if (!c) {
            perror(__FUNCTION__);
            exit(2);
        }

        c->kind = oxcText;
        c->line = line;
        memcpy(c->data, mark, (b->curr - mark));
    }

    return c;
}
