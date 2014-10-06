//
//  ox/oxutil.c
//
//  Created by Michael Henderson on 10/6/14.
//  Copyright (c) 2014 Michael D Henderson. All rights reserved.
//

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

int islist(oxcell *c) {
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

