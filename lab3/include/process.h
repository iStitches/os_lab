#ifndef _PROCESS_H__
#define _PROCESS_H__

#include <string>
#include <sys/types.h>

class Process {
public:
    Process();

    ~Process();

    int exec(const std::string &filename,
        const char *const argv[],
        const char *const envp[],
        int timeout_secs = 0);

    std::string getExecOutput() {
        return output_;
    }

private:
    int wait();

    int waitTimeout(int seconds);

private:
    pid_t pid_;
    std::string output_;
    std::string working_dir_;
};

#endif
