#include "common.h"

TEST_MAIN;

int tracee_main(pid_t parent) {
    sigset_t set;
    sigfillset(&set);
    ASSERT(sigprocmask(SIG_BLOCK, &set, nullptr) == 0);
    ASSERT(ptrace(PTRACE_TRACEME, 0, 0, 0) == 0);
    // SIGURG is masked, so will not stop here
    ASSERT(raise(SIGURG) == 0);

    u16 buffer = 0;
    ASSERT(read(pipedes[0], &buffer, 2) == 2);
    ASSERT(buffer == 0xCAFE);
    return 0;
}

int tracer_main(pid_t tracee) {
    u16 buffer = 0xCAFE;
    ASSERT(write(pipedes[1], &buffer, 2) == 2);
    // Tracee should exit instead of being stopped by masked signal
    ASSERT(wait_exit(tracee));
    return 0;
}

int execve_main() {
    ASSERT(false);
}
