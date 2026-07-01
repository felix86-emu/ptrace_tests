#include "common.h"

TEST_MAIN;

bool signal_happened = false;

void signal_handler(int sig, siginfo_t* info, void* ctx) {
    ASSERT(sig == SIGUSR2);
    ASSERT(info->si_value.sival_int == 0x12345678);
    ASSERT(info->si_code == -10);
    signal_happened = true;
}

int tracee_main(pid_t parent) {
    struct sigaction sa;
    sa.sa_sigaction = signal_handler;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);
    ASSERT(sigaction(SIGUSR1, &sa, nullptr) == 0);
    ASSERT(sigaction(SIGUSR2, &sa, nullptr) == 0);
    ASSERT(ptrace(PTRACE_TRACEME, 0, 0, 0) == 0);
    sigval_t val;
    val.sival_int = 0xDEADBEEF;
    ASSERT(sigqueue(getpid(), SIGUSR1, val) == 0);
    ASSERT(signal_happened);
    return 0;
}

int tracer_main(pid_t tracee) {
    int status;
    pid_t pid = waitpid(tracee, &status, 0);
    ASSERT(pid == tracee);
    ASSERT(WIFSTOPPED(status) == true);
    ASSERT(WSTOPSIG(status) == SIGUSR1);
    ASSERT((status >> 8) == SIGUSR1);
    siginfo_t info;
    memset(&info, 0, sizeof(info));
    info.si_code = -10;
    info.si_signo = SIGUSR2;
    info.si_value.sival_int = 0x12345678;
    ASSERT(ptrace(PTRACE_SETSIGINFO, tracee, 0, &info) == 0);
    // Change the signal to a SIGUSR2
    ASSERT(ptrace(PTRACE_CONT, tracee, 0, SIGUSR2) == 0);
    ASSERT(wait_exit(tracee));
    return 0;
}

int execve_main() {
    ASSERT(false);
}
