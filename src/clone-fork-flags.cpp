#include "common.h"

TEST_MAIN;

int tracee_main(pid_t parent) {
    ASSERT(ptrace(PTRACE_TRACEME, 0, 0, 0) == 0);
    raise(SIGSTOP);
    long ret = syscall(SYS_clone, SIGCHLD, 0, 0, 0, 0);
    if (ret == 0) {
        _exit(0);
    }
    ASSERT(ret > 0);
    int status;
    ASSERT(waitpid(ret, &status, 0) == ret);
    ASSERT(WIFEXITED(status));
    ASSERT(WEXITSTATUS(status) == 0);
    return 0;
}

int tracer_main(pid_t tracee) {
    int status;
    ASSERT(waitpid(tracee, &status, 0) == tracee);
    ASSERT(WIFSTOPPED(status));
    ASSERT(WSTOPSIG(status) == SIGSTOP);
    ASSERT(ptrace(PTRACE_SETOPTIONS, tracee, 0, PTRACE_O_TRACECLONE | PTRACE_O_TRACEFORK) == 0);
    ASSERT(ptrace(PTRACE_CONT, tracee, 0, 0) == 0);
    ASSERT(waitpid(tracee, &status, 0) == tracee);
    ASSERT(WIFSTOPPED(status));
    ASSERT(WSTOPSIG(status) == SIGTRAP);
    ASSERT((status >> 8) == (SIGTRAP | (PTRACE_EVENT_FORK << 8)));
    u64 new_pid = 0;
    ASSERT(ptrace(PTRACE_GETEVENTMSG, tracee, 0, &new_pid) == 0);
    pid_t child_pid = (pid_t)new_pid;
    ASSERT(waitpid(child_pid, &status, 0) == child_pid);
    ASSERT(WIFSTOPPED(status));
    ASSERT(WSTOPSIG(status) == SIGSTOP);
    ASSERT(ptrace(PTRACE_CONT, child_pid, 0, 0) == 0);
    ASSERT(ptrace(PTRACE_CONT, tracee, 0, 0) == 0);
    ASSERT(wait_exit(child_pid));
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
