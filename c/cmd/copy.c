#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>

#ifdef __APPLE__

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>

#endif

#ifdef __linux__
#include <sys/sendfile.h>
#endif

int cp(const char *, const char *);

int main(int argc, char **argv) {
    if (argv[1] == NULL || argv[2] == NULL) {
        printf("Usage %s <source> <destination>\n", argv[0]);
        exit(2);
    }
    printf("copy %s %s\n", argv[1], argv[2]);
    cp(argv[1], argv[2]);
}

int cp(const char *source, const char *dest) {
#ifdef __linux__
    // https://stackoverflow.com/a/2180204
    int fdSource = open(source, O_RDWR);

    /* Caf's comment about race condition... */
    if (fdSource > 0) {
        if (lockf(fdSource, F_LOCK, 0) == -1) return 0; /* FAILURE */
    } else return 0; /* FAILURE */

    /* Now the fdSource is locked */

    int fdDest = open(dest, O_CREAT);
    off_t lCount;
    struct stat sourceStat;
    if (fdSource > 0 && fdDest > 0) {
        if (!stat(source, &sourceStat)) {
            int len = sendfile(fdDest, fdSource, &lCount, sourceStat.st_size);
            if (len > 0 && len == sourceStat.st_size) {
                close(fdDest);
                close(fdSource);

                /* Sanity Check for Lock, if this is locked -1 is returned! */
                if (lockf(fdSource, F_TEST, 0) == 0) {
                    if (lockf(fdSource, F_ULOCK, 0) == -1) {
                        /* WHOOPS! WTF! FAILURE TO UNLOCK! */
                    } else {
                        return 1; /* Success */
                    }
                } else {
                    /* WHOOPS! WTF! TEST LOCK IS -1 WTF! */
                    return 0; /* FAILURE */
                }
            }
        }
    }
    return 0; /* Failure */
#else
    // https://stackoverflow.com/a/2180157
    int fd_to, fd_from;
    char buf[4096];
    ssize_t nread;
    int saved_errno;

    fd_from = open(source, O_RDONLY);
    if (fd_from < 0)
        return -1;

    fd_to = open(dest, O_WRONLY | O_CREAT | O_EXCL, 0666);
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
#endif
}
