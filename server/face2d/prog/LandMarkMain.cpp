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
#include "LandMark.h"
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

#define BUF_SIZE 1024

int main()
{
    signal( SIGSEGV, handlesig );
    signal( SIGFPE, handlesig );
    signal( SIGBUS, handlesig );
    signal( SIGSYS, handlesig );

    Logger::initLog("LandMarkProc");

    LandMark* lm = new LandMark();

    OUTPUT("------LandMarkProc started!\r\n");
    char buf[BUF_SIZE]; 
    bool bWorking = true;
    while ( true ) { 
        bWorking = doRead( 0, buf, 4 );
        if( bWorking ) {
            int pathLen = 0;
            memcpy(&pathLen, buf, 4);
            //OUTPUT("------LandMark read data, size=%d\n", pathLen);
            bool bIsSuccess = true;
            if( pathLen < BUF_SIZE ) {
                bWorking = doRead( 0, buf, pathLen );
                if( bWorking ) {
                    buf[pathLen]='\0';
                    string faceImg(buf);
                    OUTPUT("------LandMark read data, path=%s\n", faceImg.c_str());
                    string jsonObject;
                    if(!lm->ProcessImage( faceImg, jsonObject )) {
                        size_t resLen = jsonObject.length();
                        memcpy(buf, &resLen, 4);
                        doWrite( 1, buf, 4);
                        fsync( 1 ); //flush the buffer
                        
                        char* result =(char*) malloc(resLen);
                        if( result ) {
                            memcpy(result, jsonObject.c_str(), resLen);
                            doWrite( 1, result, resLen);
                            fsync( 1 ); //flush the buffer
                            result[resLen] = '\0';
                            //OUTPUT("------LandMark result=%s", result);
                            free(result);
                        }
                    } else {
                        OUTPUT("----LandMark cannot process!\n");
                        bIsSuccess = false;
                    }
                }
            } else {
                OUTPUT("----LandMark read path too long!\n");
                bIsSuccess = false;
            }
            if( !bIsSuccess ) {
                memset( buf, 0, 4 );
                doWrite( 1, buf, 4 );
                fsync( 1 ); //flush the buffer
            }
        }
    }
    OUTPUT("------LandMark ended=%d\r\n");
    
    delete(lm);
    return 0;
}
