#ifndef CRAPPY_SH_H
#define CRAPPY_SH_H

#include <stdbool.h>
#include <time.h>

static inline void parse_cli(int, char *[], const bool **, const bool **, const char **,
                             const char **, const char **, const char **);

static inline void usage_and_exit(const char *);

static inline int get_clock_val(struct tm, char);

static inline void resolve_env_vars(const char **);

static inline bool has_env_var(const char **);

char *repl_str(const char *, const char *, const char *);

static inline void prepend(char *, const char *);

static inline void handle_time_delay(const bool **, const char **, const char **, const char **);

static inline void handle_env_vars(const bool **, const bool **, const char **);

#endif //CRAPPY_SH_H
