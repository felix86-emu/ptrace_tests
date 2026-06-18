#include "common.h"

TEST_MAIN;

int fork_main() {
    // Will be stopped on fork()
    std::atomic_ref<int> atomic(*shared_int1);
    ASSERT(atomic == getpid());
    return 0;
}

int tracee_main(pid_t parent) {
    int result = ptrace(PTRACE_TRACEME, 0, 0, 0);
    ASSERT(result == 0);
    raise(SIGTRAP);
    int child = fork();
    if (child == 0) {
        return fork_main();
    } else {
        int status;
        result = waitpid(child, &status, 0);
        ASSERT(result == child);
        ASSERT(WIFEXITED(status));
        ASSERT(WEXITSTATUS(status) == 0);
        return 0;
    }
}

int tracer_main(pid_t tracee) {
    std::atomic_ref<int> atomic(*shared_int1);
    int status;
    int result = waitpid(tracee, &status, 0);
    ASSERT(result == tracee);
    ASSERT(WIFSTOPPED(status));
    ASSERT(WSTOPSIG(status) == SIGTRAP);
    result = ptrace(PTRACE_SETOPTIONS, tracee, 0, PTRACE_O_TRACEFORK);
    ASSERT(result == 0);
    result = ptrace(PTRACE_CONT, tracee, 0, 0);
    ASSERT(result == 0);
    result = waitpid(tracee, &status, 0);
    ASSERT(result == tracee);
    ASSERT(WIFSTOPPED(status));
    ASSERT(WSTOPSIG(status) == SIGTRAP);
    ASSERT((status >> 8) == (SIGTRAP | (PTRACE_EVENT_FORK << 8)));
    u64 new_pid = 0;
    result = ptrace(PTRACE_GETEVENTMSG, tracee, 0, &new_pid);
    ASSERT(result == 0);
    atomic = new_pid;
    result = waitpid(new_pid, &status, 0);
    ASSERT(result == new_pid);
    ASSERT(WIFSTOPPED(status));
    ASSERT(WSTOPSIG(status) == SIGSTOP);
    result = ptrace(PTRACE_CONT, new_pid, 0, 0);
    ASSERT(result == 0);
    result = ptrace(PTRACE_CONT, tracee, 0, 0);
    ASSERT(result == 0);
    ASSERT(wait_exit(new_pid));
    result = waitpid(tracee, &status, 0);
    ASSERT(WIFSTOPPED(status));
    ASSERT(WSTOPSIG(status) == SIGCHLD);
    result = ptrace(PTRACE_CONT, tracee, 0, 0);
    ASSERT(wait_exit(tracee));
    return 0;
}

int execve_main() {
    ASSERT(false);
}
