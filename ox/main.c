//
//  ox/main.c
//
//  Created by Michael Henderson on 10/1/14.
//  Copyright (c) 2014 Michael D Henderson. All rights reserved.
//

#include "local.h"

#include <stdio.h>
#include <string.h>

static const char *source =
"{{/* a comment */}}\n"
"{{pipeline}}\n"
"{{if pipeline}} T1 {{end}}\n"
"{{if pipeline}} T1 {{else}} T0 {{end}}\n"
"If the value of the pipeline is empty, T0 is executed;\n"
"{{if pipeline}} T1 {{else if pipeline}} T0 {{end}}\n"
"{{if pipeline}} T1 {{else}}{{if pipeline}} T0 {{end}}{{end}}\n"
"{{range pipeline}} T1 {{end}}\n"
"{{range pipeline}} T1 {{else}} T0 {{end}}\n"
"{{template \"name\"}}\n"
"{{template \"name\" pipeline}}\n"
"{{with pipeline}} T1 {{end}}\n"
"{{with pipeline}} T1 {{else}} T0 {{end}}\n"
"{\"\\\"output\\\"\"}}\n"
"A string constant.\n"
"{{`\"output\"`}}\n"
"A raw string constant.\n"
"{{printf \"%q\" \"output\"}}\n"
"A function call.\n"
"{{\"output\" | printf \"%q\"}}\n"
"A function call whose final argument comes from the previous command.\n"
"{{printf \"%q\" (print \"out\" \"put\")}}\n"
"A parenthesized argument.\n"
"{{\"put\" | printf \"%s%s\" \"out\" | printf \"%q\"}}\n"
"A more elaborate call.\n"
"{{\"output\" | printf \"%s\" | printf \"%q\"}}\n"
"A longer chain.\n"
"{{with \"output\"}}{{printf \"%q\" .}}{{end}}\n"
"A with action using dot.\n"
"{{with $x := \"output\" | printf \"%q\"}}{{$x}}{{end}}\n"
"A with action that creates and uses a variable.\n"
"{{with $x := \"output\"}}{{printf \"%q\" $x}}{{end}}\n"
"A with action that uses the variable in another action.\n"
"{{with $x := \"output\"}}{{$x | printf \"%q\"}}{{end}}\n"
"The same, but pipelined.\n";

static const char *prog =
"int main(int argc, const char * argv[]) {\n"
"    oxwhtlst w;\n"
"    memset(&w, -1, sizeof(w));\n"
"    return 0;\n"
"}\n";

int main(int argc, const char * argv[]) {
    oxwhtlst w;
    memset(&w, -1, sizeof(w));

    oxbuf *a = oxbuf_from_cstring(source);
    a = oxbuf_from_cstring(prog);
    printf("\n\n\n-------------------\n\n\n%s\n\n\n", a->data);

    char *p;
    oxchunk *c;
    int n = 0;
    for (c = oxchunk_factory(a); c; c = oxchunk_factory(a)) {
        switch (c->kind) {
            case oxcComment:
                break;
            case oxcTemplate:
                printf("%s\n", c->data);
                break;
            case oxcText:
                printf("const char *t%09d = \"", n++);
                p = c->data;
                while (*p) {
                    if (*p == '\n') {
                        if (*(p + 1)) {
                            printf("\\n\";\nconst char *t%09d = \"", n++);
                        } else {
                            printf("\\n");
                        }
                    } else if (*p == '"') {
                        printf("\"");
                    } else if (*p == '\\') {
                        printf("\\");
                    } else {
                        printf("%c", *p);
                    }
                    p++;
                }
                printf("\";\n");
                break;
        }
    }

    return 0;
}
