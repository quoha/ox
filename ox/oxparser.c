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

//typedef struct oxtoken {
//    enum {oxTokEOF, oxTokOpenParen, oxTokCloseParen, oxTokInteger, oxTokReal, oxTokSymbol, oxTokText} kind;
//    union {
//        int    integer;
//        char  *name;
//        double number;
//        double real;
//        char  *text;
//    } value;
//    char token[1];
//} oxtoken;

typedef struct oxtoken {
    enum {oxTokEOF, oxTokOpenParen, oxTokCloseParen, oxTokReal, oxTokSymbol, oxTokText} kind;
    union {
        int    integer;
        char  *name;
        double real;
        char  *text;
    } value;
    char data[1];
} oxtoken;

oxtoken *ReadToken(oxbuf *ib, oxtoken *t2) {
    oxtoken T;
    oxtoken *t = &T;
    if (!t) {
        t = malloc(sizeof(*t));
        if (!t) {
            perror(__FUNCTION__);
            exit(2);
        }
    }
    
    // skip comments and whitespace
    do {
        while (isspace(ib->curr[0])) {
            ib->curr++;
        }
        if (ib->curr[0] == '/' && ib->curr[1] == '/') {
            while (ib->curr[0] && ib->curr[0] != '\n') {
                ib->curr++;
            }
        }
    } while (isspace(ib->curr[0]));
    
    if (!ib->curr[0]) {
        oxtoken *t = malloc(sizeof(*t));
        if (!t) {
            perror(__FUNCTION__);
            exit(2);
        }
        t->kind       = oxTokEOF;
        t->data[0]    = 0;
        t->value.text = 0;
        return t;
    }
    
    unsigned char *startLexeme = ib->curr++;
    
    if (*startLexeme == '(') {
        oxtoken *t = malloc(sizeof(*t) + 1);
        if (!t) {
            perror(__FUNCTION__);
            exit(2);
        }
        t->kind       = oxTokOpenParen;
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
        t->data[0]    = *startLexeme;
        t->data[1]    = 0;
        t->value.text = 0;
        return t;
    }
    
    char escape      = '\\';
    char singleQuote = '\'';
    char doubleQuote = '"';
    
    // look for quoted text
    //
    if (*startLexeme == singleQuote || *startLexeme == doubleQuote) {
        // advance to the closing quote. accept the escape character in the string.
        //
        while (ib->curr[0] && ib->curr[0] != *startLexeme) {
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
            t->value.text = t->data;
            
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
        
        if (!ib->curr[0] || ib->curr[0] == '(' || ib->curr[0] == ')' || isspace(ib->curr[0])) {
            t->kind         = oxTokReal;
            t->value.real = atol((char *)startLexeme);
            return t;
        }
        
        if (ib->curr[0] == '.' && isdigit(ib->curr[1])) {
            do {
                ++ib->curr;
            } while (isdigit(ib->curr[0]));
            
            if (ib->curr[0] || ib->curr[0] == ')' || isspace(ib->curr[0])) {
                t->kind         = oxTokReal;
                t->value.real = strtod((char *)startLexeme, 0);
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

    oxtoken *jt = malloc(sizeof(*jt) + (ib->curr - startLexeme));
    if (!t) {
        perror(__FUNCTION__);
        exit(2);
    }
    jt->kind       = oxTokSymbol;
    jt->value.name = jt->data;
    memcpy(jt->value.name, startLexeme, ib->curr - startLexeme);
    jt->value.name[ib->curr - startLexeme] = 0;
    
    return jt;
}

