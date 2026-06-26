#include "common.h"

TEST_MAIN;

int clone_main(void*) {
    return 0;
}

int tracee_main(pid_t parent) {
    int result = ptrace(PTRACE_TRACEME, 0, 0, 0);
    ASSERT(result == 0);
    raise(SIGTRAP);
    void* child_stack = mmap(nullptr, 4096, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK, -1, 0);
    ASSERT(child_stack != MAP_FAILED);
    int child = clone(clone_main, (void*)((uint8_t*)child_stack + 4096), 0, nullptr);
    ASSERT(child > 0);
    return 0;
}

int tracer_main(pid_t tracee) {
    int status;
    pid_t result = waitpid(tracee, &status, 0);
    ASSERT(result == tracee);
    ASSERT(WIFSTOPPED(status));
    ASSERT(WSTOPSIG(status) == SIGTRAP);
    int ret = ptrace(PTRACE_SETOPTIONS, tracee, 0, PTRACE_O_TRACECLONE);
    ASSERT(ret == 0);
    ret = ptrace(PTRACE_CONT, tracee, 0, 0);
    ASSERT(ret == 0);
    result = waitpid(tracee, &status, 0);
    ASSERT(result == tracee);
    ASSERT(WIFSTOPPED(status));
    ASSERT(WSTOPSIG(status) == SIGTRAP);
    ASSERT((status >> 8) == (SIGTRAP | (PTRACE_EVENT_CLONE << 8)));
    u64 new_pid = 0;
    ret = ptrace(PTRACE_GETEVENTMSG, tracee, 0, &new_pid);
    ASSERT(ret == 0);
    pid_t child_pid = (pid_t)new_pid;
    result = waitpid(child_pid, &status, 0);
    ASSERT(result == child_pid);
    ASSERT(WIFSTOPPED(status));
    ASSERT(WSTOPSIG(status) == SIGSTOP);
    siginfo_t info;
    ret = ptrace(PTRACE_GETSIGINFO, child_pid, 0, &info);
    ASSERT(ret == 0);
    ASSERT(info.si_signo == SIGSTOP);
    ASSERT(info.si_code == 0);
    ASSERT(info.si_status == 0);
    ret = ptrace(PTRACE_CONT, child_pid, 0, 0);
    ASSERT(ret == 0);
    ret = ptrace(PTRACE_CONT, tracee, 0, 0);
    ASSERT(ret == 0);
    ASSERT(wait_exit(child_pid));
    ASSERT(wait_exit(tracee));
    return 0;
}

int execve_main() {
    ASSERT(false);
}
