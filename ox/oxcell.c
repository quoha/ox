//
//  ox/oxcell.c
//
//  Created by Michael Henderson on 10/7/14.
//  Copyright (c) 2014 Michael D Henderson. All rights reserved.
//
#if 0
#include "oxcell.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

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
    }
    
    va_end(ap);

    return c;
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

oxtext *oxtext_from_cstring(const char *cstring) {
    cstring = cstring ? cstring : "";
    return oxtext_from_memory((unsigned char *)cstring, strlen(cstring));
}

oxtext *oxtext_from_memory(unsigned char *memory, size_t length) {
    oxtext *text = malloc(sizeof(*text));
    if (!text) {
        perror(__FUNCTION__);
        exit(2);
    }

    text->curr       = text->data;
    text->endOfData  = text->data + length;
    text->length     = length;
    text->lineNumber = 0;

    memcpy(text->data, memory, length);
    text->data[text->length] = 0;

    // revisit if jenkin's hash turns out to have issues. never has
    // in the past, ya know.
    //
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
#endif