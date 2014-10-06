//
//  ox/main.c
//
//  Created by Michael Henderson on 10/1/14.
//  Copyright (c) 2014 Michael D Henderson. All rights reserved.
//

#include "oxtype.h"
#include "oxeval.h"
#include "oxinit.h"
#include "oxparser.h"
#include "oxprint.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, const char * argv[]) {
    int debugLevel = 0;
    int doCheck    = 0;
    int doEval     = 1;
    int doOpt      = 1;
    int idx;

    oxinit();
    
    oxbuf *a = 0;

    for (idx = 1; idx < argc; idx++) {
        printf("..opt: %3d %s\n", idx, argv[idx]);
        if (!doOpt || !(argv[idx][0] == '-' && argv[idx][1] == '-')) {
            // treat as a file name
            a = oxbuf_from_file(argv[idx]);
            if (!a) {
                perror(argv[idx]);
                return 0;
            }
            if (doCheck) {
                oxtoken *t;
                for (t = oxtok_read(a); t && t->kind != oxTokEOF; t = oxtok_read(a)) {
                    printf("%5d: %s %s\n", t->line, oxtok_toktype(t), t->data);
                    free(t);
                }
                if (t) {
                    free(t);
                }
            }

            if (doEval) {
                // eval the script
                oxcell *expr;
                for (expr = oxexpr_read(a); expr; expr = oxexpr_read(a)) {
                    printf(": ");
                    oxcell_print(expr);
                    printf("\n");
                    
                    oxcell *env    = myEnv;
                    oxcell *result = oxeval(expr, env);

                    printf("= ");
                    oxcell_print(result);
                    printf("\n");
                }
            }
        } else if (argv[idx][0] == '-' && argv[idx][1] == '-' && argv[idx][3] == 0) {
            doOpt = 0;
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
            } else if (!strcmp(opt, "--check-file")) {
                doCheck = -1;
                doEval  =  0;
            } else if (!strcmp(opt, "--eval-file")) {
                doCheck =  0;
                doEval  = -1;
            } else if (!strcmp(opt, "--debug")) {
                debugLevel++;
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
