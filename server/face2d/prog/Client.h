
#ifndef __CLIENT__
#define __CLIENT__

#include <event.h>
#include "PipeTable.h"
#include "utility/Output.h"

//client conn timeout
const int CONN_TIMEOUT = 3; //3 seconds

typedef enum {
    kPipeReadNone,
    kPipeReadLen
}PIPE_READ_STATUS;

class Client;

typedef void (*Timeout_Handler)(int sock, short which, void *arg);

class Client {
 public:
    //process pipe
    struct event pipeInEvt;
    struct event pipeOutEvt;

    //http client socket
    struct bufferevent *httpEvt;

    //temp buffer used when outputing from socket and input to pipe.
    struct evbuffer* bufOut;

    //input buf
    char* bufIn;
    int bufLen;
    int bufSize;
    PIPE_READ_STATUS bufInStatus;

 private:
    PipeTable* pipeTable_;
    
    //pipe index
    int pipeIndex_;

    //http client socket
    int fdSocket_;

    //timer event after everything is done
    struct event *timerEvt_;

 public:
    Client(int client_fd, 
           void (*buf_read_callback)(struct bufferevent *incoming,void *arg),
           void (*buf_write_callback)(struct bufferevent *bev,void *arg),
           void (*buf_error_callback)(struct bufferevent *bev,short what,void *arg),
           PipeTable* pTable): bufOut(evbuffer_new()), 
        bufIn(NULL), bufLen(0), bufSize(0), 
        bufInStatus(kPipeReadNone), pipeTable_(pTable),pipeIndex_(-1), fdSocket_(client_fd), timerEvt_(NULL) {
        httpEvt = bufferevent_new(client_fd,
                                          buf_read_callback,
                                          buf_write_callback,
                                          buf_error_callback,
                                          this);
    }

    ~Client() {
        //close events
        bufferevent_disable(httpEvt, EV_READ);
        bufferevent_free(httpEvt);
        httpEvt = NULL;
        
        //free pipe
        closePipe();
        
        //free timer event
        if( timerEvt_ ) {
            evtimer_del(timerEvt_);
            event_free(timerEvt_);
            timerEvt_ = NULL;
        }
        //free buIn
        freeInBuf();
        
        //free bufOut buffer
        evbuffer_free(bufOut);
        bufOut = NULL;
        
        //close socket
        close(fdSocket_);
    }

    void closePipe() {
        if( pipeIndex_ != -1 ) {
            event_del( &pipeInEvt);
            event_del( &pipeOutEvt);
            pipeTable_->releasePipe(pipeIndex_);
            pipeIndex_ = -1;
        }
    }
    void freeInBuf() {
        free(bufIn);
        bufIn = NULL;
        bufLen = bufSize = 0;
        bufInStatus = kPipeReadNone;
    }

    void writeBuf(const char* buf, int len ) 
    {
        struct evbuffer* evreturn = evbuffer_new();
        evbuffer_add(evreturn, buf, len);
        bufferevent_write_buffer(httpEvt, evreturn);
        evbuffer_free(evreturn);
    }

    void startTimeoutTimer(struct event_base * evtBase, Timeout_Handler handler) {
        struct timeval tv;
        tv.tv_sec = CONN_TIMEOUT; //very fast close
        tv.tv_usec = 0;
        timerEvt_ = evtimer_new(evtBase, handler, this);
        evtimer_add(timerEvt_, &tv);
    }

    bool tryToEnablePipe( char* url, int len,
                     void (*pipe_read_callback)(int fd,
                                                short ev,
                                                void *arg),
                     void (*pipe_write_callback)(int fd,
                                                 short ev,
                                                 void *arg)
                     ) {
        bool ret = false;
        if( pipeIndex_ == -1 ) {
            pipeIndex_ = pipeTable_->acquireUnusedPipe();
        }
        if( pipeIndex_ != -1 ) {
            ProcessPipe* pipe = pipeTable_->get(pipeIndex_);
            //register input
            event_set(&pipeInEvt,
                      pipe->getInFd(),
                      EV_WRITE|EV_PERSIST,
                      pipe_write_callback,
                      this);
            event_add(&pipeInEvt,
                      NULL);
            
            //register output
            event_set(&pipeOutEvt,
                      pipe->getOutFd(),
                      EV_READ|EV_PERSIST,
                      pipe_read_callback,
                      this);
            event_add(&pipeOutEvt,
                      NULL);                
            //copy data for future write
            evbuffer_add(bufOut, url, len);
            OUTPUT("registered processpipe, inFd=%d, outFd=%d, urllen=%d, url=%s\n", pipe->getInFd(), pipe->getOutFd(), len, url+sizeof(int));                    

            ret = true;
        }
        return ret;
    }
};
#endif //__CLIENT__
