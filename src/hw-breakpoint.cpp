#include "common.h"

TEST_MAIN;

bool ok = false;

__attribute__((noinline)) void my_func() {
    ok = true;
}

int tracee_main(pid_t parent) {
    ptrace(PTRACE_TRACEME, 0, 0, 0);
    raise(SIGSTOP);
    my_func();
    ASSERT(ok);
    return 0;
}

int tracer_main(pid_t tracee) {
    int status;
    int r = waitpid(tracee, &status, 0);
    ASSERT(WIFSTOPPED(status));
    ASSERT(WSTOPSIG(status) == SIGSTOP);
    // Unsigned long instead of u64 to match syscall variadic arg size in 32-bit
    unsigned long dr0_offset = offsetof(struct user, u_debugreg[0]);
    unsigned long dr_control_offset = offsetof(struct user, u_debugreg[7]);
    unsigned long new_control = 0b11; // enable breakpoint 0, type is instruction execution, length is byte
    unsigned long new_address = (u64)my_func;
    ASSERT(syscall(SYS_ptrace, PTRACE_POKEUSER, tracee, dr0_offset, new_address) == 0);
    ASSERT(syscall(SYS_ptrace, PTRACE_POKEUSER, tracee, dr_control_offset, new_control) == 0);
    ASSERT(ptrace(PTRACE_CONT, tracee, 0, 0) == 0);

    r = waitpid(tracee, &status, 0);
    ASSERT(WIFSTOPPED(status));
    ASSERT(WSTOPSIG(status) == SIGTRAP);
    siginfo_t guest_info;
    ASSERT(ptrace(PTRACE_GETSIGINFO, tracee, 0, &guest_info) == 0);
    ASSERT(guest_info.si_signo == SIGTRAP);
    ASSERT(guest_info.si_addr == (void*)&my_func);
    ASSERT(guest_info.si_code == TRAP_HWBKPT);
    ASSERT(ptrace(PTRACE_CONT, tracee, 0, 0) == 0);
    ASSERT(wait_exit(tracee));
    return 0;
}

int execve_main() {
    ASSERT(false);
}
