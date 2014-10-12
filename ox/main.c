//
//  ox/main.c
//
//  Created by Michael Henderson on 10/1/14.
//  Copyright (c) 2014 Michael D Henderson. All rights reserved.
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <sys/stat.h>

typedef struct oxcell    oxcell;
typedef struct oxchmap   oxchmap;
typedef struct oxtext    oxtext;
typedef struct oxtoken   oxtoken;
typedef        oxcell *(*oxfunc)(oxcell *args, oxcell *env);

// the official list of common data types
//
typedef enum { oxcBoolean, oxcChMap, oxcFunc, oxcInteger, oxcList, oxcName, oxcReal, oxcSymbol, oxcText, oxcTimestamp, oxcToken } oxcellKind;
typedef enum { oxTokEOF, oxTokOpenParen, oxTokCloseParen, oxTokInteger, oxTokName, oxTokReal, oxTokText} oxtokenKind;


// why must everyone write their own string?
//
struct oxtext {
    unsigned char *curr;
    unsigned char *endOfData;
    unsigned long  hashValue;
    size_t         length;
    int            lineNumber;
    unsigned char  data[1];
};

// token for parsing. wastes a lot of memory at the moment.
//
struct oxtoken {
    oxtokenKind  kind;
    int          lineNumber;
    oxcell      *text;
    oxcell      *value;
};

// character map for whitelisting and such
//
struct oxchmap {
    unsigned char map[256];
};

// simple structure to hold common data types
//
struct oxcell {
    oxcellKind kind;
    long    integer;
    oxtext *text;
    oxcell *first;
    oxcell *rest;
    int     isEmpty;
    union {
        struct {
            oxcell          *first;
            oxcell          *rest;
            int              isEmpty;
        } list;
        union {
            int              boolean;
            oxchmap          chmap;
            unsigned long    dttm;
            oxfunc           func;
            long             integer;
            oxtext          *name;
            double           real;
            struct {
                oxtext      *name;
                oxcell      *value;
            } symbol;
            oxtext          *text;
            oxtoken         *token;
        } atom;
    } u;
};

#define OXISLIST(c)    ((c)->kind == oxcList)
#define OXISEMPTY(c)   (OXISLIST(c) && (c)->isEmpty)
#define OXISATOM(c)    (!(OXISLIST(c)))
#define OXISNIL(c)     ((c) == oxnil)
#define OXISTEXT(c)    ((c)->kind == oxcText)
#define OXISTOKEN(c)   ((c)->kind == oxcToken)

#define OXCEMPTY(c)    ((c)->isEmpty)
//#define OXCFIRST(c)    ((c)->u.list.first)
#define OXCFIRST(c)    ((c)->first)
//#define OXCREST(c)     ((c)->u.list.rest)
#define OXCREST(c)     ((c)->rest)
#define OXCBOOLEAN(c)  ((c)->u.atom.boolean)
#define OXCHMAP(c)     ((c)->u.atom.chmap)
#define OXCDTTM(c)     ((c)->u.atom.dttm)
#define OXCFUNC(c)     ((c)->u.atom.func)
#define OXCINTEGER(c)  ((c)->integer)
//#define OXCINTEGER(c)  ((c)->u.atom.integer)
#define OXCNAME(c)     ((c)->u.atom.name)
#define OXCREAL(c)     ((c)->u.atom.real)
//#define OXCTEXT(c)     ((c)->u.atom.text)
#define OXCTEXT(c)     ((c)->text)
#define OXCTOKEN(c)    ((c)->u.atom.token)
#define OXCSYMBOL(c)   ((c)->u.atom.symbol)

oxcell *oxenv;
oxcell *oxnil;
oxcell *oxtrue;

oxcell *oxinit(void);

oxcell *oxeval(oxcell *expr, oxcell *env);

oxcell *oxcell_factory(oxcellKind kind, ...);
oxcell *oxcell_factory_cstring(const char *cstring);

oxcell *oxcell_get_env(void);
oxcell *oxcell_get_nil(void);
oxcell *oxcell_get_true(void);

oxtext *oxtext_from_cstring(const char *cstring);
oxtext *oxtext_from_file(const char *fileName);
oxtext *oxtext_from_memory(unsigned char *memory, size_t length);
oxtext *oxtext_from_text(oxtext *text);
oxtext *oxtext_hash_text(oxtext *text);
oxtext *oxtext_unescape(oxtext *text);

// built in functions
//
oxcell *oxf_expr_print(oxcell *args, oxcell *env);
oxcell *oxf_expr_read(oxcell *args, oxcell *env);
oxcell *oxf_fread(oxcell *args, oxcell *env);
oxcell *oxf_token_read(oxcell *args, oxcell *env);

unsigned char *strrcpy(unsigned char *src, size_t length);

oxcell *oxinit(void) {
    oxcell_get_nil();
    oxcell_get_env();
    oxcell_get_true();

    return oxenv;
}

oxcell *oxcell_factory(oxcellKind kind, ...) {
    oxcell  *c = malloc(sizeof(*c));
    if (!c) {
        perror(__FUNCTION__);
        exit(2);
    }
    c->kind = kind;
    
    va_list ap;
    va_start(ap, kind);
    
    switch (kind) {
        case oxcBoolean:
            c->u.atom.boolean = va_arg(ap, int);
            if (c->u.atom.boolean) {
                c->u.atom.boolean = -1;
            }
            break;
        case oxcChMap:
            memcpy(&(c->u.atom.chmap), va_arg(ap, oxchmap *), sizeof(c->u.atom.chmap));
            break;
        case oxcFunc:
            c->u.atom.func = va_arg(ap, oxfunc);
            break;
        case oxcInteger:
            OXCINTEGER(c) = va_arg(ap, long);
            break;
        case oxcList:
            c->isEmpty  = 0;
            OXCFIRST(c) = va_arg(ap, oxcell *);
            OXCREST(c)  = va_arg(ap, oxcell *);
            break;
        case oxcName:
            c->u.atom.name = va_arg(ap, oxtext *);
            break;
        case oxcReal:
            OXCREAL(c) = va_arg(ap, double);
            break;
        case oxcSymbol:
            c->u.atom.symbol.name  = va_arg(ap, oxtext *);
            c->u.atom.symbol.value = va_arg(ap, oxcell *);
            break;
        case oxcText:
            OXCTEXT(c) = va_arg(ap, oxtext *);
            break;
        case oxcTimestamp:
            c->u.atom.dttm = va_arg(ap, unsigned long);
            break;
        case oxcToken:
            OXCTOKEN(c) = va_arg(ap, oxtoken *);
            break;
    }
    
    va_end(ap);
    
    return c;
}

oxcell *oxcell_factory_cstring(const char *cstring) {
    return oxcell_factory(oxcText, oxtext_from_cstring(cstring));
}

oxcell *oxcell_get_env(void) {
    if (!oxenv) {
        oxenv = oxcell_get_nil();
    }
    return oxenv;
}

oxcell *oxcell_get_nil(void) {
    if (!oxnil) {
        oxnil = oxcell_factory(oxcList, NULL, NULL);
        OXCFIRST(oxnil) = oxnil;
        OXCREST(oxnil)  = oxnil;
    }
    return oxnil;
}

oxcell *oxcell_get_true(void) {
    if (!oxtrue) {
        oxtrue = oxcell_factory(oxcBoolean, -1);
    }
    return oxtrue;
}

//   usage: (expr-print #expression#)
// returns: nil
//
oxcell *oxf_expr_print(oxcell *args, oxcell *env) {
    oxcell *text;
    oxcell *value;
    
    if (OXISNIL(args)) {
        printf("() ");
        return oxnil;
    }
    switch (args->kind) {
        case oxcBoolean:
            printf("%s ", OXCBOOLEAN(args) ? "true" : "false");
            break;
        case oxcChMap:
            printf("#chmap# ");
            break;
        case oxcFunc:
            printf("#func# ");
            break;
        case oxcInteger:
            printf("%ld ", OXCINTEGER(args));
            break;
        case oxcList:
            if (OXISEMPTY(OXCFIRST(args))) {
                printf("() ");
            } else {
                printf("( ");
                while (args != oxnil) {
                    oxf_expr_print(OXCFIRST(args), env);
                    args = OXCREST(args);
                }
                printf(") ");
            }
            break;
        case oxcName:
            printf("%s ", OXCNAME(args)->data);
            break;
        case oxcReal:
            printf("%g ", OXCREAL(args));
            break;
        case oxcSymbol:
            printf("#symbol#");
            break;
        case oxcText:
            printf("\"%s\" ", OXCTEXT(args)->data);
            break;
        case oxcTimestamp:
            printf("#dttm# ");
            break;
        case oxcToken:
            text = OXCTOKEN(args)->text;
            value = OXCTOKEN(args)->value;
            switch (OXCTOKEN(args)->kind) {
                case oxTokCloseParen:
                    printf(") ");
                    break;
                case oxTokEOF:
                    printf("**eof** ");
                    break;
                case oxTokInteger:
                    printf("int  = %s ", OXCTEXT(text)->data);
                    break;
                case oxTokName:
                    printf("name = %s ", OXCTEXT(text)->data);
                    break;
                case oxTokOpenParen:
                    printf("( ");
                    break;
                case oxTokReal:
                    printf("real = %s ", OXCTEXT(text)->data);
                    break;
                case oxTokText:
                    printf("text = \"%s\" ", OXCTEXT(text)->data);
                    break;
            }
            break;
    }
    return oxnil;
}

//   usage: (append arg1 arg2)
// returns: (arg1 arg2)
//    note: this is not the same as (arg1 (arg2))
//
oxcell *oxf_append(oxcell *args, oxcell *env) {
    oxcell *result = oxnil;
    oxcell *arg1 = OXCFIRST(args);
    oxcell *arg2 = OXCREST(args);
    if (OXISNIL(arg2)) {
        result = arg1;
    } else {
        oxcell *tail;
        result = tail = oxcell_factory(oxcList, arg1, oxnil);
        arg1 = OXCREST(arg1);
        
        while (!OXISNIL(args)) {
            tail = OXCREST(tail) = oxcell_factory(oxcList, OXCFIRST(arg1), oxnil);
            arg1 = OXCREST(arg1);
        }

        OXCREST(tail) = oxcell_factory(oxcList, arg2, oxnil);
    }

    return result;
}

//   usage: (list args)
// returns: args
//
oxcell *oxf_list(oxcell *args, oxcell *env) {
    oxcell *result = oxnil;

    // easy to convert an atom into a list
    //
    if (!OXISLIST(args)) {
        result = oxcell_factory(oxcList, args, oxnil);
    } else if (OXISLIST(args)) {
        // otherwise arts is already a list, so just return it. seems
        // rational since the eval function will pass us a list of
        // arguments, right?
        //
        result = args;
    } else { /* NOT REACHED */
        // return a copy of the list
        //
        oxcell *tail;
        result = tail = oxcell_factory(oxcList, OXCFIRST(args), oxnil);
        args = OXCREST(args);

        while (!OXISNIL(args)) {
            tail = OXCREST(tail) = oxcell_factory(oxcList, OXCFIRST(args), oxnil);
            args = OXCREST(args);
        }
    }

    return result;
}

//   usage: (expr-read "text")
// returns: (#expression# errmsg)
//
oxcell *oxf_expr_read(oxcell *args, oxcell *env) {
    // we will return our result. the data is in the car of the list
    // and any error message is in the cdr.
    //
    oxcell *result = oxcell_factory(oxcList, oxnil, oxnil);
    
    // confirm that our argument is text
    //
    if (!OXISTEXT(args)) {
        OXCFIRST(result) = oxnil;
        OXCREST(result)  = oxcell_factory(oxcList, oxcell_factory_cstring("*input-is-not-text*"), oxnil);
        return result;
    }

    // read the next token from the input
    //
    oxcell *read = oxf_token_read(args, env);
    if (!OXISNIL(OXCREST(read))) {
        OXCFIRST(result) = oxnil;
        OXCREST(result)  = oxcell_factory_cstring("*expr-read-failed*");
        return result;
    }

    oxcell *token = OXCFIRST(read);
    switch (OXCTOKEN(token)->kind) {
        case oxTokCloseParen:
            OXCFIRST(result) = oxnil;
            OXCREST(result)  = oxcell_factory_cstring("*error-in-input*");
            return result;
        case oxTokEOF:
            OXCFIRST(result) = oxnil;
            OXCREST(result)  = oxcell_factory_cstring("*end-of-input*");
            return result;
        case oxTokInteger:
            OXCFIRST(result) = oxcell_factory(oxcInteger, OXCINTEGER(OXCTOKEN(token)->value));
            OXCREST(result)  = oxnil;
            return result;
        case oxTokOpenParen:
            break;
        case oxTokReal:
            OXCFIRST(result) = oxcell_factory(oxcReal, OXCREAL(OXCTOKEN(token)->value));
            OXCREST(result)  = oxnil;
            return result;
        case oxTokName:
            OXCFIRST(result) = oxcell_factory(oxcName, OXCNAME(OXCTOKEN(token)->value));
            OXCREST(result)  = oxnil;
            return result;
        case oxTokText:
            OXCFIRST(result) = oxcell_factory(oxcText, OXCTEXT(OXCTOKEN(token)->value));
            OXCREST(result)  = oxnil;
            return result;
    }


    oxcell *tail = oxnil;
    oxcell *expr = oxnil;

    for (read = oxf_expr_read(args, env); OXISNIL(OXCREST(read)); read = oxf_expr_read(args, env)) {
        oxcell *token = OXCFIRST(read);

        switch (OXCTOKEN(token)->kind) {
            case oxTokCloseParen:
                if (OXISNIL(OXCFIRST(result))) {
                    OXCFIRST(result) = oxcell_factory(oxcList, oxnil, oxnil);
                    OXCEMPTY(OXCFIRST(result)) = -1;
                }
                return result;
            case oxTokEOF:
                OXCFIRST(result) = oxnil;
                OXCREST(result)  = oxcell_factory_cstring("*unexpected-end-of-input*");
                return result;
            case oxTokInteger:
                expr = oxcell_factory(oxcInteger, OXCINTEGER(OXCTOKEN(token)->value));
                break;
            case oxTokOpenParen:
                expr = oxf_expr_read(args, env);
                if (!OXISNIL(OXCREST(expr))) {
                    return expr;
                }
                expr = OXCFIRST(expr);
                break;
            case oxTokReal:
                expr = oxcell_factory(oxcReal, OXCREAL(OXCTOKEN(token)->value));
                break;
            case oxTokName:
                expr = oxcell_factory(oxcName, OXCNAME(OXCTOKEN(token)->value));
                break;
            case oxTokText:
                expr = oxcell_factory(oxcText, OXCTEXT(OXCTOKEN(token)->value));
                break;
        }

        if (OXISNIL(tail)) {
            OXCFIRST(result) = expr;
        } else {
            OXCREST(tail) = oxcell_factory(oxcList, expr, oxnil);
        }
        tail = expr;
    }

    // if we get here, there was an error
    //
    return read;
}

//   usage: (fread "filename")
// returns: ("data" errmsg)
//
oxcell *oxf_fread(oxcell *args, oxcell *env) {
    // we will return our result. the data is in the car of the list
    // and any error message is in the cdr.
    //
    oxcell *result = oxcell_factory(oxcList, oxnil, oxnil);

    // confirm that our argument is text
    //
    if (!OXISTEXT(args)) {
        OXCFIRST(result) = oxnil;
        OXCREST(result)  = oxcell_factory(oxcList, oxcell_factory_cstring("*arg-must-be-text*"), oxnil);
        return result;
    }

    oxtext *text = oxtext_from_file((const char *)(OXCTEXT(args)->data));
    if (!text) {
        OXCFIRST(result) = oxnil;
        OXCREST(result)  = oxcell_factory(oxcList, oxcell_factory_cstring("*invalid-file-name*"), oxnil);
        return result;
    }

    OXCFIRST(result) = oxcell_factory(oxcText, text);

    return result;
}

//   usage: (token-read "text")
// returns: (data errmsg)
// side-ef: updates internal state of the "stream" of text
//
oxcell *oxf_token_read(oxcell *args, oxcell *env) {
    // we will return our result. the data is in the car of the list
    // and any error message is in the cdr.
    //
    oxcell *result = oxcell_factory(oxcList, oxnil, oxnil);
    
    // confirm that our argument is text
    //
    if (!OXISTEXT(args)) {
        OXCFIRST(result) = oxnil;
        OXCREST(result)  = oxcell_factory(oxcList, oxcell_factory_cstring("*arg-must-be-text*"), oxnil);
        return result;
    }

    oxtext *text = OXCTEXT(args);
    
    // skip comments and whitespace
    do {
        while (text->curr < text->endOfData && isspace(text->curr[0])) {
            if (text->curr[0] == '\n') {
                text->lineNumber++;
            }
            text->curr++;
        }
        if (text->curr[0] == '#' || text->curr[1] == ';') {
            while (text->curr < text->endOfData && text->curr[0] != '\n') {
                text->curr++;
            }
        }
    } while (text->curr < text->endOfData && isspace(text->curr[0]));
    
    oxtoken *token = malloc(sizeof(*token));
    if (!token) {
        perror(__FUNCTION__);
        exit(2);
    }
    token->lineNumber    = text->lineNumber;
    token->text          = oxnil;
    token->value         = oxnil;
    OXCFIRST(result)     = oxcell_factory(oxcToken, token);
    
    // check for end-of-input
    //
    if (!(text->curr < text->endOfData)) {
        token->kind          = oxTokEOF;
        return result;
    }
    
    unsigned char *startLexeme = text->curr++;
    
    if (*startLexeme == '(') {
        token->kind          = oxTokOpenParen;
        token->text          = oxcell_factory_cstring("(");
        token->value         = oxcell_factory_cstring("(");
        OXCTEXT(token->text)->lineNumber = token->lineNumber;
        OXCTEXT(token->value)->lineNumber = token->lineNumber;
        return result;
    }
    
    if (*startLexeme == ')') {
        token->kind          = oxTokCloseParen;
        token->text          = oxcell_factory_cstring(")");
        token->value         = oxcell_factory_cstring(")");
        OXCTEXT(token->text)->lineNumber = token->lineNumber;
        OXCTEXT(token->value)->lineNumber = token->lineNumber;
        return result;
    }
    
    if (*startLexeme == '\'') {
        token->kind          = oxTokName;
        token->text          = oxcell_factory_cstring("'");
        token->value         = oxcell_factory_cstring("'");
        OXCTEXT(token->text)->lineNumber = token->lineNumber;
        OXCTEXT(token->value)->lineNumber = token->lineNumber;
        
        return result;
    }
    
    // look for quoted text
    //
    const char escape      = '\\';
    const char doubleQuote = '"';
    
    if (*startLexeme == doubleQuote) {
        token->kind = oxTokText;
        
        // advance to the closing quote. accept the escape character in the string.
        //
        while (text->curr < text->endOfData && text->curr[0] != *startLexeme) {
            if (text->curr[0] == '\n') {
                text->lineNumber++;
            } else if (text->curr[0] == escape) {
                if (text->curr[1]) {
                    ++text->curr;
                } else {
                    // escape followed by end-of-buffer is bad
                    //
                    OXCFIRST(result) = oxnil;
                    OXCREST(result)  = oxcell_factory(oxcList, oxcell_factory_cstring("incomplete escape sequence"), oxnil);
                    return result;
                }
            }
            ++text->curr;
        }
        
        // only accept strings which have open and close quotes
        //
        if (!(text->curr < text->endOfData && text->curr[0] == *startLexeme)) {
            OXCFIRST(result) = oxnil;
            OXCREST(result)  = oxcell_factory(oxcList, oxcell_factory_cstring("quoted text not terminated"), oxnil);
            return result;
        }
        
        // skip past the closing quote
        //
        text->curr++;
        
        // only accept strings which end at end-of-buffer or a space
        //
        if (!(text->curr < text->endOfData || isspace(text->curr[0]))) {
            OXCFIRST(result) = oxnil;
            OXCREST(result)  = oxcell_factory(oxcList, oxcell_factory_cstring("quoted text not followed by space"), oxnil);
            return result;
        }
        
        // make an exact copy of the input without including the quote marks
        //
        token->text = oxcell_factory(oxcText, oxtext_from_memory(startLexeme + 1, text->curr - startLexeme - 2));
        OXCTEXT(token->text)->lineNumber = token->lineNumber;
        
        // make a copy without quotes and resolve all escaped characters in the input
        //
        token->value = oxcell_factory(oxcText, oxtext_from_memory(startLexeme + 1, text->curr - startLexeme - 2));
        OXCTEXT(token->value)->lineNumber = token->lineNumber;
        oxtext_unescape(OXCTEXT(token->value));

        return result;
    }
    
    // look for raw quoted text, \(.*)"..."(*.)/, where the .* represents
    // optional balanced open and close alphanumeric text:
    //      \" "/
    //     \\" "//
    //    \ac" "ca/
    //
    if (*startLexeme == escape) {
        // look for \(.*)" here
        //
        unsigned char *p = startLexeme + 1;
        while (*p && (*p == escape || isalnum(*p))) {
            p++;
        }
        
        if (*p == doubleQuote) {
            token->kind = oxTokText;
            
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
            } else {
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
            }
            size_t lenCloseString = strlen((char *)closeString);
            
            // ignore the first raw character if a new-line
            //
            if (*startRawText == '\n') {
                startRawText++;
            }
            
            // find that close string in the input
            //
            text->curr = startRawText;
            while (text->curr < text->endOfData) {
                if (text->curr[0] == '\n') {
                    text->lineNumber++;
                }
                if (text->curr[0] == closeString[0] && text->curr[1] == closeString[1]) {
                    if (!memcmp(text->curr, closeString, lenCloseString)) {
                        break;
                    }
                }
                text->curr++;
            }
            
            unsigned char *endRawText = text->curr;
            if (text->curr[0] == closeString[0]) {
                // found close string in the buffer
                //
                text->curr += lenCloseString;
                if (text->curr >= text->endOfData) {
                    text->curr = text->endOfData;
                }
            } else {
                // did not find close string in the buffer, so go to the end of the buffer
                //
                endRawText = text->curr = text->endOfData;
            }
            
            // make an exact copy of the input without including the open
            // and close strings.
            //
            token->text = oxcell_factory(oxcText, oxtext_from_memory(startRawText, endRawText - startRawText));
            OXCTEXT(token->text)->lineNumber = token->lineNumber;
            token->value = oxcell_factory(oxcText, oxtext_from_memory(startRawText, endRawText - startRawText));
            OXCTEXT(token->value)->lineNumber = token->lineNumber;
            
            return result;
        }
    }
    
    // look for an integer ([+-]d+) or number ([+-]d+.d+).
    //
    // note that if the text isn't a valid integer or number, we will
    // fall through to the symbol code.
    //
    //    1    is an integer, so is +1   or -1
    //    1.2  is a  number , so is +1.2 or -1.2
    //    1.   is a  name   , so is   .2 or  1.2k
    //
    if (isdigit(*startLexeme) || ((*startLexeme == '-' || *startLexeme == '+') && isdigit(text->curr[0]))) {
        while (text->curr < text->endOfData && isdigit(text->curr[0])) {
            ++text->curr;
        }
        
        if (!(text->curr < text->endOfData) || isspace(text->curr[0]) || text->curr[0] == '(' || text->curr[0] == ')') {
            token->kind = oxTokInteger;
            
            // make an exact copy of the input
            //
            token->text = oxcell_factory(oxcText, oxtext_from_memory(startLexeme, text->curr - startLexeme));
            OXCTEXT(token->text)->lineNumber = token->lineNumber;
            
            // the value is the converted text
            //
            token->value = oxcell_factory(oxcInteger, atol((const char *)(OXCTEXT(token->text)->data)));
            OXCTEXT(token->text)->lineNumber = token->lineNumber;
            
            return result;
        }
        
        if (text->curr[0] == '.' && isdigit(text->curr[1])) {
            do {
                ++text->curr;
            } while (isdigit(text->curr[0]));
            
            if (!(text->curr < text->endOfData) || isspace(text->curr[0]) || text->curr[0] == '(' || text->curr[0] == ')') {
                token->kind = oxTokReal;
                
                // make an exact copy of the input
                //
                token->text = oxcell_factory(oxcText, oxtext_from_memory(startLexeme, text->curr - startLexeme));
                OXCTEXT(token->text)->lineNumber = token->lineNumber;
                
                // the value is the converted text
                //
                token->value = oxcell_factory(oxcInteger, strtod((const char *)(OXCTEXT(token->text)->data), 0));
                OXCTEXT(token->text)->lineNumber = token->lineNumber;
                
                return result;
            }
        }
    }
    
    // look for a name. a name is any string of characters that doesn't
    // contain a blank or parentheses.
    //
    while (text->curr < text->endOfData && !(isspace(text->curr[0]) || text->curr[0] == '(' || text->curr[0] == ')')) {
        ++text->curr;
    }
    
    token->kind = oxTokName;
    
    // make an exact copy of the input
    //
    token->text = oxcell_factory(oxcText, oxtext_from_memory(startLexeme, text->curr - startLexeme));
    OXCTEXT(token->text)->lineNumber = token->lineNumber;
    
    // the value is the same
    //
    token->value = oxcell_factory(oxcText, oxtext_from_memory(startLexeme, text->curr - startLexeme));
    OXCTEXT(token->value)->lineNumber = token->lineNumber;
    
    return result;
}

oxtext *oxtext_from_cstring(const char *cstring) {
    cstring = cstring ? cstring : "";
    return oxtext_from_memory((unsigned char *)cstring, strlen(cstring));
}

// this function reads the entire file into a buffer
//
oxtext *oxtext_from_file(const char *fileName) {
    if (!fileName) {
        fprintf(stderr, "error: %s passed null filename\n", __FUNCTION__);
        exit(2);
    }

    // stat the file
    struct stat sb;
    if (stat(fileName, &sb) != 0) {
        perror(fileName);
        exit(2);
    }
    size_t length = sb.st_size;
    
    // create data for the file size
    //
    oxtext *text = malloc(sizeof(*text) + length);
    if (!text) {
        perror(__FUNCTION__);
        exit(2);
    }
    
    // read in that much data
    FILE *fp = fopen(fileName, "r");
    if (!(fp && fread(text->data, 1, length, fp) == length)) {
        perror(fileName);
        exit(2);
    }
    fclose(fp);
    
    
    text->curr       = text->data;
    text->endOfData  = text->data + length;
    text->length     = length;
    text->lineNumber = 0;
    
    text->data[length] = 0;
    
    oxtext_hash_text(text);

    return text;
}

oxtext *oxtext_from_memory(unsigned char *memory, size_t length) {
    oxtext *text = malloc(sizeof(*text) + length);
    if (!text) {
        perror(__FUNCTION__);
        exit(2);
    }
    
    text->curr       = text->data;
    text->endOfData  = text->data + length;
    text->length     = length;
    text->lineNumber = 0;
    
    memcpy(text->data, memory, length);
    text->data[length] = 0;
    
    oxtext_hash_text(text);
    
    return text;
}

oxtext *oxtext_from_text(oxtext *textToCopy) {
    oxtext *text = malloc(sizeof(*text) + textToCopy->length);
    if (!text) {
        perror(__FUNCTION__);
        exit(2);
    }
    memcpy(text, textToCopy, sizeof(*text) + textToCopy->length);

    text->curr       = text->data;
    text->endOfData  = text->data + text->length;
    
    return text;
}

// revisit if jenkin's hash turns out to have issues. never has
// in the past, ya know.
//
oxtext *oxtext_hash_text(oxtext *text) {
    size_t        length  = text->length;
    unsigned char *memory = text->data;

    text->hashValue = 0;
    while (length--) {
        text->hashValue += *(memory++);
        text->hashValue += (text->hashValue << 10);
        text->hashValue += (text->hashValue >>  6);
    }
    text->hashValue += (text->hashValue <<  3);
    text->hashValue ^= (text->hashValue >> 11);
    text->hashValue += (text->hashValue << 15);
    return text;
}

// in general, replace slash-foo with foo, then
// adjust the end of data pointer and hash value.
//
// note: resets curr pointer to start of the text.
//
oxtext *oxtext_unescape(oxtext *text) {
    unsigned char *src = text->data;
    while (src < text->endOfData && *src != '\\') {
        src++;
    }

    if (src < text->endOfData) {
        unsigned char *dst = src;
        do {
            if (*src == '\\') {
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
        } while (src < text->endOfData);

        // zero out the remainder of the buffer as a
        // convenience.
        //
        while (dst < text->endOfData) {
            *dst = 0;
        }

        // reset the pointers into the buffer
        //
        text->curr      = text->data;
        text->endOfData = dst;
        text->length    = text->endOfData - text->data;
        
        // recalculate the hash
        //
        oxtext_hash_text(text);
    }

    return text;
}

// create a copy of a string with the contents reversed.
//
// for example:
//   ""    returns ""
//   "o"   returns "o"
//   "qp"  returns "pq"
//   "abc" returns "cba"
//
unsigned char *strrcpy(unsigned char *src, size_t length) {
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








int main(int argc, const char * argv[]) {
    int debugLevel   = 0;
    int doCheck      = 1;
    int doDumpInput  = 0;
    int doDumpTokens = 1;
    int doEval       = 0;
    int doOptions    = 1;
    int idx;

    oxcell *oxenv = oxinit();
    oxcell *oxnil = oxcell_get_nil();

    for (idx = 1; idx < argc; idx++) {
        printf("..opt: %3d %s\n", idx, argv[idx]);
        if (!doOptions || !(argv[idx][0] == '-' && argv[idx][1] == '-')) {
            // treat as a file name
            //
            oxcell *result = oxf_fread(oxcell_factory_cstring(argv[idx]), oxenv);
            oxcell *text = OXCFIRST(result);
            oxcell *err = OXCREST(result);
            if (!OXISNIL(err)) {
                printf("...ox: error reading file\n");
                printf(".....: %-18s == '%s'\n", "fileName", argv[idx]);
                printf(".....: %-18s == '%s'\n", "errorMsg", OXCTEXT(OXCFIRST(err))->data);
                return 2;
            }
            if (doDumpInput) {
                printf("\n----------------------------------\n");
                oxf_expr_print(text, oxenv);
                //oxf_prexpr(oxcell_factory(oxcName, oxtext_from_cstring("\n\n")), oxenv);
                printf("\n----------------------------------\n");
            }

            if (doDumpTokens) {
                printf("\n----------------------------------\n");
                // debug step - dump out all of the tokens in the file
                //
                result = oxf_token_read(text, oxenv);
                oxcell *token = OXCFIRST(result);
                err = OXCREST(result);
                while (OXISNIL(err) && !(OXISTOKEN(token) && OXCTOKEN(token)->kind == oxTokEOF)) {
                    oxf_expr_print(token, oxenv);
                    printf("\n");
                    result = oxf_token_read(text, oxenv);
                    token = OXCFIRST(result);
                    err = OXCREST(result);
                }
                if (!OXISNIL(err)) {
                    // should be end-of-input
                    printf("...ox: error reading token from file\n");
                    printf(".....: %-18s == '%s'\n", "fileName", argv[idx]);
                    printf(".....: %-18s == '%s'\n", "errorMsg", OXCTEXT(OXCFIRST(err))->data);
                    return 2;
                }

                // reset the stream
                //
                OXCTEXT(text)->curr       = OXCTEXT(text)->data;
                OXCTEXT(text)->lineNumber = 1;

                printf("\n----------------------------------\n");
            }

            if (doEval) {
                // eval the script
                result = oxf_expr_read(text, oxenv);
                oxcell *expr = OXCFIRST(result);
                err = OXCREST(result);
                while (OXISNIL(err)) {
                    printf(": ");
                    oxf_expr_print(expr, oxenv);
                    printf("\n");
                    result = oxf_expr_read(text, oxenv);
                    expr = OXCFIRST(result);
                    err = OXCREST(result);
                }

#if 0
                for (expr = oxf_expr_read(text, oxenv); expr; expr = oxf_expr_read(text, oxenv)) {
                    oxcell *result = oxeval(expr, oxenv);
                    printf("= ");
                    oxcell_print(result);
                    printf("\n");
                }
#endif
                if (!OXISNIL(err)) {
                    // should be end-of-input
                    printf("...ox: error parsing expression\n");
                    printf(".....: %-18s == '%s'\n", "fileName", argv[idx]);
                    printf(".....: %-18s == '%s'\n", "errorMsg", OXCTEXT(OXCFIRST(err))->data);
                    return 2;
                }
                
                // reset the stream
                //
                OXCTEXT(text)->curr       = OXCTEXT(text)->data;
                OXCTEXT(text)->lineNumber = 1;
                
                printf("\n----------------------------------\n");
            }
        } else if (argv[idx][0] == '-' && argv[idx][1] == '-' && argv[idx][3] == 0) {
            doOptions = 0;
        } else if (argv[idx][0] == '-' && argv[idx][1] == '-') {
            char *opt = strdup(argv[idx]);
            if (!opt) {
                perror(__FUNCTION__);
                return 2;
            }
            char *val = opt;
            while (*val && *val != '=') {
                val++;
            }
            if (*val) {
                *(val++) = 0;
            } else {
                val = 0;
            }
            
            if (!strcmp(opt, "--help")) {
                printf("usage: ox [options] [-- script [script ...]]\n");
                return 0;
            } else if (!strcmp(opt, "--check-file") && !val) {
                doCheck = -1;
            } else if (!strcmp(opt, "--check-file") && val && (!strcmp(val, "false") || !strcmp(val, "no"))) {
                doCheck = 0;
            } else if (!strcmp(opt, "--check-file") && val && (!strcmp(val, "true") || !strcmp(val, "yes"))) {
                doCheck = -1;
            } else if (!strcmp(opt, "--eval-file")) {
                doEval  = -1;
            } else if (!strcmp(opt, "--eval-file") && val && (!strcmp(val, "false") || !strcmp(val, "no"))) {
                doEval = 0;
            } else if (!strcmp(opt, "--eval-file") && val && (!strcmp(val, "true") || !strcmp(val, "yes"))) {
                doEval = -1;
            } else if (!strcmp(opt, "--debug") && !val) {
                debugLevel++;
            } else if (!strcmp(opt, "--debug") && val && (!strcmp(val, "false") || !strcmp(val, "no"))) {
                debugLevel = 0;
            } else if (!strcmp(opt, "--debug") && val && (!strcmp(val, "true") || !strcmp(val, "yes"))) {
                debugLevel = 13;
            } else if (!strcmp(opt, "--dump") && val && !strcmp(val, "input")) {
                doDumpInput = -1;
            } else if (!strcmp(opt, "--dump") && val && !strcmp(val, "tokens")) {
                doDumpTokens = - 1;
            } else if (!strcmp(opt, "--no-dump") && val && !strcmp(val, "input")) {
                doDumpInput = 0;
            } else if (!strcmp(opt, "--no-dump") && val && !strcmp(val, "tokens")) {
                doDumpTokens = 0;
            } else {
                fprintf(stderr, "error:\tinvalid option '%s'\n", argv[idx]);
                return 2;
            }
            free(opt);
        } else {
            fprintf(stderr, "error:\tinvalid option '%s'\n", argv[idx]);
            fprintf(stderr, "\ttry --help if you're stuck\n");
            return 2;
        }
    }

    printf("...ox: thanks for stopping by\n");
    
    return 0;
}
