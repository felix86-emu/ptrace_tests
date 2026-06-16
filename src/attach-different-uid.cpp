#include "common.h"

TEST_MAIN;

int tracee_main(pid_t parent) {
    std::atomic_ref<int> atomic(*shared_int1);
    if (geteuid() != 0) {
        return 0;
    }
    // Change UID to make us not traceable
    ASSERT(setresuid(65533, 65533, 65533) == 0);
    atomic.fetch_add(1); // allow parent to attach
    u16 buffer = 0;
    ASSERT(read(pipedes[0], &buffer, 2) == 2);
    ASSERT(buffer == 0xCAFE);
    return 0;
}

int tracer_main(pid_t tracee) {
    std::atomic_ref<int> atomic(*shared_int1);
    if (geteuid() != 0) {
        printf("This test needs root, skipping\n");
        atomic = 0x42;
        return 0;
    }

    // Change to some other user
    ASSERT(setresuid(65534, 65534, 65534) == 0);

    while (atomic == 0)
        ;
    ASSERT(ptrace(PTRACE_ATTACH, tracee, 0, 0) == -1 && errno == EPERM);

    u16 buffer = 0xCAFE;
    ASSERT(write(pipedes[1], &buffer, 2) == 2);
    ASSERT(wait_exit(tracee));
    return 0;
}

int execve_main() {
    ASSERT(false);
}
