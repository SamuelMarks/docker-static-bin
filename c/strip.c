#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

static inline void trim(char **);

int main(int argc, char *argv[]) {
    const char *s = "   ; \"f\" ;     ";
    char *strip_s = strdup(s);
    trim(&strip_s);
    printf("s = \"%s\"; strip_s = \"%s\"\n", s, strip_s);
    free(*strip_s);
}

static inline void trim(char **s) {
    char *end;

    while (isspace((unsigned char) **s) || ispunct((unsigned char) **s)) ++(*s);

    if (*s == 0) return;

    end = *s + strlen(*s) - 1;
    while (end > *s && (isspace((unsigned char) *end) || ispunct((unsigned char) *end))) end--;

    end[1] = '\0';
}
