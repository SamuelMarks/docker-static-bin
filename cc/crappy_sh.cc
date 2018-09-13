#include <vector>
#include <unistd.h>

static inline void parse_cli(int, char *[], const char *&, const char *&, const char *&);

static inline void usage_and_exit(const char *);

static inline void parse_args_and_exec(char *, const char *);

static inline const std::vector<const char *> split(const char *, const char *);

static inline void execute(const char *, char *const *);

static inline void *trim(char *&);

int main(int argc, char *argv[]) {
    const char *time = "", *path = "", *command = "";

    parse_cli(argc, argv, time, path, command);

    // TODO: Rather than just `sleep`, implement a run-at `time` functionality
    if (strlen(time) > 0)
        usleep((int) strtol(time, (char **) nullptr, 10));

    char *cmd = strdup(command);
    parse_args_and_exec(cmd, path);
    free(cmd);
}

static inline void parse_args_and_exec(char *command, const char *path) {
    printf("command: \"%s\"\n", command);
    for (char *cmd = strtok(command, ";"); cmd; cmd = strtok(nullptr, ";")) {
        trim(cmd);
        printf("cmd: \"%s\"\n", cmd);
    }
    puts("\n");

    for (char *cmd = strtok(command, ";"); cmd; cmd = strtok(nullptr, ";")) {
        trim(cmd);

        const std::vector<const char *> arguments = split(cmd, " ");

        printf("##\tcmd: \"%s\"\n", cmd);
        printf("##\tpath: \"%s\"\n", path);

        /*if (arguments.empty()) {
            char *args[] = {tok};
            execute(path, args);
        } else {
            char *args[arguments.size() + 1];
            for (size_t i = 0; i < arguments.size(); i++)
                args[i] = strdup(arguments[i]);
            args[arguments.size()] = nullptr;

            execute(path, args);

            for (size_t i = 0; i < arguments.size(); i++)
                free(args[i]);
        }
        free(tok);
        puts("free(tok)");
         */
    }
}

static inline void parse_cli(int argc, char *argv[], const char *&time,
                             const char *&path, const char *&command) {
    char *previous = nullptr;
    for (int ctr = 0; ctr < argc; ctr++) {
        if (strcmp(argv[ctr], "--version") == 0) {
            printf("%s 0.0.1\n", argv[0]);
            exit(EXIT_SUCCESS);
        }
        if (previous != nullptr && previous[0] == '-') {
            const char opt = previous[1] == '-' ? previous[2] : previous[1];
            switch (opt) {
                case 't':
                    time = argv[ctr];
                    break;
                case 'p':
                    path = argv[ctr];
                    break;
                case 'c':
                    command = argv[ctr];
                    break;
                default:
                    usage_and_exit(argv[0]);
            }
        }
        previous = argv[ctr];
    }

    if (strlen(command) + strlen(path) < 2)
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

static inline void *trim(char *&s) {
    char *end;

    while (isspace((unsigned char) *s) || ispunct((unsigned char) *s)) *s++;

    if (*s == 0)
        return s;

    end = s + strlen(s) - 1;
    while (end > s && isspace((unsigned char) *end) && ispunct((unsigned char) *s)) end--;

    end[1] = '\0';

    return s;
}

static inline const std::vector<const char *> split(const char *s, const char *delim) {
    char *tok = strdup(s);

    std::vector<const char *> tokens;

    for (const char *token = strtok(tok, delim); token; token = strtok(nullptr, " "))
        tokens.push_back(token);

    free(tok);

    return tokens;
}
