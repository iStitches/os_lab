#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string>
#include "../include/process.h"
#include "../include/err.h"

Process::Process() : pid_(0){}

Process::~Process() {}

int Process::exec(const std::string &filename,
    const char *const argv[],
    const char *const envp[],
    int timeout_secs)
{
    if (pid_) {
        printf("Process::exec already forked");
        return -1;
    }

    int output_pipe_fds[2];
    int pipe_fds[2];
    if(pipe(output_pipe_fds) || pipe(pipe_fds)) {
        err_sys("Process::exec pipe failed");
    }

    //set child pipe cloexec
    if (fcntl(pipe_fds[0], F_SETFD, fcntl(pipe_fds[0], F_GETFD) | FD_CLOEXEC)) {
        err_sys("Process:exec fcntl failed");
    }
    if (fcntl(pipe_fds[1], F_SETFD, fcntl(pipe_fds[1], F_GETFD) | FD_CLOEXEC)) {
        err_sys("Process:exec fcntl failed");
    }

    pid_ = fork();

    if (pid_ < 0) {
        close(output_pipe_fds[0]);
        close(output_pipe_fds[1]);
        close(pipe_fds[0]);
        close(pipe_fds[1]);
        err_sys("Process:exec fork failed");
    } else if (pid_ == 0) {   //子进程执行命令,并将异常通过pipe_fds 发送给父进程
        close(pipe_fds[0]);
        dup2(output_pipe_fds[1], STDOUT_FILENO);
        dup2(output_pipe_fds[1], STDERR_FILENO);
        close(output_pipe_fds[0]);
        close(output_pipe_fds[1]);

        if (execve(filename.c_str(), const_cast<char *const*>(argv),
            const_cast<char*const*>(envp))) {
            int saved_errno = errno;
            write(pipe_fds[1], &saved_errno, sizeof(saved_errno));
            abort();
        }
    } else {  
        close(pipe_fds[1]);
        close(output_pipe_fds[1]);
        int child_err = 0;
        int ret = 0;
        
        while ((ret = read(pipe_fds[0], &child_err, sizeof(child_err))) < 0) {
            if (errno != EAGAIN && errno != EINTR) {
                break;
            }
        }
        close(pipe_fds[0]);
    }
    int ret = 0;
    if (timeout_secs > 0) {
        ret = waitTimeout(timeout_secs);
    } else {
        ret = wait();
    }

    char buffer[1024];
    int n = 0;
    do {
        n = read(output_pipe_fds[0], buffer, sizeof(buffer));
        output_ += std::string(buffer, n);
    } while (n > 0);
    if (n < 0) {
        close(output_pipe_fds[0]);
        return -1;
    }
    close(output_pipe_fds[0]);
    return ret;
}

int Process::wait() {
    if (pid_ <= 0) {
        perror("Process::wait no child to wait");
        return -1;
    }
    int status = 0;
    pid_t ret = waitpid(pid_, &status, 0);
    if (ret < 0) {
        err_msg("Process::wait waitpid failed");
        return -1;
    }
    int exit_status = -1;
    if (WIFEXITED(status)) {
        exit_status = WEXITSTATUS(status);
    }
    return exit_status;
}

int Process::waitTimeout(int seconds) {
    if (pid_ <= 0) {
        perror("Process::waitTimeout no child to wait");
        return -1;
    }
    time_t begin = time(NULL);
    time_t now = begin;
    bool killed = false;
    int status = 0;
    while (true) {
        if (now - begin > seconds && !killed) {
            if (kill(pid_, SIGKILL) == 0) {
                killed = true;
            } else {
                err_sys("Process::wait kill failed");
            }
        }
        pid_t ret = waitpid(pid_, &status, WNOHANG);
        if (ret < 0) {
            err_msg("Process::wait waitpid failed");
            return -1;
        } else if (ret == 0) {
            sleep(1);
        } else {
            int exit_status = -1;
            if (WIFEXITED(status)) {
                exit_status = WEXITSTATUS(status);
            }
            return exit_status;
        }
        now = time(NULL);
    }
}