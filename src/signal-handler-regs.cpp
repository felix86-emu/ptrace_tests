#include "common.h"

TEST_MAIN;

#ifndef __x86_64__
#define REG_RAX REG_EAX
#endif
void signal_handler(int sig, siginfo_t* info, void* ctx) {
    u64 rax = ((ucontext_t*)ctx)->uc_mcontext.gregs[REG_RAX];
    ASSERT(rax == 0xdeadbeef);
    ((ucontext_t*)ctx)->uc_mcontext.gregs[REG_RAX] = 0;
}

int tracee_main(pid_t parent) {
    struct sigaction sa;
    sa.sa_sigaction = (decltype(sa.sa_sigaction))signal_handler;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);
    ASSERT(sigaction(SIGTRAP, &sa, nullptr) == 0);
    ASSERT(ptrace(PTRACE_TRACEME, 0, 0, 0) == 0);
    raise(SIGTRAP);
    return 0;
}

int tracer_main(pid_t tracee) {
    int status;
    int r = waitpid(tracee, &status, 0);
    ASSERT(r == tracee);
    ASSERT(WIFSTOPPED(status));
    ASSERT(WSTOPSIG(status) == SIGTRAP);
#ifdef __x86_64__
    ASSERT(ptrace(PTRACE_POKEUSER, tracee, 80, 0xdeadbeef) == 0);
#else
    ASSERT(ptrace(PTRACE_POKEUSER, tracee, 0, 0xdeadbeef) == 0);
#endif
    ASSERT(ptrace(PTRACE_CONT, tracee, 0, 0) == 0);
    ASSERT(wait_exit(tracee));
    return 0;
}

int execve_main() {
    ASSERT(false);
}
