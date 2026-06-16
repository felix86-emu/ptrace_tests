#include "common.h"

TEST_MAIN;

int tracee_main(pid_t parent) {
    ASSERT(ptrace(PTRACE_TRACEME, 0, 0, 0) == 0);
    raise(SIGURG);
    return 0;
}

int tracer_main(pid_t tracee) {
    int status;
    pid_t pid = waitpid(tracee, &status, 0);
    ASSERT(ptrace(PTRACE_CONT, tracee, 0, -5) == -1); // negative signal
    ASSERT(errno == EIO);
    ASSERT(ptrace(PTRACE_CONT, tracee, 0, 0) == 0);
    pid = waitpid(tracee, &status, 0);
    ASSERT(WIFEXITED(status));
    ASSERT(WEXITSTATUS(status) == 0);
    return 0;
}

int execve_main() {
    ASSERT(false);
}
