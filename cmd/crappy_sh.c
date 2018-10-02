#ifndef CRAPPY_SH
#define CRAPPY_SH

#define MAX_STR 509

#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>

#include <version.h>
#include "../lib/shell.c"
#include "crappy_sh.h"

int main(int argc, char *argv[]) {
    const char *sleep = "", *modulos /* --time */= "", *time_unit = "", *command = "";
    const bool *verbose = false, *env = false;

    parse_cli(argc, argv, &verbose, &env, &sleep, &modulos /* --time */, &time_unit, &command);

    handle_time_delay(&verbose, &sleep, &modulos, &time_unit);

    handle_env_vars(&verbose, &env, &command);

    return strlen(command) ? processInput(command, verbose).return_code : EXIT_SUCCESS;
}

static inline void parse_cli(int argc, char *argv[],
                             const bool **verbose, const bool **env, const char **sleep,
                             const char **modulos /* --time */,
                             const char **time_unit, const char **command) {
    char *previous = NULL;
    for (int ctr = 0; ctr < argc; ctr++) {
        if (strcmp(argv[ctr], "--version") == 0) {
            printf("%s %d.%d.%d\n", argv[0], PROJECT_VERSION_MAJOR, PROJECT_VERSION_MINOR, PROJECT_VERSION_PATCH);
            exit(EXIT_SUCCESS);
        } else if (strcmp(argv[ctr], "--verbose") == 0) goto verbose;
        else if (strcmp(argv[ctr], "--env") == 0) goto env;
        if (previous != NULL && previous[0] == '-') {
            const char opt = previous[1] == '-' ? previous[2] : previous[1];
            switch (opt) {
                case 'v':
                verbose:
                    *verbose = (const bool *) true;
                    break;
                case 'e':
                env:
                    *env = (const bool *) true;
                    break;
                case 's':
                    *sleep = argv[ctr];
                    break;
                case 't':
                    *modulos = argv[ctr];
                    break;
                case 'u':
                    *time_unit = argv[ctr];
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

    if (strlen(*command) + strlen(*sleep) + strlen(*modulos) + strlen(*time_unit) < 3)
        usage_and_exit(argv[0]);
}

static inline void usage_and_exit(const char *argv0) {
    fprintf(stderr, "usage: %s [-h] [--version] [-v] [-e] [-s SLEEP] [-t TIME = 10] [-u UNIT = minute] [-c COMMAND]\n"
                    "\n"
                    "%s runs semicolon separated commands, optionally execute only at <time>.\n"
                    "\n"
                    "  -h, --help                       show this help message and exit\n"
                    "  --version                        show program's version number and exit\n"
                    "  -v, --verbose                    enables verbose output\n"
                    "  -e, --env                        expand environment variables\n"
                    "  -s SLEEP, --sleep SLEEP          SLEEP intervals for time, passed to usleep\n"
                    "  -t TIME, --time TIME             TIME until command can execute, -t 5 will run every 5 units\n"
                    "                                   on clock, e.g.: with minutes 10:05 11:15 will both match \n"
                    "  -u UNIT, --unit INTERVAL         UNIT to check, one of: h[our] m[inute] s[second]\n"
                    "  -c COMMAND, --command COMMAND    COMMAND(s) to run, e.g.: '/bin/ls | /bin/wc -l ; /bin/du -h ;'"
                    "\n",
            argv0, argv0);
    exit(EXIT_FAILURE);
}

static inline int get_clock_val(struct tm timeinfo, const char c) {
    switch (c) {
        case 'h':
            return timeinfo.tm_hour;
        case 'm':
        minute:
            return timeinfo.tm_min;
        case 's':
            return timeinfo.tm_sec;
        default:
            goto minute;
    }
}

static inline void resolve_env_vars(const char **command) {
    static char sbuf[MAX_STR];
    const size_t slen = strlen(*command);
    memset(sbuf, 0, slen);
    size_t cur = 0;
    size_t escaped_idx = slen + 1;
    bool eat = false, brace = false;
    for (size_t i = 0; i < slen; i++) {
        switch ((*command)[i]) {
            case '\\':
                escaped_idx = i;
                break;
            case '$':
                eat = escaped_idx != i - 1;
                break;
            case '{':
                brace = eat;
                break;
                /* Shell and Utilities volume of IEEE Std 1003.1-2001 as referenced in stackoverflow.com/a/2821183
                 * with lowercase ascii characters support also...
                 * Generated with Python `'\n'.join('case \'{}\':'.format(i) for i in string.digits + string.letters)`
                 */
            case '_':
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
            case 'A':
            case 'B':
            case 'C':
            case 'D':
            case 'E':
            case 'F':
            case 'G':
            case 'H':
            case 'I':
            case 'J':
            case 'K':
            case 'L':
            case 'M':
            case 'N':
            case 'O':
            case 'P':
            case 'Q':
            case 'R':
            case 'S':
            case 'T':
            case 'U':
            case 'V':
            case 'W':
            case 'X':
            case 'Y':
            case 'Z':
            case 'a':
            case 'b':
            case 'c':
            case 'd':
            case 'e':
            case 'f':
            case 'g':
            case 'h':
            case 'i':
            case 'j':
            case 'k':
            case 'l':
            case 'm':
            case 'n':
            case 'o':
            case 'p':
            case 'q':
            case 'r':
            case 's':
            case 't':
            case 'u':
            case 'v':
            case 'w':
            case 'x':
            case 'y':
            case 'z':
                if (eat) sbuf[cur++] = (*command)[i];
                break;
            default:
                if (eat) {
                    if (brace) sbuf[cur++] = '}';
                    sbuf[cur] = '\0';
                    const char *res = strlen(sbuf) > 1 ? getenv(sbuf) : NULL;
                    if (res != NULL) {
                        if (brace) prepend(sbuf, "{");
                        prepend(sbuf, "$");
                        *command = repl_str(*command, sbuf, res);
                    }
                }
                memset(sbuf, 0, slen);
                cur = 0;
                eat = false;
        }
    }
}

static inline bool has_env_var(const char **command) {
    const size_t slen = strlen(*command);
    size_t escaped_idx = slen + 1;
    for (size_t i = 0; i < slen; i++)
        switch ((*command)[i]) {
            case '\\':
                escaped_idx = i;
                break;
            case '$':
                if (escaped_idx != i - 1)
                    return true;
            default:
                break;
        }
    return false;
}

static inline void prepend(char *s, const char *t) {
    const size_t len = strlen(t);

    memmove(s + len, s, strlen(s) + 1);

    for (size_t i = 0; i < len; ++i)
        s[i] = t[i];
}

/* stolen from https://creativeandcritical.net/str-replace-c */
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

static inline void handle_time_delay(const bool **verbose, const char **sleep,
                                     const char **modulos, const char **time_unit) {
    if (strlen(*modulos) + strlen(*time_unit) > 0) {
        puts("true time delay");
        for (time_t epoch;;) {
            epoch = time(NULL);
            struct tm timeinfo = *gmtime(&epoch);
            int mod = (int) strtol(*modulos, NULL, 10);
            if (!mod) mod = 10;

            const int clock_val = get_clock_val(timeinfo, (*time_unit)[0]);

            if (verbose)
                printf("Waiting for %d %ss %% %d to be 0, currently %d != 0\n",
                       clock_val, *time_unit, mod, clock_val % mod);
            if (clock_val % mod == 0)
                break;
            else if (strlen(*sleep) > 0) {
                if (verbose)
                    printf("Sleeping for %s\n", *sleep);
                usleep((useconds_t) strtol(*sleep, NULL, 10));
            }
        }
    }
}

static inline void handle_env_vars(const bool **verbose, const bool **env, const char **command) {
    if (*env) {
        *verbose && printf("command before expansion of environment variables:\t\"%s\"\n", *command);

        do resolve_env_vars(*&command);
        while (has_env_var(*&command));

        *verbose && printf("command after expansion of environment variables:\t\"%s\"\n", *command);
    }
}

#endif /* CRAPPY_SH */
