#include "common.h"

TEST_MAIN;

int tracee_main(pid_t parent) {
    ASSERT(ptrace(PTRACE_TRACEME, 0, 0, 0) == 0);
    ASSERT(raise(SIGURG) == 0);
    return 0;
}

int tracer_main(pid_t tracee) {
    int status;
    pid_t pid = waitpid(tracee, &status, 0);
    ASSERT(pid == tracee);
    ASSERT(WIFSTOPPED(status));
    ptrace_syscall_info info;
    info.op = 0xff;
    int r = syscall(SYS_ptrace, PTRACE_GET_SYSCALL_INFO, tracee, sizeof(info), &info);
    ASSERT(r == 24);
    ASSERT(info.op == PTRACE_SYSCALL_INFO_NONE);
    ASSERT(info.instruction_pointer != 0);
    ASSERT(info.stack_pointer != 0);
    ASSERT(ptrace(PTRACE_CONT, pid, 0, 0) == 0);
    ASSERT(wait_exit(tracee));
    return 0;
}

int execve_main() {
    ASSERT(false);
}
