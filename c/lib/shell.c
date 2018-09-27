#ifndef SHELL
#define SHELL

// Butchered https://github.com/prakhar1989/shell

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <ctype.h>
#include <stdbool.h>
#include "shell.h"

/* useful for keeping track of parent's prev call for cleanup */
static struct command *parent_cmd;
static struct commands *parent_cmds;
static char *temp_line;

static const PidCode pid_code_nulled = {0, 0};

/* Frees up memory for the commands */
void cleanup_commands(struct commands *cmds) {
    for (int i = 0; i < cmds->cmd_count; i++)
        free(cmds->cmds[i]);

    free(cmds);
}

/* Parses a single command into a command struct.
 * Allocates memory for keeping the struct and the caller is responsible
 * for freeing up the memory
 */
struct command *parse_command(char *input) {
    int tokenCount = 0;
    char *token;

    /* allocate memory for the cmd structure */
    struct command *cmd = calloc(sizeof(struct command) +
                                 ARG_MAX_COUNT * sizeof(char *), 1);

    if (cmd == NULL) {
        fprintf(stderr, "error: memory alloc error\n");
        exit(EXIT_FAILURE);
    }

    /* get token by splitting on whitespace */
    token = strtok(input, " ");

    while (token != NULL && tokenCount < ARG_MAX_COUNT) {
        cmd->argv[tokenCount++] = token;
        token = strtok(NULL, " ");
    }
    cmd->name = cmd->argv[0];
    cmd->argc = tokenCount;
    return cmd;
}

/* Parses a command with pipes into a commands* structure.
 * Allocates memory for keeping the struct and the caller is responsible
 * for freeing up the memory
 */
struct commands *parse_commands_with_pipes(char *input) {
    int commandCount = 0;
    int i = 0;
    char *token;
    char *saveptr;
    char *c = input;
    struct commands *cmds;

    while (*c != '\0') {
        if (*c == '|')
            commandCount++;
        c++;
    }

    commandCount++;

    cmds = calloc(sizeof(struct commands) +
                  commandCount * sizeof(struct command *), 1);

    if (cmds == NULL) {
        fprintf(stderr, "error: memory alloc error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok_r(input, "|", &saveptr);
    while (token != NULL && i < commandCount) {
        cmds->cmds[i++] = parse_command(token);
        token = strtok_r(NULL, "|", &saveptr);
    }

    cmds->cmd_count = commandCount;
    return cmds;
}

int is_blank(char *input) {
    int n = (int) strlen(input);
    int i;

    for (i = 0; i < n; i++) {
        if (!isspace(input[i]))
            return 0;
    }
    return 1;
}

/* closes all the pipes */
void close_pipes(int (*pipes)[2], int pipe_count) {
    for (int i = 0; i < pipe_count; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

}

/* Returns whether a command is a built-in. As of now
 * one of [exit, cd]
 */
int check_built_in(struct command *cmd) {
    return strcmp(cmd->name, "exit") == 0 ||
           strcmp(cmd->name, "cd") == 0;
}

/* Handles the shell built-in commands. Takes the input/output file descriptors
 * to execute the built-in against. Since none of the built-ins read input from
 * STDIN, only the value of output fd is to be used that too only for the
 * history command since that is the only command that writes to STDOUT.
 *
 * Returns -1 to indicate that program should exit.
 */
const PidCode handle_built_in(struct commands *cmds, struct command *cmd) {
    int ret;

    if (strcmp(cmd->name, "exit") == 0) {
        const PidCode pid_code = {0, -1};
        return pid_code;
    }

    if (strcmp(cmd->name, "cd") == 0) {
        ret = chdir(cmd->argv[1]);
        if (ret != 0) {
            fprintf(stderr, "error: unable to change dir\n");
            const PidCode pid_code = {0, ret};
            return pid_code;
        }
        return pid_code_nulled;
    }
    return pid_code_nulled;
}

/* Executes a command by forking of a child and calling exec.
 * Causes the calling progress to halt until the child is done executing.
 */
const PidCode exec_command(struct commands *cmds, struct command *cmd, int (*pipes)[2]) {
    if (check_built_in(cmd) == 1)
        return handle_built_in(cmds, cmd);

    pid_t child_pid = fork();

    if (child_pid == -1) {
        fprintf(stderr, "error: fork error\n");
        return pid_code_nulled;
    } /* in the child */ else if (child_pid == 0) {
        int input_fd = cmd->fds[0];
        int output_fd = cmd->fds[1];

        // change input/output file descriptors if they aren't standard
        if (input_fd != -1 && input_fd != STDIN_FILENO)
            dup2(input_fd, STDIN_FILENO);

        if (output_fd != -1 && output_fd != STDOUT_FILENO)
            dup2(output_fd, STDOUT_FILENO);

        if (pipes != NULL) {
            int pipe_count = cmds->cmd_count - 1;

            close_pipes(pipes, pipe_count);
        }

        /* execute the command */
        execv(cmd->name, cmd->argv);

        /* execv returns only if an error occurs */
        fprintf(stderr, "error: %s\n", strerror(errno));

        /* cleanup in the child to avoid memory leaks */
        free(pipes);
        cleanup_commands(cmds);

        if (parent_cmd != NULL) {
            free(parent_cmd);
            free(temp_line);
            free(parent_cmds);
        }

        /* exit from child so that the parent can handle the scenario*/
        _exit(EXIT_FAILURE);
    } else {
        int child_status;
        waitpid(child_pid, &child_status, 0);

        const PidCode pid_code = {child_pid, child_status};
        return pid_code;
    }
}

/* Executes a set of commands that are piped together.
 * If it's a single command, it simply calls `exec_command`.
 */
const PidCode exec_commands(struct commands *cmds) {
    PidCode pid_code = {.pid = -1};

    /* single command? run it */
    if (cmds->cmd_count == 1) {
        cmds->cmds[0]->fds[STDIN_FILENO] = STDIN_FILENO;
        cmds->cmds[0]->fds[STDOUT_FILENO] = STDOUT_FILENO;
        exec_command(cmds, cmds->cmds[0], NULL);
    } else {
        /* execute a pipeline */
        int pipe_count = cmds->cmd_count - 1;

        /* if any command in the pipeline is a built-in, raise error */
        int i;

        for (i = 0; i < cmds->cmd_count; i++) {
            if (check_built_in(cmds->cmds[i])) {
                fprintf(stderr, "error: no builtins in pipe\n");
                pid_code.return_code = 0;
                return pid_code;
            }
        }

        /* allocate an array of pipes. Each member is array[2] */
        int (*pipes)[2] = calloc(pipe_count * sizeof(int[2]), 1);

        if (pipes == NULL) {
            fprintf(stderr, "error: memory alloc error\n");
            pid_code.return_code = 0;
            return pid_code;
        }


        /* create pipes and set file descriptors on commands */
        cmds->cmds[0]->fds[STDIN_FILENO] = STDIN_FILENO;
        for (i = 1; i < cmds->cmd_count; i++) {
            pipe(pipes[i - 1]);
            cmds->cmds[i - 1]->fds[STDOUT_FILENO] = pipes[i - 1][1];
            cmds->cmds[i]->fds[STDIN_FILENO] = pipes[i - 1][0];
        }
        cmds->cmds[pipe_count]->fds[STDOUT_FILENO] = STDOUT_FILENO;

        /* execute the commands */
        for (i = 0; i < cmds->cmd_count; i++)
            pid_code = exec_command(cmds, cmds->cmds[i], pipes);
        /* only the last one matters */

        close_pipes(pipes, pipe_count);

        /* wait for children to finish */
        for (i = 0; i < cmds->cmd_count; ++i)
            wait(NULL);

        free(pipes);
    }

    return pid_code;
}

const PidCode run_cmd(const char *s) {
    PidCode pid_code = pid_code_nulled;
    char *input = strdup(s);

    if (input == NULL || !strlen(input))
        return pid_code_nulled;

    if (strlen(input) > 0 && !is_blank(input) && input[0] != '|') {
        char *linecopy = strdup(input);

        struct commands *commands = parse_commands_with_pipes(input);

        free(linecopy);
        pid_code = exec_commands(commands);
        cleanup_commands(commands);
    }

    free(input);

    return pid_code;
}

static inline PidCode run_command(const char *sbuf, const bool verbose) {
    if (!strlen(sbuf)) return pid_code_nulled;
    else if (verbose) {
        printf("cmd: \"%s\"\n", sbuf);
        return run_cmd(sbuf);
    }
    return run_cmd(sbuf);
}

/* Process all input commands */
const PidCode processInput(const char *in, const bool verbose) {
    const size_t slen = strlen(in);
    char sbuf[slen];
    PidCode last_pid_code = pid_code_nulled;

    for (size_t i = 0, cur = 0; i < slen; i++) {
        switch (in[i]) {
            case ';':
                for (size_t j = cur; j != 0; j--) {
                    switch (sbuf[j]) {
                        case '\n':
                        case '\t':
                        case '\b':
                        case '\r':
                        case ' ':
                        case '\0':
                        case 1:
                        case '':
                        space:
                            break;
                        default:
                            if (isspace((unsigned char) sbuf[i]))
                                goto space;

                            sbuf[cur - 1] = '\0';
                            goto end;
                    }
                }
            end:
                last_pid_code = run_command(sbuf, verbose);
                cur = 0;
                memset(sbuf, 0, slen);
                break;
            case '\n':
            case '\t':
            case '\b':
            case '\r':
            case ' ':
                if (cur == 0) break;
            default:
                sbuf[cur++] = in[i];
        }
    }

    const PidCode pid0 = run_command(sbuf, verbose);
    if (!pid0.return_code)
        return last_pid_code;
    return pid0;
}

#endif /* SHELL */
