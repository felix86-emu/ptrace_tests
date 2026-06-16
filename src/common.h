#include <atomic>
#include <cstring>
#include <string>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/ptrace.h>
#include <sys/ucontext.h>
#include <sys/wait.h>
#include <unistd.h>

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

static int pipedes[2];
static int memfd;
static void* shared_memory = nullptr;
static int* shared_int1 = nullptr;
static int* shared_int2 = nullptr;
static u64* shared_u64 = nullptr;

#define ASSERT(cond)                                                                                                                                 \
    do {                                                                                                                                             \
        if (!(cond)) {                                                                                                                               \
            printf("Condition %s failed!\n", #cond);                                                                                                 \
            exit(1);                                                                                                                                 \
        }                                                                                                                                            \
    } while (0)

#define TEST_MAIN                                                                                                                                    \
    int tracee_main(pid_t pid);                                                                                                                      \
    int tracer_main(pid_t pid);                                                                                                                      \
    int execve_main();                                                                                                                               \
    int main() {                                                                                                                                     \
        if (getenv("__EXECVE_PROCESS")) {                                                                                                            \
            const char* pipe_0 = getenv("__PIPE_0");                                                                                                 \
            const char* pipe_1 = getenv("__PIPE_1");                                                                                                 \
            const char* smemfd = getenv("__MEMFD");                                                                                                  \
            ASSERT(pipe_0);                                                                                                                          \
            ASSERT(pipe_1);                                                                                                                          \
            ASSERT(smemfd);                                                                                                                          \
            pipedes[0] = std::atoi(pipe_0);                                                                                                          \
            pipedes[1] = std::atoi(pipe_1);                                                                                                          \
            memfd = std::atoi(smemfd);                                                                                                               \
            ASSERT(pipedes[0] > 2);                                                                                                                  \
            ASSERT(pipedes[1] > 2);                                                                                                                  \
            ASSERT(memfd > 2);                                                                                                                       \
            shared_memory = mmap(nullptr, 4096 * 2, PROT_READ | PROT_WRITE, MAP_SHARED, memfd, 0);                                                   \
            shared_int1 = (int*)shared_memory;                                                                                                       \
            shared_int2 = (int*)((uint8_t*)shared_memory + 4);                                                                                       \
            shared_u64 = (u64*)((uint8_t*)shared_memory + 8);                                                                                        \
            return execve_main();                                                                                                                    \
        }                                                                                                                                            \
        memfd = memfd_create("shared", 0);                                                                                                           \
        ASSERT(memfd > 2);                                                                                                                           \
        ASSERT(ftruncate(memfd, 4096 * 2) == 0);                                                                                                     \
        shared_memory = mmap(nullptr, 4096 * 2, PROT_READ | PROT_WRITE, MAP_SHARED, memfd, 0);                                                       \
        shared_int1 = (int*)shared_memory;                                                                                                           \
        shared_int2 = (int*)((uint8_t*)shared_memory + 4);                                                                                           \
        shared_u64 = (u64*)((uint8_t*)shared_memory + 8);                                                                                            \
        int ret = pipe(pipedes);                                                                                                                     \
        ASSERT(ret == 0);                                                                                                                            \
        ASSERT(pipedes[0] > 2);                                                                                                                      \
        ASSERT(pipedes[1] > 2);                                                                                                                      \
        pid_t pid = getpid();                                                                                                                        \
        pid_t child = fork();                                                                                                                        \
        if (child == 0) {                                                                                                                            \
            return tracee_main(pid);                                                                                                                 \
        } else {                                                                                                                                     \
            return tracer_main(child);                                                                                                               \
        }                                                                                                                                            \
    }

inline void execve_self() {
    std::string pipe_0 = std::to_string(pipedes[0]);
    std::string pipe_1 = std::to_string(pipedes[1]);
    std::string smemfd = std::to_string(memfd);
    ASSERT(setenv("__EXECVE_PROCESS", "1", 0) == 0);
    ASSERT(setenv("__PIPE_0", pipe_0.c_str(), 1) == 0);
    ASSERT(setenv("__PIPE_1", pipe_1.c_str(), 1) == 0);
    ASSERT(setenv("__MEMFD", smemfd.c_str(), 1) == 0);
    char* const args[] = {nullptr};
    execve("/proc/self/exe", args, environ);
    ASSERT(false);
}

inline bool wait_exit(pid_t child) {
    int status;
    pid_t pid = waitpid(child, &status, 0);
    return pid == child && WIFEXITED(status) && WEXITSTATUS(status) == 0;
}

inline bool wait_terminate(pid_t child, int sig) {
    int status;
    pid_t pid = waitpid(child, &status, 0);
    return pid == child && WIFSIGNALED(status) && WTERMSIG(status) == sig;
}
