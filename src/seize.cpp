#include "common.h"

TEST_MAIN;

int tracee_main(pid_t parent) {
    u16 buffer = 0;
    ASSERT(read(pipedes[0], &buffer, 2) == 2);
    ASSERT(buffer == 0xCAFE);
    return 0;
}

int tracer_main(pid_t tracee) {
    int status;
    ASSERT(ptrace(PTRACE_SEIZE, tracee, 0, 0) == 0);
    usleep(100000);
    // seize by itself doesn't stop the tracee
    pid_t pid = waitpid(tracee, &status, WNOHANG);
    ASSERT(pid == 0);
    ASSERT(ptrace(PTRACE_INTERRUPT, tracee, 0, 0) == 0);
    pid = waitpid(tracee, &status, 0);
    ASSERT(pid == tracee);
    ASSERT(WIFSTOPPED(status));
    ASSERT(WSTOPSIG(status) == SIGTRAP);
    ASSERT(status >> 8 == (SIGTRAP | (PTRACE_EVENT_STOP << 8)));
    ASSERT(ptrace(PTRACE_CONT, tracee, 0, 0) == 0);
    u16 buffer = 0xCAFE;
    ASSERT(write(pipedes[1], &buffer, 2) == 2);
    ASSERT(wait_exit(tracee));
    return 0;
}

int execve_main() {
    ASSERT(false);
}
