#ifndef CRAPPY_SH
#define CRAPPY_SH

#define MAX_STR 509

#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include "../lib/shell.c"

static inline void parse_cli(int, char *[], const char **, const bool **, const char **);

static inline void usage_and_exit(const char *);

static inline void resolve_env_vars(const char **);

char *repl_str(const char *, const char *, const char *);

static inline void prepend(char *, const char *);

int main(int argc, char *argv[]) {
    const char *time = "", *command = "";
    const bool *verbose = false;

    parse_cli(argc, argv, &time, &verbose, &command);

    // TODO: Rather than just `sleep`, implement a run-at `time` functionality
    if (strlen(time) > 0)
        usleep((int) strtol(time, (char **) NULL, 10));

    resolve_env_vars(&command);

    return processInput(command, verbose);
}

static inline void parse_cli(int argc, char *argv[], const char **time,
                             const bool **verbose, const char **command) {
    char *previous = NULL;
    for (int ctr = 0; ctr < argc; ctr++) {
        if (strcmp(argv[ctr], "--version") == 0) {
            printf("%s 0.0.3\n", argv[0]);
            exit(EXIT_SUCCESS);
        } else if (strcmp(argv[ctr], "--verbose") == 0) goto verbose;
        if (previous != NULL && previous[0] == '-') {
            const char opt = previous[1] == '-' ? previous[2] : previous[1];
            switch (opt) {
                case 't':
                    *time = argv[ctr];
                    break;
                case 'v':
                verbose:
                    *verbose = (const bool *) true;
                    break;
                case 'c':
                    *command = argv[ctr];
                    break;
                default:
                    usage_and_exit(argv[0]);
            }
        }
        previous = argv[ctr];
    }

    if (strlen(*command) < 2)
        usage_and_exit(argv[0]);
}

static inline void usage_and_exit(const char *argv0) {
    fprintf(stderr, "usage: %s [-h] [--version] [-v] [-t TIME] [-c COMMAND]\n"
                    "\n"
                    "%s runs semicolon separated commands, optionally execute only at <time>.\n"
                    "\n"
                    "  -h, --help                       show this help message and exit\n"
                    "  --version                        show program's version number and exit\n"
                    "  -v, --verbose                    enables verbose output\n"
                    "  -t TIME, --time TIME             TIME to run COMMAND\n"
                    "  -c COMMAND, --command COMMAND    COMMAND(s) to run, e.g.: '/bin/ls | /bin/wc -l ; /bin/du -h ;'"
                    "\n",
            argv0, argv0);
    exit(EXIT_FAILURE);
}


char *repl_str(const char *str, const char *from, const char *to) {
    /* Adjust each of the below values to suit your needs. */

    /* Increment positions cache size initially by this number. */
    size_t cache_sz_inc = 16;
    /* Thereafter, each time capacity needs to be increased,
     * multiply the increment by this factor. */
    const size_t cache_sz_inc_factor = 3;
    /* But never increment capacity by more than this number. */
    const size_t cache_sz_inc_max = 1048576;

    char *pret, *ret = NULL;
    const char *pstr2, *pstr = str;
    size_t i, count = 0;
#if (__STDC_VERSION__ >= 199901L)
    uintptr_t *pos_cache_tmp, *pos_cache = NULL;
#else
    ptrdiff_t *pos_cache_tmp, *pos_cache = NULL;
#endif
    size_t cache_sz = 0;
    size_t cpylen, orglen, retlen, tolen, fromlen = strlen(from);

    /* Find all matches and cache their positions. */
    while ((pstr2 = strstr(pstr, from)) != NULL) {
        count++;

        /* Increase the cache size when necessary. */
        if (cache_sz < count) {
            cache_sz += cache_sz_inc;
            pos_cache_tmp = realloc(pos_cache, sizeof(*pos_cache) * cache_sz);
            if (pos_cache_tmp == NULL) {
                goto end_repl_str;
            } else pos_cache = pos_cache_tmp;
            cache_sz_inc *= cache_sz_inc_factor;
            if (cache_sz_inc > cache_sz_inc_max) {
                cache_sz_inc = cache_sz_inc_max;
            }
        }

        pos_cache[count - 1] = pstr2 - str;
        pstr = pstr2 + fromlen;
    }

    orglen = pstr - str + strlen(pstr);

    /* Allocate memory for the post-replacement string. */
    if (count > 0) {
        tolen = strlen(to);
        retlen = orglen + (tolen - fromlen) * count;
    } else retlen = orglen;
    ret = malloc(retlen + 1);
    if (ret == NULL)
        goto end_repl_str;

    if (count == 0)
        /* If no matches, then just duplicate the string. */
        strcpy(ret, str);
    else {
        /* Otherwise, duplicate the string whilst performing
         * the replacements using the position cache. */
        pret = ret;
        memcpy(pret, str, pos_cache[0]);
        pret += pos_cache[0];
        for (i = 0; i < count; i++) {
            memcpy(pret, to, tolen);
            pret += tolen;
            pstr = str + pos_cache[i] + fromlen;
            cpylen = (i == count - 1 ? orglen : pos_cache[i + 1]) - pos_cache[i] - fromlen;
            memcpy(pret, pstr, cpylen);
            pret += cpylen;
        }
        ret[retlen] = '\0';
    }

    end_repl_str:
    /* Free the cache and return the post-replacement string,
     * which will be NULL in the event of an error. */
    free(pos_cache);
    return ret;
}

static inline void prepend(char *s, const char *t) {
    const size_t len = strlen(t);

    memmove(s + len, s, strlen(s) + 1);

    for (size_t i = 0; i < len; ++i)
        s[i] = t[i];
}

static inline void resolve_env_vars(const char **command) {
    static char sbuf[MAX_STR];
    const size_t slen = strlen(*command);
    memset(sbuf, 0, slen);
    size_t cur = 0;
    size_t escaped_idx = slen + 1;
    bool eat = false;
    for (size_t i = 0; i < slen; i++) {
        switch ((*command)[i]) {
            case '\\':
                escaped_idx = i;
                break;
            case '$':
                eat = escaped_idx != i - 1;
                break;
            case '{':
                break;
            case ' ':
            case '}':
                if (eat) {
                    sbuf[cur] = '\0';
                    const char *res = getenv(sbuf);
                    if (res != NULL) {
                        prepend(sbuf, "$");
                        *command = repl_str(*command, sbuf, res);
                    }
                    memset(sbuf, 0, slen);
                }
                cur = 0;
                eat = false;
                break;
            default:
                if (eat) sbuf[cur++] = (*command)[i];
        }
    }
}

#endif /* CRAPPY_SH */
