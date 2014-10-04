//
//  ox/main.c
//
//  Created by Michael Henderson on 10/1/14.
//  Copyright (c) 2014 Michael D Henderson. All rights reserved.
//

#include "local.h"
#include "oxparser.h"

#include <stdio.h>
#include <string.h>

int main(int argc, const char * argv[]) {
    int debugLevel = 0;
    int doCheck    = 0;
    int doOpt      = 1;
    int idx;

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
                return 0;
            }
            // execute the script
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
                doCheck++;
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
