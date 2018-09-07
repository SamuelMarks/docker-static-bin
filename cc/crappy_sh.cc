#include <string>
// #include <vector>
#include <unistd.h>

static inline void execute(const char*, const char*);
static inline void ltrim(std::string&);
static inline void rtrim(std::string&);
static inline void trim(std::string&);

int main(int argc, char *argv[]) {
    if (argv[1] == NULL || argv[2] == NULL) {
      printf("Usage %s PATH 'cmd0 ; cmd1 ; ...\n", argv[0]);
      exit(2);
    }

    const std::string delim = ";", s = argv[2];

    unsigned start = 0U;
    size_t end = s.find(delim);
    // std::vector<std::string> tokens;
    /*if (end == std::string::npos) {
        execute(argv[1], s.c_str());
        // tokens.push_back(s);
    }
    else */while (end != std::string::npos) { 
        std::string token = s.substr(start, end - start);
        trim(token);
        
        execute(argv[1], token.c_str());
        // tokens.push_back(s.substr(start, end - start));
        start = end + delim.length();
        end = s.find(delim, start);
    }
}

static inline void execute(const char* path, const char* command) {
    const pid_t i = fork();
    if (i > 0) {
        char *cmd = (char*)malloc(strlen(path) + strlen(command) + 2);
        sprintf(cmd, "%s/%s", path, command);

        const int ret = execl(cmd, command, "-1", (char *)0);
        free(cmd);
        _exit(ret);
    } /*else {
        perror("fork failed");
        _exit(3);
    }*/
}

static inline void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
        return !std::isspace(ch);
    }));
}

static inline void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

static inline void trim(std::string &s) {
    ltrim(s);
    rtrim(s);
}