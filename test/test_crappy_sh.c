#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdbool.h>
#include "../lib/shell.c"


/* A test case that does nothing and succeeds. */
static void null_test_success(void **state) {
    char command0[18];
    sprintf(command0, "-c %s /bin/ls ; %s", "\"", "\"");
    printf("command0 = \"%s\"\n", command0);
    processInput(command0, true);
}

static void null_test_failure(void **state) {
    assert_false(true || state);
}


int main(void) {
    const struct CMUnitTest tests[] = {
            cmocka_unit_test(null_test_success),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
