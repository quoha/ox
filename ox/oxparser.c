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

oxcell *nill   = 0;
oxcell *myEnv  = 0;

int oxinit(void) {
    if (!nill) {
        nill = oxcell_alloc();
        nill->cons.car = nill;
        nill->cons.cdr = nill;
    }

    // add the built ins to the environment
    //
    myEnv = oxcell_alloc_cons(oxcell_alloc_symbol(oxcell_alloc_cstring("car"   ), oxcell_alloc_func(oxf_car   )), nill );
    myEnv = oxcell_alloc_cons(oxcell_alloc_symbol(oxcell_alloc_cstring("cdr"   ), oxcell_alloc_func(oxf_cdr   )), myEnv);
    myEnv = oxcell_alloc_cons(oxcell_alloc_symbol(oxcell_alloc_cstring("cons"  ), oxcell_alloc_func(oxf_cons  )), myEnv);
    myEnv = oxcell_alloc_cons(oxcell_alloc_symbol(oxcell_alloc_cstring("lambda"), oxcell_alloc_func(oxf_lambda)), myEnv);
    myEnv = oxcell_alloc_cons(oxcell_alloc_symbol(oxcell_alloc_cstring("noop"  ), oxcell_alloc_func(oxf_noop  )), myEnv);
    myEnv = oxcell_alloc_cons(oxcell_alloc_symbol(oxcell_alloc_cstring("quote" ), oxcell_alloc_func(oxf_quote )), myEnv);

    return -1;
}

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
    
    // look for a name. a name is any string of characters that doesn't
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
    t->kind       = oxTokName;
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
            case oxTokName:
                return "name   ";
            case oxTokReal:
                return "real   ";
            case oxTokText:
                return "text   ";
        }
    }
    return "**null**";
}

oxcell *car(oxcell *c) {
    if (c) {
        if (c->kind == octCons) {
            return c->cons.car;
        }
    }
    return 0;
}

oxcell *cdr(oxcell *c) {
    if (c) {
        if (c->kind == octCons) {
            return c->cons.cdr;
        }
    }
    return 0;
}

int isatom(oxcell *c) {
    if (c) {
        if (c->kind == octAtom) {
            return -1;
        }
    }
    return 0;
}

int iscons(oxcell *c) {
    if (c) {
        if (!isatom(c)) {
            return -1;
        }
    }
    return 0;
}

int isnil(oxcell *c) {
    //return (c == nill) ? -1 : 0;
    if (c == nill) {
        return -1;
    }
    return 0;
}

int issymbol(oxcell *c) {
    if (c) {
        if (c->kind == octCons) {
            if (c->isSymbol) {
                return -1;
            }
        }
    }
    return 0;
}

int istext(oxcell *c) {
    if (c) {
        if (isatom(c)) {
            if (c->atom->kind == eAtomText) {
                return -1;
            }
        }
    }
    return 0;
}

oxcell *oxcell_get_car(oxcell *c) {
    if (iscons(c)) {
        return c->cons.car;
    }
    return 0;
}

oxcell *oxcell_get_cdr(oxcell *c) {
    if (iscons(c)) {
        return c->cons.cdr;
    }
    return 0;
}

long oxcell_get_integer(oxcell *c) {
    if (c) {
        if (c->kind == octAtom) {
            if (c->atom->kind == eAtomInteger) {
                return c->atom->integer;
            }
        }
    }
    return 0;
}

oxcell *oxcell_get_name(oxcell *c) {
    if (issymbol(c)) {
        return oxcell_get_car(c);
    }
    return nill;
}

double oxcell_get_real(oxcell *c) {
    if (c) {
        if (c->kind == octAtom) {
            if (c->atom->kind == eAtomReal) {
                return c->atom->real;
            }
        }
    }
    return 0.0;
}

const char *oxcell_get_text(oxcell *c) {
    if (c) {
        if (c->kind == octAtom) {
            if (c->atom->kind == eAtomText) {
                return c->atom->text;
            }
        }
    }
    return "";
}

unsigned long oxcell_get_timestamp(oxcell *c) {
    if (c) {
        if (c->kind == octAtom) {
            if (c->atom->kind == eAtomTime) {
                return c->atom->real;
            }
        }
    }
    return 0;
}

oxcell *oxcell_get_value(oxcell *c) {
    if (issymbol(c)) {
        return oxcell_get_cdr(c);
    }
    return nill;
}

oxcell *oxcell_set_car(oxcell *c, oxcell *t) {
    if (c) {
        if (c->kind == octCons) {
            c->cons.car = t;
            return t;
        }
    }

    if (1) {
        printf("error: internal error - setCar on non cons");
        exit(2);
    }
    return c;
}

oxcell *oxcell_set_cdr(oxcell *c, oxcell *t) {
    if (c) {
        if (c->kind == octCons) {
            c->cons.cdr = t;
            return t;
        }
    }

    if (1) {
        printf("error: internal error - setCdr on non cons");
        exit(2);
    }
    return c;
}

oxcell *oxcell_alloc(void) {
    oxcell *c = malloc(sizeof(*c));
    if (!c) {
        perror(__FUNCTION__);
        exit(2);
    }

    c->kind         = octCons;
    c->atom         = 0;
    c->isSymbol     = 0;
    c->cons.car     = 0;
    c->cons.cdr     = 0;
    
    return c;
}

oxcell *oxcell_alloc_cons(oxcell *car, oxcell *cdr) {
    oxcell *c = oxcell_alloc();
    oxcell_set_car(c, car);
    oxcell_set_cdr(c, cdr);
    return c;
}

oxcell *oxcell_alloc_cstring(const char *cstring) {
    return oxcell_alloc_text(cstring, strlen(cstring));
}

oxcell *oxcell_alloc_func(oxcell *(func)(oxcell *arg, oxcell *env)) {
    oxcell *c           = oxcell_alloc();
    c->kind             = octAtom;

    c->atom = malloc(sizeof(*(c->atom)));
    if (!c->atom) {
        perror(__FUNCTION__);
        exit(2);
    }
    memset(c->atom, 0, sizeof(*(c->atom)));

    c->atom->kind      = eAtomFunc;
    c->atom->func      = func;
    c->atom->integer   = 0;
    c->atom->real      = 0.0;
    c->atom->text      = 0;
    c->atom->timestamp = 0;

    return c;
}

oxcell *oxcell_alloc_integer(long integer) {
    oxcell *c           = oxcell_alloc();
    c->kind             = octAtom;

    c->atom = malloc(sizeof(*(c->atom)));
    if (!c->atom) {
        perror(__FUNCTION__);
        exit(2);
    }
    memset(c->atom, 0, sizeof(*(c->atom)));
    
    c->atom->kind      = eAtomInteger;
    c->atom->func      = 0;
    c->atom->integer   = integer;
    c->atom->real      = 0.0;
    c->atom->text      = 0;
    c->atom->timestamp = 0;
    
    return c;
}

oxcell *oxcell_alloc_real(double real) {
    oxcell *c           = oxcell_alloc();
    c->kind             = octAtom;

    c->atom = malloc(sizeof(*(c->atom)));
    if (!c->atom) {
        perror(__FUNCTION__);
        exit(2);
    }
    memset(c->atom, 0, sizeof(*(c->atom)));
    
    c->atom->kind      = eAtomReal;
    c->atom->func      = 0;
    c->atom->real      = real;
    c->atom->real      = 0.0;
    c->atom->text      = 0;
    c->atom->timestamp = 0;

    return c;
}

oxcell *oxcell_alloc_text(const char *text, size_t length) {
    oxcell *c           = oxcell_alloc();
    c->kind             = octAtom;

    c->atom = malloc(sizeof(*(c->atom)));
    if (!c->atom) {
        perror(__FUNCTION__);
        exit(2);
    }
    memset(c->atom, 0, sizeof(*(c->atom)));

    c->atom->kind      = eAtomText;
    c->atom->func      = 0;
    c->atom->integer   = 0;
    c->atom->real      = 0.0;
    c->atom->text      = malloc(length + 1);
    c->atom->timestamp = 0;

    if (!c->atom->text) {
        perror(__FUNCTION__);
        exit(2);
    }
    memcpy(c->atom->text, text, length);
    c->atom->text[length] = 0;

    return c;
}

oxcell *oxcell_alloc_timestamp(unsigned long timestamp) {
    oxcell *c           = oxcell_alloc();
    c->kind             = octAtom;
    
    c->atom = malloc(sizeof(*(c->atom)));
    if (!c->atom) {
        perror(__FUNCTION__);
        exit(2);
    }
    memset(c->atom, 0, sizeof(*(c->atom)));
    
    c->atom->kind      = eAtomTime;
    c->atom->func      = 0;
    c->atom->integer   = 0;
    c->atom->real      = 0.0;
    c->atom->text      = 0;
    c->atom->timestamp = timestamp;
    
    return c;
}

oxcell *oxcell_alloc_symbol(oxcell *name, oxcell *value) {
    oxcell *c   = oxcell_alloc();
    c->isSymbol = -1;
    oxcell_set_car(c, name);
    oxcell_set_cdr(c, value);
    return c;
}

void oxexpr_print(oxcell *sexp) {
    if (!sexp) {
        // do nothing?
    } else if (isnil(sexp)) {
        printf("()");
    } else if (isatom(sexp)) {
        switch (sexp->atom->kind) {
            case eAtomFunc:
                printf("#CFUNC");
                return;
            case eAtomInteger:
                printf("%ld", oxcell_get_integer(sexp));
                return;
            case eAtomReal:
                printf("%g", oxcell_get_real(sexp));
                return;
            case eAtomText:
                printf("\"%s\"", oxcell_get_text(sexp));
                return;
            case eAtomTime:
                printf("*time*");
                return;
        }
    } else if (issymbol(sexp)) {
        printf("%s", oxcell_get_text(oxcell_get_name(sexp)));
        return;
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
        return;
    }
    printf("error: internal error - cell not atom/list/symbol\n");
    exit(2);
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
        case oxTokName:
            return oxcell_alloc_symbol(oxcell_alloc_text(t->value.name, strlen(t->value.name)), 0);
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
        case oxTokName:
            return oxcell_alloc_cons(oxcell_alloc_symbol(oxcell_alloc_text(t->value.name, strlen(t->value.name)), 0), oxexpr_read_tail(ib));
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

oxcell *oxf_car(oxcell *args, oxcell *env) {
    if (args) {
        if (args->kind == octCons) {
            return args->cons.car;
        }
    }
    return 0;
}

oxcell *oxf_cdr(oxcell *args, oxcell *env) {
    if (args) {
        if (args->kind == octCons) {
            return args->cons.cdr;
        }
    }
    return 0;
}

oxcell *oxf_cons(oxcell *args, oxcell *env) {
    oxcell *car = oxf_car(args, env);
    oxcell *cdr = oxf_car(oxf_cdr(args, env), env);
    oxcell *c   = oxcell_alloc();
    oxcell_set_car(c, car);
    oxcell_set_cdr(c, cdr);
    return c;
}

oxcell *oxf_lambda(oxcell *args, oxcell *env) {
    return nill;
}

oxcell *oxf_noop(oxcell *args, oxcell *env) {
    return nill;
}

oxcell *oxf_quote(oxcell *args, oxcell *env) {
    return nill;
}

int oxcell_eq_text(oxcell *c1, oxcell *c2) {
    if (c1 && c2) {
        if (isatom(c1) && isatom(c2)) {
            if (istext(c1) && istext(c2)) {
                if (c1 != nill && c2 != nill) {
                    const char *p1 = oxcell_get_text(c1);
                    const char *p2 = oxcell_get_text(c2);
                    if (p1 && p2) {
                        if (p1 == p2) {
                            return -1;
                        } else if (strcmp(p1, p2) == 0) {
                            return -1;
                        }
                    }
                }
            }
        }
    }
    return 0;
}

oxcell *oxsym_lookup(oxcell *name, oxcell *env) {
    for (; env != nill; env = oxcell_get_cdr(env)) {
        oxcell *entry = oxcell_get_car(env);
        if (issymbol(entry)) {
            if (oxcell_eq_text(name, oxcell_get_name(entry))) {
                return entry;
            }
        }
    }

    return nill;
}

oxcell *oxeval_atom(oxcell *c, oxcell *env) {
    if (isnil(c)) {
        return nill;
    } else if (isatom(c)) {
        return c;
    }
    return nill;
}

oxcell *oxeval(oxcell *expr, oxcell *env) {
    if (isatom(expr)) {
        return oxeval_atom(expr, env);
    }

    oxcell *symbol = oxcell_get_car(expr);
    oxcell *args   = oxcell_get_cdr(expr);

    if (issymbol(symbol)) {
        printf(".eval: %s, how cute\n", oxcell_get_text(oxcell_get_name(symbol)));
        // do we need to lookup the symbol?
        //
        if (!symbol->cons.cdr) {
            oxcell *lookup = oxsym_lookup(oxcell_get_name(symbol), env);
            if (!lookup || lookup == nill) {
                printf("error: undefined symbol '%s'\n", oxcell_get_text(oxcell_get_name(symbol)));
                exit(2);
            }
            symbol = lookup;
        }
    } else if (iscons(symbol)) {
        // symbol will be the result of evaluating the list
        //
        symbol = oxeval(symbol, env);
        if (!issymbol(symbol)) {
            // the expression didn't return something that we can execute
            //
        }
    }

    // symbol should either be a built-in or a lamba
    //
    oxcell *l = oxcell_get_cdr(symbol);
    if (!isatom(l)) {
        printf("error: can't execute a list\n");
        exit(2);
    }
    
    if (l->atom->kind == eAtomFunc) {
        printf(".info: executing builtin\n");
        return l->atom->func(args, env);
    }

    if (1) {
        printf("error: don't know how to execute symbol\n");
        exit(2);
    }

    return nill;
}

