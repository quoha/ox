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
typedef enum { oxTokEOF, oxTokOpenParen, oxTokCloseParen, oxTokSingleQuote, oxTokInteger, oxTokName, oxTokReal, oxTokText} oxtokenKind;


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
    union {
        struct {
            oxcell          *first;
            oxcell          *rest;
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
#define OXISATOM(c)    (!(OXISLIST(c)))
#define OXISNIL(c)     ((c) == oxnil)
#define OXISTOKEN(c)   ((c)->kind == oxcToken)

#define OXCFIRST(c)    ((c)->u.list.first)
#define OXCREST(c)     ((c)->u.list.rest)
#define OXCBOOLEAN(c)  ((c)->u.atom.boolean)
#define OXCHMAP(c)     ((c)->u.atom.chmap)
#define OXCDTTM(c)     ((c)->u.atom.dttm)
#define OXCFUNC(c)     ((c)->u.atom.func)
#define OXCINTEGER(c)  ((c)->u.atom.integer)
#define OXCNAME(c)     ((c)->u.atom.name)
#define OXCREAL(c)     ((c)->u.atom.real)
#define OXCTEXT(c)     ((c)->u.atom.text)
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

oxcell     *oxexpr_read(oxcell *ib);
oxcell     *oxexpr_read_tail(oxcell *ib);
oxcell     *oxtok_read(oxcell *args, oxcell *env);
const char *oxtok_toktype(oxtoken *t);

// built in functions
//
oxcell *oxf_fread(oxcell *arg, oxcell *env);
oxcell *oxf_prexpr(oxcell *arg, oxcell *env);
oxcell *oxf_token_read(oxcell *arg, oxcell *env);

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
            c->u.atom.integer = va_arg(ap, long);
            break;
        case oxcList:
            c->u.list.first = va_arg(ap, oxcell *);
            c->u.list.rest  = va_arg(ap, oxcell *);
            break;
        case oxcName:
            c->u.atom.name = va_arg(ap, oxtext *);
            break;
        case oxcReal:
            c->u.atom.real = va_arg(ap, double);
            break;
        case oxcSymbol:
            c->u.atom.symbol.name  = va_arg(ap, oxtext *);
            c->u.atom.symbol.value = va_arg(ap, oxcell *);
            break;
        case oxcText:
            c->u.atom.text = va_arg(ap, oxtext *);
            break;
        case oxcTimestamp:
            c->u.atom.dttm = va_arg(ap, unsigned long);
            break;
        case oxcToken:
            c->u.atom.token = va_arg(ap, oxtoken *);
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
        oxnil->u.list.first = oxnil;
        oxnil->u.list.rest  = oxnil;
    }
    return oxnil;
}

oxcell *oxcell_get_true(void) {
    if (!oxtrue) {
        oxtrue = oxcell_factory(oxcBoolean, -1);
    }
    return oxtrue;
}

#if 0
// dangerous? accept cells but only read text.
//
// returns the traditional (data err) list. if err is non-nil then we
// had a problem (most likely end-of-input).
//
oxcell *oxexpr_read(oxcell *ib) {
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
    
    // NOT REACHED
    return 0;
}
#endif

// returns the traditional (data err) list. if err is non-nil then we
// had a problem (most likely end-of-input).
//
//   usage: (fread "filename")
// returns: (data errmsg)
//
oxcell *oxf_fread(oxcell *arg, oxcell *env) {
    // we will return our result. the data is in the car of the list
    // and any error message is in the cdr.
    //
    oxcell *result = oxcell_factory(oxcList, oxnil, oxnil);

    // confirm that our argument is text
    //
    if (arg->kind != oxcText) {
        result->u.list.rest = oxcell_factory(oxcList, oxcell_factory_cstring("arg must be text"), oxnil);
        return result;
    }

    oxtext *text = oxtext_from_file((const char *)(arg->u.atom.text->data));
    if (!text) {
        result->u.list.rest = oxcell_factory(oxcList, oxcell_factory_cstring("invalid file name"), oxnil);
        return result;
    }

    result->u.list.first = oxcell_factory(oxcText, text);

    return result;
}

oxcell *oxf_prexpr(oxcell *arg, oxcell *env) {
    oxcell *text;
    oxcell *value;

    if (arg == oxnil) {
        printf("() ");
        return oxnil;
    }
    switch (arg->kind) {
        case oxcBoolean:
            printf("%s ", OXCBOOLEAN(arg) ? "true" : "false");
            break;
        case oxcChMap:
            printf("#chmap# ");
            break;
        case oxcFunc:
            printf("#func# ");
            break;
        case oxcInteger:
            printf("%ld ", OXCINTEGER(arg));
            break;
        case oxcList:
            printf("( ");
            while (arg != oxnil) {
                oxf_prexpr(OXCFIRST(arg), env);
                arg = OXCREST(arg);
            }
            printf(") ");
            break;
        case oxcName:
            printf("%s ", OXCNAME(arg)->data);
            break;
        case oxcReal:
            printf("%g ", OXCREAL(arg));
            break;
        case oxcSymbol:
            printf("#symbol#");
            break;
        case oxcText:
            printf("\"%s\" ", OXCTEXT(arg)->data);
            break;
        case oxcTimestamp:
            printf("#dttm# ");
            break;
        case oxcToken:
            text = OXCTOKEN(arg)->text;
            value = OXCTOKEN(arg)->value;
            switch (OXCTOKEN(arg)->kind) {
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
                case oxTokSingleQuote:
                    printf("' ");
                    break;
                case oxTokText:
                    printf("text = \"%s\" ", OXCTEXT(text)->data);
                    break;
            }
            break;
    }
    return oxnil;
}

//   usage: (token-read text)
// returns: (data errmsg)
// side-ef: updates internal state of the "stream" of text
//
oxcell *oxf_token_read(oxcell *arg, oxcell *env) {
    // we will return our result. the data is in the car of the list
    // and any error message is in the cdr.
    //
    oxcell *result = oxcell_factory(oxcList, oxnil, oxnil);
    
    // confirm that our argument is text
    //
    if (arg->kind != oxcText) {
        result->u.list.rest = oxcell_factory(oxcList, oxcell_factory_cstring("input is not text"), oxnil);
        return result;
    }
    
    oxtext *text = arg->u.atom.text;
    
    // skip comments and whitespace
    do {
        while (isspace(text->curr[0])) {
            if (text->curr[0] == '\n') {
                text->lineNumber++;
            }
            text->curr++;
        }
        if (text->curr[0] == '#' || text->curr[1] == ';') {
            while (text->curr[0] && text->curr[0] != '\n') {
                text->curr++;
            }
        }
    } while (isspace(text->curr[0]));
    
    oxtoken *token = malloc(sizeof(*token));
    if (!token) {
        perror(__FUNCTION__);
        exit(2);
    }
    token->lineNumber    = text->lineNumber;
    token->text          = oxnil;
    token->value         = oxnil;
    result->u.list.first = oxcell_factory(oxcToken, token);
    
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
        return result;
    }
    
    if (*startLexeme == ')') {
        token->kind          = oxTokCloseParen;
        token->text          = oxcell_factory_cstring(")");
        token->value         = oxcell_factory_cstring(")");
        return result;
    }
    
    if (*startLexeme == '\'') {
        token->kind          = oxTokSingleQuote;
        token->text          = oxcell_factory_cstring("'");
        token->value         = oxcell_factory_cstring("'");
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
                    result->u.list.first = oxnil;
                    result->u.list.rest  = oxcell_factory(oxcList, oxcell_factory_cstring("incomplete escape sequence"), oxnil);
                    return result;
                }
            }
            ++text->curr;
        }
        
        // only accept strings which have open and close quotes
        //
        if (!(text->curr < text->endOfData && text->curr[0] == *startLexeme)) {
            result->u.list.first = oxnil;
            result->u.list.rest  = oxcell_factory(oxcList, oxcell_factory_cstring("quoted text not terminated"), oxnil);
            return result;
        }
        
        // skip past the closing quote
        //
        text->curr++;
        
        // only accept strings which end at end-of-buffer or a space
        //
        if (!(text->curr < text->endOfData || isspace(text->curr[0]))) {
            result->u.list.first = oxnil;
            result->u.list.rest  = oxcell_factory(oxcList, oxcell_factory_cstring("quoted text not followed by space"), oxnil);
            return result;
        }
        
        // make an exact copy of the input without including the quote marks
        //
        token->text = oxcell_factory(oxcText, oxtext_from_memory(startLexeme + 1, text->curr - startLexeme - 2));
        token->text->u.atom.text->lineNumber = token->lineNumber;
        
        // make a copy without quotes and resolve all escaped characters in the input
        //
        token->value = oxcell_factory(oxcText, oxtext_from_memory(startLexeme + 1, text->curr - startLexeme - 2));
        token->value->u.atom.text->lineNumber = token->lineNumber;
        oxtext_unescape(token->value->u.atom.text);
        
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
            token->text->u.atom.text->lineNumber = token->lineNumber;
            token->value = oxcell_factory(oxcText, oxtext_from_memory(startRawText, endRawText - startRawText));
            token->value->u.atom.text->lineNumber = token->lineNumber;
            
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
            token->text->u.atom.text->lineNumber = token->lineNumber;
            
            // the value is the converted text
            //
            token->value = oxcell_factory(oxcInteger, atol((const char *)(token->text->u.atom.text->data)));
            token->text->u.atom.text->lineNumber = token->lineNumber;
            
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
                token->text->u.atom.text->lineNumber = token->lineNumber;
                
                // the value is the converted text
                //
                token->value = oxcell_factory(oxcInteger, strtod((const char *)(token->text->u.atom.text->data), 0));
                token->text->u.atom.text->lineNumber = token->lineNumber;
                
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
    token->text->u.atom.text->lineNumber = token->lineNumber;
    
    // the value is the same
    //
    token->value = oxcell_factory(oxcText, oxtext_from_memory(startLexeme, text->curr - startLexeme));
    token->text->u.atom.text->lineNumber = token->lineNumber;
    
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
    int doDumpTokens = 0;
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
                oxf_prexpr(text, oxenv);
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
                    oxf_prexpr(token, oxenv);
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
                
            }
#if 0

            a = oxtext_from_file(argv[idx]);
            if (!a) {
                perror(argv[idx]);
                return 0;
            }

            // eval the script
            oxcell *expr;
            for (expr = oxexpr_read(a); expr; expr = oxexpr_read(a)) {
                printf(": ");
                oxcell_print(expr);
                printf("\n");
                oxcell_dump(expr, 0);
                if (expr)                     return 2;
                
                /* NOT REACHED */
                oxcell *result = oxeval(expr, oxenv);
                
                printf("= ");
                oxcell_print(result);
                printf("\n");
            }
#endif
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

    return 0;
}
