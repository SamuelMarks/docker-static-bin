#ifndef CRAPPY_SH
#define CRAPPY_SH

#define MAX_STR 509

#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include "../lib/shell.c"

static inline void parse_cli(int, char *[], const char **, const bool **, const char **);

static inline void usage_and_exit(const char *);

static inline void resolve_env_vars(const char **);

int main(int argc, char *argv[]) {
    const char *time = "", *command = "";
    const bool *verbose = false;

    parse_cli(argc, argv, &time, &verbose, &command);

    // TODO: Rather than just `sleep`, implement a run-at `time` functionality
    if (strlen(time) > 0)
        usleep((int) strtol(time, (char **) NULL, 10));

    resolve_env_vars(&command);

    return processInput(command, verbose);
}

static inline void parse_cli(int argc, char *argv[], const char **time,
                             const bool **verbose, const char **command) {
    char *previous = NULL;
    for (int ctr = 0; ctr < argc; ctr++) {
        if (strcmp(argv[ctr], "--version") == 0) {
            printf("%s 0.0.2\n", argv[0]);
            exit(EXIT_SUCCESS);
        } else if (strcmp(argv[ctr], "--verbose") == 0) goto verbose;
        if (previous != NULL && previous[0] == '-') {
            const char opt = previous[1] == '-' ? previous[2] : previous[1];
            switch (opt) {
                case 't':
                    *time = argv[ctr];
                    break;
                case 'v':
                verbose:
                    *verbose = (const bool *) true;
                    break;
                case 'c':
                    *command = argv[ctr];
                    break;
                default:
                    usage_and_exit(argv[0]);
            }
        }
        previous = argv[ctr];
    }

    if (strlen(*command) < 2)
        usage_and_exit(argv[0]);
}

static inline void usage_and_exit(const char *argv0) {
    fprintf(stderr, "usage: %s [-h] [--version] [-v] [-t TIME] [-c COMMAND]\n"
                    "\n"
                    "%s runs semicolon separated commands, optionally execute only at <time>.\n"
                    "\n"
                    "  -h, --help                       show this help message and exit\n"
                    "  --version                        show program's version number and exit\n"
                    "  -v, --verbose                    enables verbose output\n"
                    "  -t TIME, --time TIME             TIME to run COMMAND\n"
                    "  -c COMMAND, --command COMMAND    COMMAND(s) to run, e.g.: '/bin/ls | /bin/wc -l ; /bin/du -h ;'"
                    "\n",
            argv0, argv0);
    exit(EXIT_FAILURE);
}

static inline void resolve_env_vars(const char **command) {
    static char sbuf[MAX_STR];
    const size_t slen = strlen(*command);
    memset(sbuf, 0, slen);
    size_t cur = 0;
    size_t escaped_idx = slen + 1;
    bool eat = false;
    for (size_t i = 0; i < slen; i++) {
        printf("(*command)[%02lu] \t= '%c'\t\tcharcode: %03d\t\t\t\tcur = %lu\n", i, (*command)[i], (*command)[i], cur);
        switch ((*command)[i]) {
            case '\\':
                escaped_idx = i;
                break;
            case '$':
                eat = escaped_idx != i - 1;
                break;
            case '{':
                break;
            case ' ':
            case '}':
                if (eat) {
                    sbuf[cur+1] = '\0';
                    printf("sbuf\t\t\t= \"%s\"\tgetenv(sbuf):\t\t\t\t\"%s\"\n", sbuf, getenv(sbuf));
                    for(size_t j=0; j<strlen(sbuf); j++) {
                        printf("sbuf[%02zd]\t\t= '%c'\t\tcharcode: %03d\n", j, sbuf[j], sbuf[j]);
                    }
                    memset(sbuf, 0, slen);
                }
                cur = 0;
                eat = false;
                break;
            default:
                if (cur > 0)
                    sbuf[cur++] = *command[i];
                break;
        }
    }
    // *command = sbuf;
}

#endif /* CRAPPY_SH */
