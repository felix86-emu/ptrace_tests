#include "common.h"

TEST_MAIN;

int tracee_main(pid_t parent) {
    ASSERT(ptrace(PTRACE_TRACEME, 0, 0, 0) == 0);
    ASSERT(raise(SIGSTOP) == 0);
    return 0;
}

int tracer_main(pid_t tracee) {
    int status;
    pid_t pid = waitpid(tracee, &status, 0);
    ASSERT(pid == tracee);
    ASSERT(WIFSTOPPED(status));
    ASSERT(WSTOPSIG(status) == SIGSTOP);
    int result = ptrace(PTRACE_SINGLESTEP, tracee, 0, 0);
    pid = waitpid(tracee, &status, 0);
    ASSERT(WIFSTOPPED(status));
    ASSERT(WSTOPSIG(status) == SIGTRAP);
    siginfo_t remote_info;
    result = ptrace(PTRACE_GETSIGINFO, tracee, 0, &remote_info);
    ASSERT(remote_info.si_code == TRAP_TRACE);
    result = ptrace(PTRACE_CONT, tracee, 0, 0);
    ASSERT(result == 0);
    ASSERT(wait_exit(tracee));
    return 0;
}

int execve_main() {
    ASSERT(false);
}
