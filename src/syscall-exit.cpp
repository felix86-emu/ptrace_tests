#include "common.h"

TEST_MAIN;

int tracee_main(pid_t parent) {
    ASSERT(ptrace(PTRACE_TRACEME, 0, 0, 0) == 0);
    raise(SIGURG);
    close(0);
    return 0;
}

int tracer_main(pid_t tracee) {
    int status;
    pid_t pid = waitpid(tracee, &status, 0);
    ASSERT(WIFSTOPPED(status));
    ASSERT(WSTOPSIG(status) == SIGURG);
    ASSERT(ptrace(PTRACE_SETOPTIONS, tracee, 0, PTRACE_O_TRACESYSGOOD) == 0);
    ASSERT(ptrace(PTRACE_SYSCALL, tracee, 0, 0) == 0);
    pid = waitpid(tracee, &status, 0);
    ASSERT(WIFSTOPPED(status));
    ASSERT(WSTOPSIG(status) == (SIGTRAP | 0x80));
    ptrace_syscall_info info;
    int size = ptrace(PTRACE_GET_SYSCALL_INFO, tracee, sizeof(info), &info);
    ASSERT(size == 80);
    ASSERT(info.op == PTRACE_SYSCALL_INFO_ENTRY);
    ASSERT(info.entry.nr == SYS_close);
    ASSERT(info.entry.args[0] == 0);
    ASSERT(ptrace(PTRACE_SYSCALL, tracee, 0, 0) == 0);
    pid = waitpid(tracee, &status, 0);
    ASSERT(WIFSTOPPED(status));
    ASSERT(WSTOPSIG(status) == (SIGTRAP | 0x80));
    size = ptrace(PTRACE_GET_SYSCALL_INFO, tracee, sizeof(info), &info);
    ASSERT(size == 33);
    ASSERT(info.op == PTRACE_SYSCALL_INFO_EXIT);
    ASSERT(info.exit.is_error == false);
    ASSERT(info.exit.rval == 0);
    u64 data = 0;
#ifdef __x86_64__
    unsigned long offset = offsetof(user_regs_struct, rax);
#else
    unsigned long offset = offsetof(user_regs_struct, eax);
#endif
    ASSERT(syscall(SYS_ptrace, PTRACE_PEEKUSER, tracee, offset, &data) == 0);
    ASSERT(data == 0);
    ASSERT(ptrace(PTRACE_CONT, pid, 0, 0) == 0);
    ASSERT(wait_exit(tracee));
    return 0;
}

int execve_main() {
    ASSERT(false);
}
