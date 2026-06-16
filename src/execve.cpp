#include "common.h"

TEST_MAIN;

int tracee_main(pid_t parent) {
    ASSERT(ptrace(PTRACE_TRACEME, 0, 0, 0) == 0);
    execve_self();
    return 1;
}

int tracer_main(pid_t tracee) {
    int status;
    int r = waitpid(tracee, &status, 0);
    ASSERT(WIFSTOPPED(status));
    ASSERT(WSTOPSIG(status) == SIGTRAP);
    ASSERT(ptrace(PTRACE_CONT, tracee, 0, 0) == 0);
    r = waitpid(tracee, &status, 0);
    ASSERT(WIFEXITED(status));
    ASSERT(WEXITSTATUS(status) == 42);
    return 0;
}

int execve_main() {
    return 42;
}
