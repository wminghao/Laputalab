#include <stdio.h>
#include <stdlib.h>
#include <execinfo.h>
#include <sys/signal.h>
#include <time.h>
#include <errno.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>    /* For O_RDWR */
#include <unistd.h>   /* For open(), creat() */
#include <string>
#include <assert.h>
#include "utility/Output.h"

using namespace std;

void handlesig( int signum )
{
    OUTPUT("Exiting on signal: %d\r\n", signum );
    exit( 0 );
}

bool doRead( int fd, char *buf, size_t len ) {
    size_t bytesRead = 0;
    
    while ( bytesRead < len ) {
        size_t bytesToRead = len - bytesRead;
        int t = read( fd, buf + bytesRead, bytesToRead );
        if ( t <= 0 ) {
            OUTPUT("read() failed: %s\r\n", strerror(errno) );
                        
            return false;
        }
        bytesRead += t;
    }
    
    return true;
}

bool doWrite( int fd, const char *buf, size_t len ) {
    size_t bytesWrote = 0;
    
    while ( bytesWrote < len ) {
        size_t bytesToWrite = len - bytesWrote;
        int t = write( fd, buf + bytesWrote, bytesToWrite );
        if ( t <= 0 ) {
            OUTPUT("write() failed: %s \r\n", strerror(errno) );
            return false;
        }
        
        bytesWrote += t;
    }
    
    return true;
}

//test example to read input, parse the image data and return immediately
int main()
{
    signal( SIGSEGV, handlesig );
    signal( SIGFPE, handlesig );
    signal( SIGBUS, handlesig );
    signal( SIGSYS, handlesig );

    Logger::initLog("dummyProc");

    OUTPUT("------dummy started=%d\r\n");
    bool bWorking = true;
    while ( bWorking ) {
        char buf[4]; 
        bWorking = doRead( 0, buf, 4 );
        if( bWorking ) {
            int totalBytes = 0;
            memcpy( &totalBytes, buf, 4 );
            OUTPUT("------dummy read data, size=%d\n", totalBytes);

            doWrite( 1, buf, 4);
            fsync( 1 ); //flush the buffer

            char buf2[totalBytes];
            bWorking = doRead( 0, buf2, totalBytes );
            if( bWorking ) {
                doWrite( 1, buf2, totalBytes);
                fsync( 1 ); //flush the buffer
            }
        }
    }
    OUTPUT("------dummy ended=%d\r\n");

    return 0;
}
