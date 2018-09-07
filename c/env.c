int main(int argc, char **argv, char** envp) { for(char** env=envp;*env!=0;env++) puts(*env, "\n"); }
