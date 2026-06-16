#include "common.h"

TEST_MAIN;

int tracee_main(pid_t parent) {
    ASSERT(ptrace(PTRACE_TRACEME, 0, 0, 0) == 0);
    volatile int* memory = nullptr;
    *memory = 0;
    return 0;
}

int tracer_main(pid_t tracee) {
    int status;
    pid_t pid = waitpid(tracee, &status, 0);
    ASSERT(pid == tracee);
    ASSERT(WIFSTOPPED(status));
    ASSERT(WSTOPSIG(status) == SIGSEGV);
    ASSERT(ptrace(PTRACE_CONT, tracee, 0, SIGSEGV) == 0);
    pid = waitpid(tracee, &status, 0);
    ASSERT(WIFSIGNALED(status));
    ASSERT(WTERMSIG(status) == SIGSEGV);
    return 0;
}

int execve_main() {
    ASSERT(false);
}
