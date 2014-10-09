//
//  ox/oxutil.c
//
//  Created by Michael Henderson on 10/6/14.
//  Copyright (c) 2014 Michael D Henderson. All rights reserved.
//
#if 0
#include "oxalloc.h"
#include "oxinit.h"
#include "oxutil.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

int isatom(oxcell *c) {
    if (c) {
        if (c->kind == octAtom) {
            return -1;
        }
    }
    return 0;
}

int islist(oxcell *c) {
    if (c) {
        if (!isatom(c)) {
            return -1;
        }
    }
    return 0;
}

int isnil(oxcell *c) {
    if (c == oxnil) {
        return -1;
    }
    return 0;
}

int isNumber(oxcell *c) {
    if (c) {
        if (isatom(c)) {
            if (c->atom->kind == eAtomInteger) {
                return -1;
            }
            if (c->atom->kind == eAtomReal) {
                return -1;
            }
        }
    }
    return 0;
}

int issymbol(oxcell *c) {
    if (c) {
        if (!isatom(c)) {
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

oxcell *oxcell_get_first(oxcell *c) {
    if (!isatom(c)) {
        return c->first ? c->first : oxnil;
    }
    return oxnil;
}

oxcell *oxcell_get_rest(oxcell *c) {
    if (!isatom(c)) {
        return c->rest ? c->rest : oxnil;
    }
    return oxnil;
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
        return oxcell_get_first(c);
    }
    return oxnil;
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
        return oxcell_get_rest(c);
    }
    return oxnil;
}

oxcell *oxcell_set_first(oxcell *c, oxcell *first) {
    if (c) {
        if (c->kind == octList) {
            c->first = first;
            return c->first;
        }
    }
    
    if (1) {
        printf("error: internal error - setCar on non cons");
        exit(2);
    }
    return c;
}

oxcell *oxcell_set_rest(oxcell *c, oxcell *rest) {
    if (c) {
        if (c->kind == octList) {
            c->rest = rest;
            return c->rest;
        }
    }
    
    if (1) {
        printf("error: internal error - setCdr on non cons");
        exit(2);
    }
    return c;
}

//   ((key value) (key value))
//   first|rest -> first|rest -> nill
//     |             |
//     |             v
//     |           first|rest -> first|rest -> nill
//     |             |             |
//     |             v             v
//     |            key          value
//     |
//     v
//   first|rest -> first|rest -> nill
//     |             |
//     v             v
//    key          value
//
oxcell *oxassoc_find_key(oxcell *assoc, oxcell *key) {
    oxcell *entry;
    for (entry = assoc; entry != oxnil; entry = oxcell_get_rest(entry)) {
        oxcell *kv = oxcell_get_first(entry);
        if (oxcell_eq_text(key, kv)) {
            return entry;
        }
    }

    return oxnil;
}

// symbol stores information in an associative list
//
oxcell *oxsym_lookup(oxcell *name, oxcell *env) {
    for (; env != oxnil; env = oxcell_get_first(env)) {
        oxcell *entry = oxcell_get_rest(env);
        if (issymbol(entry)) {
            if (oxcell_eq_text(name, oxcell_get_name(entry))) {
                return entry;
            }
        }
    }
    
    return oxnil;
}


int oxcell_eq_text(oxcell *c1, oxcell *c2) {
    if (c1 && c2) {
        if (isatom(c1) && isatom(c2)) {
            if (istext(c1) && istext(c2)) {
                if (c1 != oxnil && c2 != oxnil) {
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

#endif