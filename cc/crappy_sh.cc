#include <string>
#include <unistd.h>
#include <vector>
#include <iostream>

static inline void parse_cli(int, char *[], const char *&, const char *&, const char *&);

static inline void usage_and_exit(const char *);

static inline void parse_args_and_exec(const char *, const char *);

static inline const std::vector<const char *> split(char *, const char *);

static inline void execute(const char *, char *const *);

static inline void ltrim(std::string &);

static inline void rtrim(std::string &);

static inline void trim(std::string &);

int main(int argc, char *argv[]) {
    const char *time = "", *path = "", *command = "";

    parse_cli(argc, argv, time, path, command);

    // TODO: Rather than just `sleep`, implement a run-at `time` functionality
    if (strlen(time) > 0)
        usleep((int) strtol(time, (char **) nullptr, 10));

    parse_args_and_exec(command, path);
}

static inline void parse_args_and_exec(const char *command, const char *path) {
    const std::string cmd = command, delim = ";";

    size_t start = 0U, end = cmd.find(delim);

    while (end != std::string::npos) {
        std::string token = cmd.substr(start, end - start);
        trim(token);

        const std::vector<const char *> arguments = split((char *) token.c_str(), " ");

        if (arguments.empty()) {
            char *args[] = {strdup(token.c_str())};
            execute(path, args);
            free(args[0]);
        } else {
            char *args[arguments.size() + 1];
            std::cout << arguments.size();
            for (size_t i = 0; i < arguments.size(); i++) {
                printf("i = %d\n", (int)i);
                args[i] = strdup(arguments[i]);
            }
            args[arguments.size()] = nullptr;
            execute(path, args);
            for (size_t i = 0; i < arguments.size() - 1; i++)
                free(args[i]);
        }
        start = end + delim.length();
        end = cmd.find(delim, start);
    }
}

static inline void parse_cli(int argc, char *argv[], const char *&time, const char *&path, const char *&command) {
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

static inline void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
        return !std::isspace(ch) && ch != '\'';
    }));
}

static inline void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
        return !std::isspace(ch) && ch != '\'';
    }).base(), s.end());
}

static inline void trim(std::string &s) {
    ltrim(s);
    rtrim(s);
}

static inline const std::vector<const char *> split(char *s, const char *delim) {
    std::vector<const char *> tokens;

    for (char *token = strtok(s, delim); token; token = strtok(nullptr, " "))
        tokens.push_back(token);

    return tokens;
}

