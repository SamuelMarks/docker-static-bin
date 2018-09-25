#include <stdio.h>
#include "list.h"
#include "../lib/tinydir.h"

int list_main(int argc, char **argv) {
    if (argv[1] == NULL) {
      printf("Usage %s <location>\n", argv[0]);
      exit(2);
    }
    list_dir(argv[1]);

    return EXIT_SUCCESS;
}

void list_dir(const char *path) {
    tinydir_dir dir;
    tinydir_open(&dir, path);

    while (dir.has_next) {
        tinydir_file file;
        tinydir_readfile(&dir, &file);

        printf("%s", file.name);
        if (file.is_dir)
            printf("/");
        printf("\n");

        tinydir_next(&dir);
    }

    tinydir_close(&dir);
}
