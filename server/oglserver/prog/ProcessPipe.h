#ifndef __PROC_PIPE__
#define __PROC_PIPE__
#include <string>
#include <sys/types.h>

using namespace std;
class ProcessPipe
{
 public:
    ProcessPipe(const char* processLocation);
    ~ProcessPipe();

    int getInFd();
    int getOutFd();

 private:
    pid_t open(const char* processLoc);
    void close();

 private:
    int p_[2], q_[2];
    pid_t childPid_;
    string processLocation_;
};

#endif
