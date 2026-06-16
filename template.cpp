#include "common.h"

TEST_MAIN;

int tracee_main(pid_t parent) {
    return 0;
}

int tracer_main(pid_t tracee) {
    return 0;
}

int execve_main() {
    ASSERT(false);
}
