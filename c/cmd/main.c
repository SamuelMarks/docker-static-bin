#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "copy.h"
#include "crappy_sh.h"
#include "env.h"
#include "list.h"
#include "symlink.h"

int main(int argc, char **argv, char **envp) {
    const size_t slen = strlen(argv[0]);
    char exec_name[slen];
    size_t c = 0;
    for (size_t i = 0; i < slen; i++)
        switch (argv[0][i]) {
            case '/':
                memset(exec_name, 0, slen);
                c = 0;
                break;
            default:
                exec_name[c++] = argv[0][i];
        }
    exec_name[c] = '\0';

    if (strcmp(exec_name, "copy") == 0 || strcmp(exec_name, "cp") == 0)
        return copy_main(argc, argv);
    else if (strcmp(exec_name, "env") == 0)
        return env_main(argc, argv, envp);
    else if (strcmp(exec_name, "crappy_sh") == 0 || strcmp(exec_name, "bash") == 0 || strcmp(exec_name, "sh") == 0)
        return crappy_sh_main(argc, argv);
    else if (strcmp(exec_name, "list") == 0 || strcmp(exec_name, "ls") == 0)
        return list_main(argc, argv);
    else if (strcmp(exec_name, "symlink") == 0 || strcmp(exec_name, "ln") == 0)
        return symlink_main(argc, argv);

    fprintf(stderr, "Rather than %s; use one of the symbolically linked: copy; env; crappy_sh; list\n", exec_name);
    exit(EXIT_FAILURE);
}
