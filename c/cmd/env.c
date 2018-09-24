#include "env.h"

int env_main(int argc, char **argv, char** envp) { for(char** env=envp;*env!=0;env++) puts(*env, "\n"); return 0; }
