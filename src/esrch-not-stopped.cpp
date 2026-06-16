#include "common.h"

TEST_MAIN;

int tracee_main(pid_t parent) {
    std::atomic_ref<int> atomic(*shared_int1);
    ASSERT(ptrace(PTRACE_TRACEME, 0, 0, 0) == 0);
    atomic.fetch_add(1);
    u16 buffer = 0;
    ASSERT(read(pipedes[0], &buffer, 2) == 2);
    ASSERT(buffer == 0xCAFE);
    return 0;
}

int tracer_main(pid_t tracee) {
    std::atomic_ref<int> atomic(*shared_int1);
    while (atomic != 1)
        ;
    ASSERT(ptrace(PTRACE_PEEKDATA, tracee, 0x10000, 0) == -1 && errno == ESRCH);
    u16 buffer = 0xCAFE;
    ASSERT(write(pipedes[1], &buffer, 2) == 2);
    ASSERT(wait_exit(tracee));
    return 0;
}

int execve_main() {
    ASSERT(false);
}
