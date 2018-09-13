#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

static inline void parse_cli(int, char *[], const char **, const char **, const char **);

static inline void usage_and_exit(const char *);

static inline void parse_args_and_exec(char *, const char *);

static inline void execute(const char *, char *const *);

int main(int argc, char *argv[]) {
    const char *time = "", *path = "", *command = "";

    parse_cli(argc, argv, &time, &path, &command);

    // TODO: Rather than just `sleep`, implement a run-at `time` functionality
    if (strlen(time) > 0)
        usleep((int) strtol(time, (char **) NULL, 10));

    char *cmd = strdup(command);
    parse_args_and_exec(cmd, path);
    free(cmd);
}

static inline void parse_args_and_exec(char *command, const char *path) {
    printf("command: \"%s\"; path \"%s\"\n", command, path);
    const size_t len = strlen(command);
    char *sbuf[len];
    size_t cur = 0;
    for (size_t i = 0; i < len; i++) {
        printf("command[%zd] = '%c'; \t(int)command[i] \t%lu\t %lu\n", i, command[i], (long)command[i], _CTYPE_S);
        // if (isspace(command[i])) continue;
        switch(command[i]) {
            case _CTYPE_S:
            case ' ':
            case '\t':
            case '\n':
                break;
            case ';':
                sbuf[++cur] = '\0';
                printf("sbuf = \"%s\"\n", *sbuf);
                cur = 0;
                memset(sbuf, cur, len);
            default:
                *sbuf[++cur] = command[i];
        }
    }
}

static inline void parse_cli(int argc, char *argv[], const char **time,
                             const char **path, const char **command) {
    char *previous = NULL;
    for (int ctr = 0; ctr < argc; ctr++) {
        if (strcmp(argv[ctr], "--version") == 0) {
            printf("%s 0.0.1\n", argv[0]);
            exit(EXIT_SUCCESS);
        }
        if (previous != NULL && previous[0] == '-') {
            const char opt = previous[1] == '-' ? previous[2] : previous[1];
            switch (opt) {
                case 't':
                    *time = argv[ctr];
                    break;
                case 'p':
                    *path = argv[ctr];
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

    if (strlen(*command) + strlen(*path) < 2)
        usage_and_exit(argv[0]);
}

static inline void usage_and_exit(const char *argv0) {
    fprintf(stderr, "usage: %s [-h] [--version] [-p PATH] [-t TIME] [-c COMMAND]\n"
                    "\n"
                    "%s runs semicolon separated commands, optionally execute only at <time>.\n"
                    "\n"
                    "  -h, --help                       show this help message and exit\n"
                    "  --version                        show program's version number and exit\n"
                    "  -p PATH, --path PATH             PATH of binaries, e.g.: '/bin'.\n"
                    "  -t TIME, --time TIME             TIME to run COMMAND\n"
                    "  -c COMMAND, --command COMMAND    COMMAND(s) to run, e.g.: 'ls; du;'\n",
            argv0, argv0);
    exit(EXIT_FAILURE);
}

static inline void execute(const char *path, char *const *argv) {
    const pid_t i = fork();
    if (i > 0) {
        char *cmd = (char *) malloc(strlen(path) + strlen(argv[0]) + 2);
        sprintf(cmd, "%s/%s", path, argv[0]);

        const int ret = execv(cmd, argv);
        free(cmd);

        _exit(ret);
    } /*else {
        perror("fork failed");
        _exit(3);
    }*/
}
