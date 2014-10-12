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

int main(int argc, const char * argv[]) {
    int debugLevel   = 0;
    int doCheck      = 1;
    int doDumpInput  = 0;
    int doDumpTokens = 1;
    int doEval       = 0;
    int doOptions    = 1;
    int idx;

    for (idx = 1; idx < argc; idx++) {
        printf("..opt: %3d %s\n", idx, argv[idx]);
        if (!doOptions || !(argv[idx][0] == '-' && argv[idx][1] == '-')) {
            // treat as a file name
            //
            if (doDumpInput) {
                printf("\n----------------------------------\n");
            }

            if (doDumpTokens) {
                printf("\n----------------------------------\n");
            }

            if (doEval) {
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
