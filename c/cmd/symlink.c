#include <unistd.h>
#include <stdlib.h>

int symlink_main(int argc, char **argv) {
    if (argc < 3 || argv[1] == NULL || argv[2] == NULL) {
        printf("Usage %s <source> <destination>\n", argv[0]);
        exit(2);
    }
    printf("symlink %s %s\n", argv[1], argv[2]);
    symlink(argv[1], argv[2]);
    return EXIT_SUCCESS;
}
