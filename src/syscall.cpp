#include "common.h"

TEST_MAIN;

int tracee_main(pid_t parent) {
    ASSERT(ptrace(PTRACE_TRACEME, 0, 0, 0) == 0);
    ASSERT(raise(SIGURG) == 0);

    u16 buffer = 0;
    ASSERT(read(pipedes[0], &buffer, 2) == 2);
    ASSERT(buffer == 0xCAFE);
    return 0;
}

int tracer_main(pid_t tracee) {
    int status;
    int r = waitpid(tracee, &status, 0);
    ASSERT(WIFSTOPPED(status));
    ASSERT(WSTOPSIG(status) == SIGURG);
    ASSERT(ptrace(PTRACE_SYSCALL, tracee, 0, 0) == 0);

    r = waitpid(tracee, &status, 0);
    ASSERT(WIFSTOPPED(status));
    ASSERT(WSTOPSIG(status) == SIGTRAP);

    u16 buffer = 0xCAFE;
    ASSERT(write(pipedes[1], &buffer, 2) == 2);

    user_regs_struct regs;
    ASSERT(ptrace(PTRACE_GETREGS, tracee, 0, &regs) == 0);
#ifdef __x86_64__
    ASSERT(regs.orig_rax == SYS_read);
#else
    ASSERT(regs.orig_eax == SYS_read);
#endif

    ASSERT(ptrace(PTRACE_SYSCALL, tracee, 0, 0) == 0);
    r = waitpid(tracee, &status, 0);
    ASSERT(WIFSTOPPED(status));
    ASSERT(WSTOPSIG(status) == SIGTRAP);
    r = ptrace(PTRACE_GETREGS, tracee, 0, &regs);
#ifdef __x86_64__
    ASSERT(regs.rax == 2);
#else
    ASSERT(regs.eax == 2);
#endif
    ASSERT(ptrace(PTRACE_CONT, tracee, 0, 0) == 0);

    // Tracee should exit instead of being stopped by masked signal
    ASSERT(wait_exit(tracee));
    return 0;
}

int execve_main() {
    ASSERT(false);
}
