#include "ProcessPipe.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/fcntl.h>
#include <sys/wait.h>
#include <sys/prctl.h>
#include <sys/signal.h>
#include <sys/types.h>
#include <assert.h>
#include <vector>
#include "utility/Output.h"

const char* LD_LIBRARY_PATH = "LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib:.";

void killChild() {
    OUTPUT( "ProcessPipe exiting." );
    //kill(childPid_, SIGKILL);
    //wait(0);
}

ProcessPipe::ProcessPipe(const char* processLoc):processLocation_(processLoc)
{    
    atexit(killChild);    
    childPid_ = open(processLoc);
}
ProcessPipe::~ProcessPipe()
{
    close();
}
int ProcessPipe::getInFd() 
{
    return p_[1];
}

int ProcessPipe::getOutFd()
{
    return q_[0];
}

pid_t ProcessPipe::open(const char* processLocation)
{
    if ( 0 > pipe( p_ ) || 0 > pipe( q_ ) ) {
        OUTPUT("!!!Failed to create process pipe = %s p_[1]=%d q_[0]=%d" , processLocation, p_[1], q_[0]);        
        perror( "pipe" );
        return -1;
    }
    char **arguments;
    {
        std::vector<char *> strings;
        strings.push_back( strdup( processLocation ) );
        //strings.push_back( strdup( more arg ) );
        size_t arrayLen = strings.size() + 1;
        arguments = new char *[ arrayLen ];
        for ( size_t i = 0 ; i < arrayLen - 1 ; ++i ) {
            arguments[i] = strings[i];
        }
        arguments[ arrayLen - 1 ] = NULL;
    }

    char **env;
    {
        std::vector<char *> stringsEnv;
        stringsEnv.push_back( strdup( LD_LIBRARY_PATH ) );
        size_t arrayLenEnv = stringsEnv.size() + 1;
        env = new char *[ arrayLenEnv ];
        for ( size_t i = 0 ; i < arrayLenEnv - 1 ; ++i ) {
            env[i] = stringsEnv[i];
            //OUTPUT("env[%d]=%s", i, env[i]);
        }
        env[ arrayLenEnv - 1 ] = NULL;
    }
    
    pid_t rval = fork();
    
    if( 0 == rval ) {
        //child process
        ::close(p_[1]);
        ::close(q_[0]);
        
        if( 0 > dup2( p_[0], 0 ) ) {
            perror( "dup2 in child p_[0]" );
        }
        
        if( 0 > dup2( q_[1], 1 ) ) {
            perror( "dup2 in child q_[1]" );
        }
        
        if( 0 > prctl( PR_SET_PDEATHSIG, SIGKILL ) ) {
            perror( "prctl" );
        }

        //somehow execve does not work, have to change /etc/ld.so.conf.d/
        //if( -1 == execve( processLocation, arguments, env ) ) {
        if( -1 == execvp( processLocation, arguments ) ) {
            assert(0);
            OUTPUT("Fatal error: EXECLP FAILED, error=%d?!\n", errno);
        }

        exit(-1);
    } else {
        //parent process
        ::close(p_[0]);
        ::close(q_[1]);
        OUTPUT("----Launching process=%s, pid=%d", processLocation, rval);
    }    
    delete [] arguments;
    delete [] env;
    
    return rval;
}

void ProcessPipe::close()
{
    OUTPUT("----Closing process=%s, pid=%d", processLocation_.c_str(), childPid_);
    if ( childPid_ ) {
        ::kill( childPid_, SIGKILL );
        int status;
        while ( waitpid( -1, &status, WNOHANG ) > 0 ) {}
        childPid_ = 0;
    }
    ::close(p_[1]);
    ::close(q_[0]);
    OUTPUT("----Closed process=%s", processLocation_.c_str());
    return;
}
