#include <sys/syscall.h>
#include "common.h"

TEST_MAIN;

bool signaled = false;
void signal_handler(int sig, siginfo_t* info, void* ctx) {
    signaled = true;
}

int tracee_main(pid_t parent) {
    struct sigaction sa;
    sa.sa_sigaction = signal_handler;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);
    ASSERT(sigaction(SIGURG, &sa, nullptr) == 0);
    ASSERT(ptrace(PTRACE_TRACEME, 0, 0, 0) == 0);
    ASSERT(raise(SIGURG) == 0);
    ASSERT(signaled);
    return 0;
}

int tracer_main(pid_t tracee) {
    int status;
    pid_t pid = waitpid(tracee, &status, 0);
    ASSERT(pid == tracee);
    ASSERT(WIFSTOPPED(status));
    ASSERT(WSTOPSIG(status) == SIGURG);
    int result = ptrace(PTRACE_SINGLESTEP, tracee, 0, SIGURG);
    pid = waitpid(tracee, &status, 0);
    ASSERT(WIFSTOPPED(status));
    ASSERT(WSTOPSIG(status) == SIGTRAP);
#ifdef __x86_64__
    u64 rip;
    int offset = 128;
#else
    u32 rip;
    int offset = 48;
#endif
    result = syscall(SYS_ptrace, PTRACE_PEEKUSER, tracee, offset, &rip);
    ASSERT((void*)rip == &signal_handler);
    siginfo_t remote_info;
    result = ptrace(PTRACE_GETSIGINFO, tracee, 0, &remote_info);
    // ASSERT(remote_info.si_code == TRAP_UNK);
    result = ptrace(PTRACE_CONT, tracee, 0, 0);
    ASSERT(result == 0);
    ASSERT(wait_exit(tracee));
    return 0;
}

int execve_main() {
    ASSERT(false);
}
