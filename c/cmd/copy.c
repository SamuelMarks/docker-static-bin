#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "copy.h"

int copy_main(int argc, char **argv) {
    if (argc < 3 || argv[1] == NULL || argv[2] == NULL) {
        printf("Usage %s <source> <destination> [mode=0666]\n", argv[0]);
        exit(2);
    }
    printf("copy %s %s\n", argv[1], argv[2]);
    cp(argv[1], argv[2], argc > 3 && argv[3] != NULL ? (int) strtol(argv[3], NULL, 10) : 0666);

    return EXIT_SUCCESS;
}

int cp(const char *source, const char *dest, int mode) {
    // https://stackoverflow.com/a/2180157
    int fd_to, fd_from;
    char buf[4096];
    ssize_t nread;
    int saved_errno;

    fd_from = open(source, O_RDONLY);
    if (fd_from < 0)
        return -1;

    fd_to = open(dest, O_WRONLY | O_CREAT | O_EXCL, mode);
    if (fd_to < 0)
        goto out_error;

    while (nread = read(fd_from, buf, sizeof buf), nread > 0) {
        char *out_ptr = buf;
        ssize_t nwritten;

        do {
            nwritten = write(fd_to, out_ptr, nread);

            if (nwritten >= 0) {
                nread -= nwritten;
                out_ptr += nwritten;
            } else if (errno != EINTR) {
                goto out_error;
            }
        } while (nread > 0);
    }

    if (nread == 0) {
        if (close(fd_to) < 0) {
            fd_to = -1;
            goto out_error;
        }
        close(fd_from);

        /* Success! */
        return 0;
    }

    out_error:
    saved_errno = errno;

    close(fd_from);
    if (fd_to >= 0)
        close(fd_to);

    errno = saved_errno;
    return -1;
}
