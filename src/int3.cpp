#include <sys/syscall.h>
#include "common.h"

TEST_MAIN;

void __attribute__((naked)) int3() {
    asm(R"(
        int3
        ret
    )");
}

int tracee_main(pid_t parent) {
    int result = ptrace(PTRACE_TRACEME, 0, 0, 0);
    int3();
    return 0;
}

int tracer_main(pid_t tracee) {
    int status;
    int result = waitpid(tracee, &status, 0);
    ASSERT(result == tracee);
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
    ASSERT(result == 0);
    ASSERT(rip == (u64)int3 + 1);
    result = ptrace(PTRACE_CONT, tracee, 0, 0);
    ASSERT(wait_exit(tracee));
    return 0;
}

int execve_main() {
    ASSERT(false);
}
