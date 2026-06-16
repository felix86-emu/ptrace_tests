#include "common.h"

TEST_MAIN;

bool ok = false;
void signal_handler(int sig, siginfo_t* info, void* ctx) {
    u64 mask;
    ASSERT(syscall(SYS_rt_sigprocmask, SIG_SETMASK, nullptr, &mask, sizeof(u64)) == 0);
    ASSERT((mask >> (sig - 1)) & 1); // blocked because no SA_NODEFER
    ok = true;
}

int tracee_main(pid_t parent) {
    struct sigaction sa;
    sa.sa_sigaction = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_SIGINFO;
    ASSERT(sigaction(SIGURG, &sa, nullptr) == 0);
    ASSERT(ptrace(PTRACE_TRACEME, 0, 0, 0) == 0);
    ASSERT(raise(SIGURG) == 0);
    u16 buffer = 0;
    ASSERT(read(pipedes[0], &buffer, 2) == 2);
    ASSERT(buffer == 0xCAFE);
    ASSERT(ok);
    return 0;
}

int tracer_main(pid_t tracee) {
    int status;
    int r = waitpid(tracee, &status, 0);
    ASSERT(WIFSTOPPED(status));
    ASSERT(WSTOPSIG(status) == SIGURG);
    u64 mask = -1ull & ~(1 << (SIGURG - 1)); // injected signal won't be injected if masked
    ASSERT(syscall(SYS_ptrace, PTRACE_SETSIGMASK, tracee, sizeof(u64), &mask) == 0);
    ASSERT(syscall(SYS_ptrace, PTRACE_CONT, tracee, 0, SIGURG) == 0);
    u16 buffer = 0xCAFE;
    ASSERT(write(pipedes[1], &buffer, 2) == 2);
    // Tracee should exit instead of being stopped by masked signal
    ASSERT(wait_exit(tracee));
    return 0;
}

int execve_main() {
    ASSERT(false);
}
