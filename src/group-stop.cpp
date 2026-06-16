#include "common.h"

TEST_MAIN;

void* pthread_main(void*) {
    std::atomic_ref<int> atomic(*shared_int1);
    ASSERT(ptrace(PTRACE_TRACEME, 0, 0, 0) == 0);
    atomic = 1;
    while (atomic == 1)
        ;
    return nullptr;
}

int tracee_main(pid_t parent) {
    std::atomic_ref<int> atomic(*shared_int1);
    atomic = 0;
    ASSERT(ptrace(PTRACE_TRACEME, 0, 0, 0) == 0);
    pthread_t thread;
    pthread_create(&thread, nullptr, pthread_main, nullptr);
    while (atomic != 1)
        ;
    raise(SIGSTOP);
    pthread_join(thread, nullptr);

    u16 buffer = 0;
    ASSERT(read(pipedes[0], &buffer, 2) == 2);
    ASSERT(buffer == 0xCAFE);
    return 0;
}

int tracer_main(pid_t tracee) {
    std::atomic_ref<int> atomic(*shared_int1);
    int status;
    int r = waitpid(tracee, &status, 0);
    // signal-delivery-stop
    ASSERT(WIFSTOPPED(status));
    ASSERT(WSTOPSIG(status) == SIGSTOP);
    siginfo_t info;
    ASSERT(ptrace(PTRACE_GETSIGINFO, tracee, 0, &info) == 0);
    ASSERT(ptrace(PTRACE_CONT, tracee, 0, SIGSTOP) == 0);
    r = waitpid(tracee, &status, 0);
    // group-stop
    ASSERT(WIFSTOPPED(status));
    ASSERT(WSTOPSIG(status) == SIGSTOP);
    ASSERT(ptrace(PTRACE_GETSIGINFO, tracee, 0, &info) == -1 && errno == EINVAL);
    ASSERT(ptrace(PTRACE_CONT, tracee, 0, 0) == 0);

    r = waitpid(-1, &status, 0);
    // thread group-stop
    ASSERT(WIFSTOPPED(status));
    ASSERT(WSTOPSIG(status) == SIGSTOP);
    ASSERT(ptrace(PTRACE_GETSIGINFO, r, 0, &info) == -1 && errno == EINVAL);
    atomic = 2;
    ASSERT(ptrace(PTRACE_CONT, r, 0, 0) == 0);

    r = waitpid(r, &status, 0);
    ASSERT(WIFEXITED(status));
    ASSERT(WEXITSTATUS(status) == 0);

    u16 buffer = 0xCAFE;
    ASSERT(write(pipedes[1], &buffer, 2) == 2);
    // Tracee should exit instead of being stopped by masked signal
    ASSERT(wait_exit(tracee));
    return 0;
}

int execve_main() {
    ASSERT(false);
}
