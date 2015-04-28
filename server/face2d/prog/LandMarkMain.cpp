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
#include <jansson.h>
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

//convert from jsonArray into jsonObject
bool convertToJson(string& jsonArray, string& jsonObject) {
    bool bIsSucess = false;

    //input
    json_t *rootInput;
    json_error_t error;

    rootInput = json_loads(jsonArray.c_str(), 0, &error);
    if( rootInput && json_is_array(rootInput)) {
        int total = json_array_size(rootInput);
        //output
        json_t* rootOutput = json_object();
        for(int i = 0; i < total; i+=2) {
            json_t *data = json_array_get(rootInput, i);
            double x = json_real_value(data);
            data = json_array_get(rootInput, i+1);
            double y = json_real_value(data);

            char pointKeyword[100];
            sprintf(pointKeyword, "point%02d", i/2);
            json_t* point_obj = json_object();
            json_object_set_new( rootOutput, pointKeyword, point_obj );            
            json_object_set_new( point_obj, "x", json_real(x) );
            json_object_set_new( point_obj, "y", json_real(y) );
        }
        
        bIsSucess = true;
        char *ret_strings = json_dumps(rootOutput, JSON_SORT_KEYS|JSON_REAL_PRECISION(6));
        jsonObject = ret_strings;
        free(ret_strings);
        json_decref( rootOutput );
    }
    json_decref( rootInput );
    return bIsSucess;
}

//#define TEST_INPUT

int main()
{
    signal( SIGSEGV, handlesig );
    signal( SIGFPE, handlesig );
    signal( SIGBUS, handlesig );
    signal( SIGSYS, handlesig );

    Logger::initLog("LandMarkProc");

    LandMark* lm = new LandMark();

#ifdef TEST_INPUT
    char outputFilename[] = "out.list";
    FILE *ofp = fopen(outputFilename, "w");
#endif

    OUTPUT("------LandMarkProc started!\r\n");
    bool bWorking = true;
#ifdef TEST_INPUT
    for(int i=0; i<2; i++) {
#else 
    while ( true ) { 
#endif
        char lenBuf[4]; 
        bWorking = doRead( 0, lenBuf, 4 );
#ifdef TEST_INPUT
        int t = fwrite(lenBuf, 1, 4, ofp);
#endif
        if( bWorking ) {
            int pathLen = 0;
            memcpy(&pathLen, lenBuf, 4);
            //OUTPUT("------LandMark read data, size=%d\n", pathLen);
            bool bIsSuccess = false;
            char buf[pathLen];
            bWorking = doRead( 0, buf, pathLen );
            if( bWorking ) {
#ifdef TEST_INPUT
                fwrite(buf, 1, pathLen, ofp);
#endif
                buf[pathLen]='\0';
                string faceImg(buf);
                //OUTPUT("------LandMark read data, path=%s\n", faceImg.c_str());
                string jsonArray;
                string jsonObject;
                int ret = lm->ProcessImage( faceImg, jsonArray );
                if( !ret ) {
                    if(convertToJson(jsonArray, jsonObject)) {
                        size_t resLen = jsonObject.length();
                        memcpy(buf, &resLen, 4);
                        doWrite( 1, buf, 4);
                        fsync( 1 ); //flush the buffer
                    
                        if( resLen > 0 ) {
                            doWrite( 1, jsonObject.c_str(), resLen);
                            fsync( 1 ); //flush the buffer
                            //OUTPUT("------LandMark result len=%d", resLen);
                        }
                    }
                } else {
                    OUTPUT("----LandMark cannot process! ret=%d\n", ret);
                    memset( buf, 0, 4 );
                    doWrite( 1, buf, 4 );
                    fsync( 1 ); //flush the buffer
                }     
            }        
        }
    }
    delete(lm);

#ifdef TEST_INPUT
    fclose(ofp);
#endif    
    OUTPUT("------LandMark ended!\n");    
    return 0;
}
