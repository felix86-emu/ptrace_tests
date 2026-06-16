#include "common.h"

TEST_MAIN;

int tracee_main(pid_t parent) {
    u16 buffer = 0;
    ASSERT(read(pipedes[0], &buffer, 2) == 2);
    ASSERT(buffer == 0xCAFE);
    return 0;
}

int tracer_main(pid_t tracee) {
    ASSERT(ptrace(PTRACE_PEEKDATA, tracee, 0x10000, 0) == -1 && errno == ESRCH);

    u16 buffer = 0xCAFE;
    ASSERT(write(pipedes[1], &buffer, 2) == 2);
    ASSERT(wait_exit(tracee));
    return 0;
}

int execve_main() {
    ASSERT(false);
}
