#ifndef C_CRAPPY_SH_H
#define C_CRAPPY_SH_H

#include <stdbool.h>

static inline void parse_cli(int, char *[], const bool **, const bool **, const char **,
                             const char **, const char **, const char **);

static inline void usage_and_exit(const char *);

static inline int get_clock_val(struct tm, char);

static inline void resolve_env_vars(const char **);

char *repl_str(const char *, const char *, const char *);

static inline void prepend(char *, const char *);

int crappy_sh_main(int argc, char *argv[]);

#endif //C_CRAPPY_SH_H
