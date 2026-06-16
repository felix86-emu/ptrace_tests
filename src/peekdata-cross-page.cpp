#include "common.h"

TEST_MAIN;

constexpr u64 value = 0xABCDEF0123456789;

int tracee_main(pid_t parent) {
    u8* at_end = (u8*)shared_memory + 4094;
    memcpy(at_end, &value, 8);
    ASSERT(ptrace(PTRACE_TRACEME, 0, 0, 0) == 0);
    ASSERT(raise(SIGSTOP) == 0);

    u16 buffer = 0;
    ASSERT(read(pipedes[0], &buffer, 2) == 2);
    ASSERT(buffer == 0xCAFE);
    return 0;
}

int tracer_main(pid_t tracee) {
    u8* at_end = (u8*)shared_memory + 4094;
    int status;
    pid_t pid = waitpid(tracee, &status, 0);
    ASSERT(pid == tracee);
    ASSERT(WIFSTOPPED(status) == true);
    ASSERT(WSTOPSIG(status) == SIGSTOP);
    ASSERT((status >> 8) == SIGSTOP);
    u64 result = ptrace(PTRACE_PEEKDATA, tracee, at_end, nullptr);
#ifdef __x86_64__
    ASSERT(result == value);
#else
    ASSERT(result == (u32)value);
#endif
    ASSERT(ptrace(PTRACE_CONT, tracee, 0, 0) == 0);

    u16 buffer = 0xCAFE;
    ASSERT(write(pipedes[1], &buffer, 2) == 2);
    ASSERT(wait_exit(tracee));
    return 0;
}

int execve_main() {
    ASSERT(false);
}
