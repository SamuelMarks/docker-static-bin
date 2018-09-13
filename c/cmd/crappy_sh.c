#ifndef CRAPPY_SH
#define CRAPPY_SH

#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include "../lib/shell.c"

static inline void parse_cli(int, char *[], const char **, const bool **, const char **);

static inline void usage_and_exit(const char *);

int main(int argc, char *argv[]) {
    const char *time = "", *command = "";
    const bool *verbose = false;

    parse_cli(argc, argv, &time, &verbose, &command);

    // TODO: Rather than just `sleep`, implement a run-at `time` functionality
    if (strlen(time) > 0)
        usleep((int) strtol(time, (char **) NULL, 10));

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

#endif /* CRAPPY_SH */
