#ifndef __CLIENT__
#define __CLIENT__

#include <event.h>
#include "PipeTable.h"

//client conn timeout
const int CONN_TIMEOUT = 3; //3 seconds

typedef enum {
    kPipeReadNone,
    kPipeReadLen
}PIPE_READ_STATUS;

class Client;

static void timeout_handler(int sock, short which, void *arg);

class Client {
 public:
    //process pipe
    int pipeIndex;
    struct event pipeInEvt;
    struct event pipeOutEvt;

    //http client socket
    int fdSocket;
    struct bufferevent *httpEvt;

    //temp buffer used when outputing from socket and input to pipe.
    struct evbuffer* bufOut;

    //timer event after everything is done
    struct event *timerEvt;

    //input buf
    char* bufIn;
    int bufLen;
    int bufSize;
    PIPE_READ_STATUS bufInStatus;

 private:
    PipeTable* pipeTable;

 public:
    Client(int client_fd, 
           void (*buf_read_callback)(struct bufferevent *incoming,void *arg),
           void (*buf_write_callback)(struct bufferevent *bev,void *arg),
           void (*buf_error_callback)(struct bufferevent *bev,short what,void *arg),
           PipeTable* pTable): pipeIndex(-1), fdSocket(client_fd), 
        bufOut(evbuffer_new()), timerEvt(NULL), 
        bufIn(NULL), bufLen(0), bufSize(0), 
        bufInStatus(kPipeReadNone),pipeTable(pTable) {
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
        if( timerEvt ) {
            evtimer_del(timerEvt);
            timerEvt = NULL;
        }
        //free buIn
        freeInBuf();
        
        //free bufOut buffer
        evbuffer_free(bufOut);
        bufOut = NULL;
        
        //close socket
        close(fdSocket);
    }

    void closePipe() {
        if( pipeIndex != -1 ) {
            event_del( &pipeInEvt);
            event_del( &pipeOutEvt);
            pipeTable->releasePipe(pipeIndex);
            pipeIndex = -1;
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

    void startTimeoutTimer(struct event_base * evtBase) {
        struct timeval tv;
        tv.tv_sec = CONN_TIMEOUT; //very fast close
        tv.tv_usec = 0;
        timerEvt = evtimer_new(evtBase, timeout_handler, this);
        evtimer_add(timerEvt, &tv);
    }
};

static void timeout_handler(int sock, short which, void *arg){
    Client *client = (Client *)arg;    
    if ( client && client->timerEvt && !evtimer_pending(client->timerEvt, NULL)) {
        delete(client);
    }
}
#endif //__CLIENT__
