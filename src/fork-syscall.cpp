#include "common.h"

TEST_MAIN;

int tracee_main(pid_t parent) {
    ASSERT(ptrace(PTRACE_TRACEME, 0, 0, 0) == 0);
    raise(SIGSTOP);
    int child = syscall(SYS_fork);
    if (child == 0) {
        _exit(0);
    } else {
        int status;
        ASSERT(waitpid(child, &status, 0) == child);
        ASSERT(WIFEXITED(status));
        ASSERT(WEXITSTATUS(status) == 0);
        return 0;
    }
}

int tracer_main(pid_t tracee) {
    int status;
    ASSERT(waitpid(tracee, &status, 0) == tracee);
    ASSERT(WIFSTOPPED(status));
    ASSERT(WSTOPSIG(status) == SIGSTOP);

    ASSERT(ptrace(PTRACE_SETOPTIONS, tracee, 0, PTRACE_O_TRACEFORK | PTRACE_O_TRACECLONE | PTRACE_O_TRACESYSGOOD) == 0);

    ASSERT(ptrace(PTRACE_SYSCALL, tracee, 0, 0) == 0);

    ASSERT(waitpid(tracee, &status, 0) == tracee);
    ASSERT(WIFSTOPPED(status));
    ASSERT(WSTOPSIG(status) == (SIGTRAP | 0x80));
    ptrace_syscall_info info;
    ASSERT(ptrace(PTRACE_GET_SYSCALL_INFO, tracee, sizeof(info), &info) != -1);
    ASSERT(info.op == PTRACE_SYSCALL_INFO_ENTRY);
    ASSERT(info.entry.nr == SYS_fork);

    ASSERT(ptrace(PTRACE_SYSCALL, tracee, 0, 0) == 0);

    ASSERT(waitpid(tracee, &status, 0) == tracee);
    ASSERT(WIFSTOPPED(status));
    ASSERT(WSTOPSIG(status) == SIGTRAP);
    ASSERT((status >> 8) == (SIGTRAP | (PTRACE_EVENT_FORK << 8)));
    u64 new_pid = 0;
    ASSERT(ptrace(PTRACE_GETEVENTMSG, tracee, 0, &new_pid) == 0);

    ASSERT(waitpid(new_pid, &status, 0) == new_pid);
    ASSERT(WIFSTOPPED(status));
    ASSERT(WSTOPSIG(status) == SIGSTOP);
    ASSERT(ptrace(PTRACE_CONT, new_pid, 0, 0) == 0);

    ASSERT(ptrace(PTRACE_SYSCALL, tracee, 0, 0) == 0);

    ASSERT(waitpid(tracee, &status, 0) == tracee);
    ASSERT(WIFSTOPPED(status));
    ASSERT(WSTOPSIG(status) == (SIGTRAP | 0x80));
    ASSERT(ptrace(PTRACE_GET_SYSCALL_INFO, tracee, sizeof(info), &info) != -1);
    ASSERT(info.op == PTRACE_SYSCALL_INFO_EXIT);

    ASSERT(ptrace(PTRACE_CONT, tracee, 0, 0) == 0);
    ASSERT(wait_exit(new_pid));

    ASSERT(waitpid(tracee, &status, 0) == tracee);
    ASSERT(WIFSTOPPED(status));
    ASSERT(WSTOPSIG(status) == SIGCHLD);
    ASSERT(ptrace(PTRACE_CONT, tracee, 0, 0) == 0);
    ASSERT(wait_exit(tracee));
    return 0;
}

int execve_main() {
    ASSERT(false);
}
