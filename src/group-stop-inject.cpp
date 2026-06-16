#include "common.h"

TEST_MAIN;

bool ok = false;

void signal_handler(int sig, siginfo_t* info, void* ctx) {
    ASSERT(sig == 53);
    ok = true;
}

int tracee_main(pid_t parent) {
    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = signal_handler;
    sigemptyset(&sa.sa_mask);
    ASSERT(sigaction(53, &sa, nullptr) == 0);
    ASSERT(ptrace(PTRACE_TRACEME, 0, 0, 0) == 0);
    ASSERT(raise(SIGSTOP) == 0);
    ASSERT(ok);

    u16 buffer = 0;
    ASSERT(read(pipedes[0], &buffer, 2) == 2);
    ASSERT(buffer == 0xCAFE);
    return 0;
}

int tracer_main(pid_t tracee) {
    int status;
    int r = waitpid(tracee, &status, 0);
    ASSERT(WIFSTOPPED(status));
    ASSERT(WSTOPSIG(status) == SIGSTOP);
    siginfo_t info;
    ASSERT(ptrace(PTRACE_GETSIGINFO, tracee, 0, &info) == 0);
    ASSERT(ptrace(PTRACE_CONT, tracee, 0, 53) == 0);

    u16 buffer = 0xCAFE;
    ASSERT(write(pipedes[1], &buffer, 2) == 2);
    // Tracee should exit instead of being stopped by masked signal
    ASSERT(wait_exit(tracee));
    return 0;
}

int execve_main() {
    ASSERT(false);
}
