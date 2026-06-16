#include "common.h"

TEST_MAIN;

int tracee_main(pid_t parent) {
    std::atomic_ref<int> atomic(*shared_int1);
    ASSERT(ptrace(PTRACE_TRACEME, 0, 0, 0) == 0);
    atomic.fetch_add(1);
    // SIGURG is ignored by default, but it should still enter signal-delivery-stop
    ASSERT(raise(SIGURG) == 0);
    atomic.fetch_add(1);

    u16 buffer = 0;
    ASSERT(read(pipedes[0], &buffer, 2) == 2);
    ASSERT(buffer == 0xCAFE);
    return 0;
}

int tracer_main(pid_t tracee) {
    std::atomic_ref<int> atomic(*shared_int1);
    int status;
    pid_t pid = waitpid(tracee, &status, 0);
    ASSERT(atomic == 1); // Ensure it didn't move past the SIGURG
    ASSERT(pid == tracee);
    ASSERT(WIFSTOPPED(status) == true);
    ASSERT(WSTOPSIG(status) == SIGURG);
    ASSERT((status >> 8) == SIGURG);
    ASSERT(ptrace(PTRACE_CONT, tracee, 0, SIGURG) == 0);

    u16 buffer = 0xCAFE;
    ASSERT(write(pipedes[1], &buffer, 2) == 2);
    ASSERT(wait_exit(tracee));
    ASSERT(atomic == 2);
    return 0;
}

int execve_main() {
    ASSERT(false);
}
