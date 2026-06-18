#include "common.h"

TEST_MAIN;

constexpr u64 initial_value = 0xABCDEF0123456789;
constexpr u64 new_value = 0xDEADBEEFCAFE1234;

int tracee_main(pid_t parent) {
    *shared_u64 = initial_value;
    ASSERT(ptrace(PTRACE_TRACEME, 0, 0, 0) == 0);
    ASSERT(raise(SIGSTOP) == 0);

#ifdef __x86_64__
    ASSERT(*shared_u64 == new_value);
#else
    ASSERT((u32)*shared_u64 == (u32)new_value);
#endif

    u16 buffer = 0;
    ASSERT(read(pipedes[0], &buffer, 2) == 2);
    ASSERT(buffer == 0xCAFE);
    return 0;
}

int tracer_main(pid_t tracee) {
    int status;
    pid_t pid = waitpid(tracee, &status, 0);
    ASSERT(pid == tracee);
    ASSERT(WIFSTOPPED(status) == true);
    ASSERT(WSTOPSIG(status) == SIGSTOP);
    ASSERT((status >> 8) == SIGSTOP);
    u64 result;
    ASSERT(syscall(SYS_ptrace, PTRACE_PEEKDATA, tracee, shared_u64, &result) == 0);
#ifdef __x86_64__
    ASSERT(result == initial_value);
#else
    ASSERT((u32)result == (u32)initial_value);
#endif
    ASSERT(syscall(SYS_ptrace, PTRACE_POKEDATA, tracee, shared_u64, new_value) == 0);
    ASSERT(syscall(SYS_ptrace, PTRACE_PEEKDATA, tracee, shared_u64, &result) == 0);
#ifdef __x86_64__
    ASSERT(result == new_value);
#else
    ASSERT((u32)result == (u32)new_value);
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
