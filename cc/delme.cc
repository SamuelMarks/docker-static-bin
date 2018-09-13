#include <vector>

static inline void parse_cli(int, char *[], const char *&, const char *&);

static inline void usage_and_exit(const char *);

static inline void parse_args_and_exec(char *, const char *);

static inline const std::vector<const char *> split(const char *, const char *);

static inline void *trim(char *&);

int main(int argc, char *argv[]) {
    const char *path = "", *command = "";

    parse_cli(argc, argv, path, command);

    char *cmd = strdup(command);
    parse_args_and_exec(cmd, path);
    free(cmd);
}

static inline void parse_args_and_exec(char *command, const char *path) {
    printf("command: \"%s\"\n", command);
    for (char *cmd = strtok(command, ";"); cmd; cmd = strtok(NULL, ";")) {
        trim(cmd);
        printf("cmd: \"%s\"\n", cmd);
    }
    puts("\n");

    for (char *cmd = strtok(command, ";"); cmd; cmd = strtok(NULL, ";")) {
        trim(cmd);

        const std::vector<const char *> arguments = split(cmd, " ");

        printf("##\tcmd: \"%s\"\n", cmd);
        printf("##\tpath: \"%s\"\n", path);
    }
}

static inline void parse_cli(int argc, char *argv[], const char *&path, const char *&command) {
    char *previous = NULL;
    for (int ctr = 0; ctr < argc; ctr++) {
        if (strcmp(argv[ctr], "--version") == 0) {
            printf("%s 0.0.1\n", argv[0]);
            exit(EXIT_SUCCESS);
        }
        if (previous != NULL && previous[0] == '-') {
            const char opt = previous[1] == '-' ? previous[2] : previous[1];
            switch (opt) {
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
    fprintf(stderr, "usage: %s [-h] [--version] [-p PATH] [-c COMMAND]\n"
                    "\n"
                    "%s runs semicolon separated commands, optionally execute only at <time>.\n"
                    "\n"
                    "  -h, --help                       show this help message and exit\n"
                    "  --version                        show program's version number and exit\n"
                    "  -p PATH, --path PATH             PATH of binaries, e.g.: '/bin'.\n"
                    "  -c COMMAND, --command COMMAND    COMMAND(s) to run, e.g.: 'ls; du;'\n",
            argv0, argv0);
    exit(EXIT_FAILURE);
}

static inline void *trim(char *&s) {
    char *end;

    while (isspace((unsigned char) *s) || ispunct((unsigned char) *s)) s++;

    if (*s == 0)
        return s;

    end = s + strlen(s) - 1;
    while (end > s && isspace((unsigned char) *end) && ispunct((unsigned char) *s)) end--;

    end[1] = '\0';
}

char** str_split(char* a_str, const char a_delim)
{
    char** result    = 0;
    size_t count     = 0;
    char* tmp        = a_str;
    char* last_comma = 0;
    char delim[2];
    delim[0] = a_delim;
    delim[1] = 0;

    /* Count how many elements will be extracted. */
    while (*tmp)
    {
        if (a_delim == *tmp)
        {
            count++;
            last_comma = tmp;
        }
        tmp++;
    }

    /* Add space for trailing token. */
    count += last_comma < (a_str + strlen(a_str) - 1);

    /* Add space for terminating null string so caller
       knows where the list of returned strings ends. */
    count++;

    result = malloc(sizeof(char*) * count);

    if (result)
    {
        size_t idx  = 0;
        char* token = strtok(a_str, delim);

        while (token)
        {
            assert(idx < count);
            *(result + idx++) = strdup(token);
            token = strtok(0, delim);
        }
        assert(idx == count - 1);
        *(result + idx) = 0;
    }

    return result;
}
