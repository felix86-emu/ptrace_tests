#include "common.h"

TEST_MAIN;

int tracee_main(pid_t parent) {
    // Unmap the second page so we guarantee there's nothing afterwards
    ASSERT(munmap((u8*)shared_memory + 4096, 4096) == 0);
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
    ASSERT(result == -1 && errno == EIO);
    ASSERT(ptrace(PTRACE_CONT, tracee, 0, 0) == 0);

    u16 buffer = 0xCAFE;
    ASSERT(write(pipedes[1], &buffer, 2) == 2);
    ASSERT(wait_exit(tracee));
    return 0;
}

int execve_main() {
    ASSERT(false);
}
