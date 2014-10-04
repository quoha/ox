//
//  ox/oxparser.c
//
//  Created by Michael Henderson on 10/3/14.
//  Copyright (c) 2014 Michael D Henderson. All rights reserved.
//

#include "oxparser.h"
#include "oxbuf.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

oxcell *asname(oxcell *c);
int     istext(oxcell *c);

// create a copy of a string with the contents reversed.
//
// for example:
//   ""    returns ""
//   "o"   returns "o"
//   "qp"  returns "pq"
//   "abc" returns "cba"
//
static unsigned char *strrcpy(unsigned char *src, size_t length) {
    unsigned char *tgt = src ? malloc(length + 1) : 0;
    if (tgt) {
        unsigned char *p = tgt + length;
        *(p--) = 0;
        while (length--) {
            *(p--) = *(src++);
        }
    }
    return tgt;
}

oxtoken *oxtok_read(oxbuf *ib) {
    if (ib) {
        // skip comments and whitespace
        do {
            while (isspace(ib->curr[0])) {
                if (ib->curr[0] == '\n') {
                    ib->lineNumber++;
                }
                ib->curr++;
            }
            if (ib->curr[0] == '#' || ib->curr[1] == ';') {
                while (ib->curr[0] && ib->curr[0] != '\n') {
                    ib->curr++;
                }
            }
        } while (isspace(ib->curr[0]));
    }

    if (!ib || !ib->curr[0]) {
        oxtoken *t = malloc(sizeof(*t));
        if (!t) {
            perror(__FUNCTION__);
            exit(2);
        }
        t->kind       = oxTokEOF;
        t->line       = ib ? ib->lineNumber : -1;
        t->data[0]    = 0;
        t->value.text = 0;
        return t;
    }
    
    int            lineNumber  = ib->lineNumber;
    unsigned char *startLexeme = ib->curr++;
    
    if (*startLexeme == '(') {
        oxtoken *t = malloc(sizeof(*t) + 1);
        if (!t) {
            perror(__FUNCTION__);
            exit(2);
        }
        t->kind       = oxTokOpenParen;
        t->line       = lineNumber;
        t->data[0]    = *startLexeme;
        t->data[1]    = 0;
        t->value.text = 0;
        return t;
    }
    
    if (*startLexeme == ')') {
        oxtoken *t = malloc(sizeof(*t) + 1);
        if (!t) {
            perror(__FUNCTION__);
            exit(2);
        }
        t->kind       = oxTokCloseParen;
        t->line       = lineNumber;
        t->data[0]    = *startLexeme;
        t->data[1]    = 0;
        t->value.text = 0;
        return t;
    }
    
    const char escape      = '\\';
    const char singleQuote = '\'';
    const char doubleQuote = '"';
    
    // look for quoted text
    //
    if (*startLexeme == singleQuote || *startLexeme == doubleQuote) {
        // advance to the closing quote. accept the escape character in the string.
        //
        while (ib->curr[0] && ib->curr[0] != *startLexeme) {
            if (ib->curr[0] == '\n') {
                ib->lineNumber++;
            }
            if (ib->curr[0] == escape) {
                if (ib->curr[1]) {
                    ++ib->curr;
                } else {
                    // escape followed by end-of-buffer is bad
                    //
                    break;
                }
            }
            ++ib->curr;
        }
        
        // only accept valid strings. must have open/close quotes and end
        // at end-of-buffer or a space or an open/close parenthesis
        //
        if (ib->curr[0] == *startLexeme && (ib->curr[1] == 0 || ib->curr[1] == '(' || ib->curr[1] == ')' || isspace(ib->curr[1]))) {
            oxtoken *t = malloc(sizeof(*t) + (ib->curr - startLexeme));
            if (!t) {
                perror(__FUNCTION__);
                exit(2);
            }
            t->kind       = oxTokText;
            t->line       = lineNumber;
            t->value.text = (char *)(t->data);
            
            // copy the string into the new lexeme while getting rid
            // of the escape characters. make sure that our copy of
            // the data is terminated properly.
            //
            unsigned char *src = startLexeme + 1;
            unsigned char *dst = (unsigned char *)(t->data);
            while (src < ib->curr) {
                if (*src == escape) {
                    src++;
                    if (*src == 'n') {
                        *(dst++) = '\n';
                    } else if (*src == 't') {
                        *(dst++) = '\t';
                    } else {
                        *(dst++) = *src;
                    }
                    src++;
                } else {
                    *(dst++) = *(src++);
                }
            }
            *dst = 0;
            
            // skip past the closing quote
            //
            ib->curr++;
            
            return t;
        }
        
        // getting here means that the string wasn't terminated properly
        //
        printf("error:\t%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
        printf("\tunterminated quoted string\n");
        exit(2);
    }
    
    // look for raw quoted text, \(.*)"..."(*.)/, where the .* represents
    // optional balanced open and close alphanumeric text:
    //      \" "/
    //     \\" "//
    //      \' '/
    //    \ac' 'ca/
    //
    if (*startLexeme == escape) {
        // look for \(.*)" here
        //
        unsigned char *p = startLexeme + 1;
        while (*p && (*p == escape || isalnum(*p))) {
            p++;
        }

        if (*p == singleQuote || *p == doubleQuote) {
            // found a raw string. accept everything up to end of buffer
            // or the balanced closing text.
            //
            unsigned char *startRawText = p + 1;

            // create the close string prior to the check to ignore a
            // leading new-line in the raw text
            //
            unsigned char *closeString  = strrcpy(startLexeme, startRawText - startLexeme);
            if (!closeString) {
                perror(__FUNCTION__);
                exit(2);
            }
            
            // ignore the first raw character if a new-line,
            //
            if (*startRawText == '\n') {
                startRawText++;
            }

            // be clever and change the \ to / in the reversed string
            //
            unsigned char *p = closeString;
            while (*p) {
                if (*p == '/') {
                    *p = '\\';
                } else if (*p == '\\') {
                    *p = '/';
                }
                p++;
            }

            // find that close string in the input
            //
            ib->curr = startRawText;
            size_t lenCloseString = strlen((char *)closeString);
            while (ib->curr < ib->endOfData) {
                if (ib->curr[0] == '\n') {
                    ib->lineNumber++;
                }
                if (ib->curr[0] == closeString[0] && ib->curr[1] == closeString[1]) {
                    if (!memcmp(ib->curr, closeString, lenCloseString)) {
                        break;
                    }
                }
                ib->curr++;
            }

            unsigned char *endRawText = ib->curr;
            if (ib->curr[0] == closeString[0]) {
                // found close string in the buffer
                //
                ib->curr += lenCloseString;
                if (ib->curr >= ib->endOfData) {
                    ib->curr = ib->endOfData;
                }
            } else {
                // did not find close string in the buffer, so go to the end of the buffer
                //
                endRawText = ib->curr = ib->endOfData;
            }

            // the open and close delimiters never go in the token
            //
            oxtoken *t = malloc(sizeof(*t) + (endRawText - startRawText));
            if (!t) {
                perror(__FUNCTION__);
                exit(2);
            }
            t->kind       = oxTokText;
            t->line       = lineNumber;
            t->value.text = (char *)(t->data);
            memcpy(t->data, startRawText, endRawText - startRawText);
            t->data[endRawText - startRawText] = 0;

            return t;
        }
    }

    // look for an integer ([+-]d+) or number ([+-]d+.d+).
    //
    // note that if the text isn't a valid integer or number, we will
    // fall through to the symbol code.
    //
    //    1    is an integer, so is +1   or -1
    //    1.2  is a  number , so is +1.2 or -1.2
    //    1.   is a  symbol , so is   .2 or  1.2k
    //
    if (isdigit(*startLexeme) || ((*startLexeme == '-' || *startLexeme == '+') && isdigit(ib->curr[0]))) {
        while (isdigit(ib->curr[0])) {
            ++ib->curr;
        }

        if (!ib->curr[0] || isspace(ib->curr[0]) || ib->curr[0] == '(' || ib->curr[0] == ')') {
            oxtoken *t = malloc(sizeof(*t) + (ib->curr - startLexeme));
            if (!t) {
                perror(__FUNCTION__);
                exit(2);
            }
            t->kind          = oxTokInteger;
            t->line          = lineNumber;
            t->value.integer = atol((char *)startLexeme);
            memcpy(t->data, startLexeme, ib->curr - startLexeme);
            t->data[ib->curr - startLexeme] = 0;
            return t;
        }
        
        if (ib->curr[0] == '.' && isdigit(ib->curr[1])) {
            do {
                ++ib->curr;
            } while (isdigit(ib->curr[0]));

            if (!ib->curr[0] || isspace(ib->curr[0]) || ib->curr[0] == '(' || ib->curr[0] == ')') {
                oxtoken *t = malloc(sizeof(*t) + (ib->curr - startLexeme));
                if (!t) {
                    perror(__FUNCTION__);
                    exit(2);
                }
                t->kind       = oxTokReal;
                t->line       = lineNumber;
                t->value.real = strtod((char *)startLexeme, 0);
                memcpy(t->data, startLexeme, ib->curr - startLexeme);
                t->data[ib->curr - startLexeme] = 0;
                return t;
            }
        }
    }
    
    // look for a symbol. a symbol is any string of characters that doesn't
    // contain a blank or parentheses.
    //
    while (ib->curr[0] && !(isspace(ib->curr[0]) || ib->curr[0] == '(' || ib->curr[0] == ')')) {
        ++ib->curr;
    }

    oxtoken *t = malloc(sizeof(*t) + (ib->curr - startLexeme));
    if (!t) {
        perror(__FUNCTION__);
        exit(2);
    }
    t->kind       = oxTokSymbol;
    t->line       = lineNumber;
    t->value.name = (char *)(t->data);
    memcpy(t->data, startLexeme, ib->curr - startLexeme);
    t->data[ib->curr - startLexeme] = 0;
    
    return t;
}

const char *oxtok_toktype(oxtoken *t) {
    if (t) {
        switch (t->kind) {
            case oxTokCloseParen:
                return "paren_c";
            case oxTokEOF:
                return "**eof***";
            case oxTokInteger:
                return "integer";
            case oxTokOpenParen:
                return "paren_o";
            case oxTokReal:
                return "real   ";
            case oxTokSymbol:
                return "symbol ";
            case oxTokText:
                return "text   ";
        }
    }
    return "**null**";
}

static oxcell *nill = 0;

long asinteger(oxcell *c) {
    return c->u.atom.u.integer;
}

oxcell *asname(oxcell *c) {
    return c->u.atom.u.name;
}

double asreal(oxcell *c) {
    return c->u.atom.u.real;
}

const char *astext(oxcell *c) {
    if (istext(c)) {
        return c->u.atom.u.text;
    }
    return "";
}

oxcell *car(oxcell *c) {
    return c->u.cons.car;
}

oxcell *cdr(oxcell *c) {
    return c->u.cons.cdr;
}

int iscons(oxcell *c) {
    return c->isCons;
}

int istext(oxcell *c) {
    return !iscons(c) && c->u.atom.kind == eAtomText;
}

int isnil(oxcell *c) {
    return c == nill;
}

enum atomType type(oxcell *c) {
    return c->u.atom.kind;
}

oxcell *oxcell_set_car(oxcell *c, oxcell *t) {
    c->u.cons.car = t;
    return c;
}

oxcell *oxcell_set_cdr(oxcell *c, oxcell *t) {
    c->u.cons.cdr = t;
    return c;
}

oxcell *oxcell_set_integer(oxcell *c, long integer) {
    oxcell_set_type(c, eAtomInteger);
    c->isCons = 0;
    c->u.atom.u.integer = integer;
    return c;
}

oxcell *oxcell_set_real(oxcell *c, double real) {
    oxcell_set_type(c, eAtomReal);
    c->isCons = 0;
    c->u.atom.u.real = real;
    return c;
}

oxcell *oxcell_set_name(oxcell *c, oxcell *name) {
    oxcell_set_type(c, eAtomVariable);
    c->isCons = 0;
    c->u.atom.u.name = name;
    return c;
}

oxcell *oxcell_set_text(oxcell *c, const char *text) {
    oxcell_set_type(c, eAtomText);
    c->isCons = 0;
    c->u.atom.u.text = (char *)text;
    return c;
}

oxcell *oxcell_set_type(oxcell *c, enum atomType k) {
    c->u.atom.kind = k;
    return c;
}

oxcell *oxcell_alloc(oxcell *first, oxcell *second) {
    oxcell *cell = malloc(sizeof(*cell));
    if (!cell) {
        perror(__FUNCTION__);
        exit(2);
    }
    cell->isCons = 1;
    oxcell_set_car(cell, first );
    oxcell_set_cdr(cell, second);
    return cell;
}

oxcell *oxcell_alloc_cons(oxcell *c, oxcell *t) {
    return oxcell_alloc(c, t);
}

oxcell *oxcell_alloc_cstring(const char *cstring) {
    oxcell *c = oxcell_alloc(nill, nill);
    oxcell_set_text(c, strdup(cstring));
    if (!c->u.atom.u.text) {
        perror(__FUNCTION__);
        exit(2);
    }
    return c;
}

oxcell *oxcell_alloc_integer(long integer) {
    oxcell *c = oxcell_alloc(nill, nill);
    oxcell_set_integer(c, integer);
    return c;
}

oxcell *oxcell_alloc_real(double real) {
    oxcell *c = oxcell_alloc(nill, nill);
    oxcell_set_real(c, real);
    return c;
}

oxcell *oxcell_alloc_text(const char *text, size_t length) {
    oxcell *c = oxcell_alloc(nill, nill);
    oxcell_set_text(c, malloc(length + 1));
    if (!c->u.atom.u.text) {
        perror(__FUNCTION__);
        exit(2);
    }
    memcpy(c->u.atom.u.text, text, length);
    c->u.atom.u.text[length] = 0;
    return c;
}

oxcell *oxcell_alloc_variable(const char *name) {
    oxcell *c = oxcell_alloc(nill, nill);
    c->isCons = 0;
    oxcell_set_type(c, eAtomVariable);
    oxcell_set_name(c, oxcell_alloc_cstring(name));
    return c;
}

void oxexpr_print(oxcell *sexp) {
    if (isnil(sexp)) {
        printf("()");
    } else if (iscons(sexp)) {
        printf("(");
        oxexpr_print(car(sexp));
        sexp = cdr(sexp);
        while (!isnil(sexp) && iscons(sexp)) {
            printf(" ");
            oxexpr_print(car(sexp));
            sexp = cdr(sexp);
        }
        printf( ")");
    } else {
        switch (type(sexp)) {
            case eAtomFunc:
                printf("#CFUNC");
                break;
            case eAtomInteger:
                printf("%ld", asinteger(sexp));
                break;
            case eAtomReal:
                printf("%g", asreal(sexp));
                break;
            case eAtomSymbol:
                printf("#SYMBOL");
                break;
            case eAtomText:
                printf("\"%s\"", astext(sexp));
                break;
            case eAtomVariable:
                printf("%s", astext(asname(sexp)));
                break;
        }
    }
}


oxcell *oxexpr_read(oxbuf *ib) {
    oxtoken *t = oxtok_read(ib);
    
    switch (t->kind) {
        case oxTokCloseParen:
            printf("error:\t%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
            printf("error:\tunexpected ')' in input\n");
            exit(1);
        case oxTokEOF:
            return 0;
        case oxTokInteger:
            return oxcell_alloc_integer(t->value.integer);
        case oxTokOpenParen:
            return oxexpr_read_tail(ib);
        case oxTokReal:
            return oxcell_alloc_real(t->value.real);
        case oxTokSymbol:
            return oxcell_alloc_variable(t->value.name);
        case oxTokText:
            return oxcell_alloc_cstring(t->value.text);
    }
    
    if (1) {
        printf("error:\t%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
        printf("error:\tinternal error - unhandled token %d\n", t->kind);
        exit(2);
    }

    return 0;
}

oxcell *oxexpr_read_tail(oxbuf *ib) {
    oxtoken *t = oxtok_read(ib);
    switch (t->kind) {
        case oxTokCloseParen:
            return nill;
        case oxTokEOF:
            printf("error:\t%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
            printf("error:\tunexpected end of input\n");
            exit(1);
        case oxTokInteger:
            return oxcell_alloc_cons(oxcell_alloc_integer(t->value.integer), oxexpr_read_tail(ib));
        case oxTokReal:
            return oxcell_alloc_cons(oxcell_alloc_real(t->value.real), oxexpr_read_tail(ib));
        case oxTokOpenParen:
            return oxcell_alloc_cons(oxexpr_read_tail(ib), oxexpr_read_tail(ib));
        case oxTokSymbol:
            return oxcell_alloc_cons(oxcell_alloc_variable(t->value.name), oxexpr_read_tail(ib));
        case oxTokText:
            return oxcell_alloc_cons(oxcell_alloc_cstring(t->value.text), oxexpr_read_tail(ib));
    }
    
    if (1) {
        printf("error:\t%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
        printf("error:\tinternal error - unhandled token %d\n", t->kind);
        exit(2);
    };
    
    return 0;
}
