#include <event.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <unordered_map>
#include "Output.h"
#include "ProcessPipe.h"

using namespace std;
#define SERVER_PORT 1234

//#define TEST_DUMMY

#ifdef TEST_DUMMY
const char* PROCESS_LOCATION = "dummy";//"/usr/bin/dummy";
#else
const char* PROCESS_LOCATION = "LandMarkMain";//"/usr/bin/LandMarkMain";
#endif

const char* LANDMARK_URL_PREFIX = "GET /getlandmark?url=";
const char* LANDMARK_URL_SUFFIX = " HTTP/";

const char* TWO_HUNDRED_OK = "HTTP/1.1 200 OK\r\n\r\n";
const char* FIVE_HUNDRED_ERROR = "HTTP/1.1 500 Cannot process image\r\n\r\n";
const char* FIVE_HUNDRED_THREE_ERROR = "HTTP/1.1 503 Service unavailable. Too busy\r\n\r\n";

//client conn timeout
const int CONN_TIMEOUT = 3; //3 seconds

//process pipe table.
const int MAX_PROCESS_PIPES = 4; //max 32 instances at the same time.
typedef pair <int, ProcessPipe*> Int_Pipe_Pair;
unordered_map <int, ProcessPipe*> gPipeMap;
const unsigned int maskArray[ ] = {0x7fffffff,
                                   0xbfffffff,
                                   0xdfffffff,
                                   0xefffffff,

                                   0xf7ffffff,
                                   0xfbffffff,
                                   0xfdffffff,
                                   0xfeffffff,

                                   0xff7fffff,
                                   0xffbfffff,
                                   0xffdfffff,
                                   0xffefffff,

                                   0xfff7ffff,
                                   0xfffbffff,
                                   0xfffdffff,
                                   0xfffeffff,

                                   0xffff7fff,
                                   0xffffbfff,
                                   0xffffdfff,
                                   0xffffefff,

                                   0xfffff7ff,
                                   0xfffffbff,
                                   0xfffffdff,
                                   0xfffffeff,

                                   0xffffff7f,
                                   0xffffffbf,
                                   0xffffffdf,
                                   0xffffffef,

                                   0xfffffff7,
                                   0xfffffffb,
                                   0xfffffffd,
                                   0xfffffffe };
const unsigned int ALL_PIPE_OCCUPIED = 0xffffffff;
unsigned int gPipeMask;
struct event_base * gEvtBase;


//return index
int acquireUnusedPipe() {
    int index = -1;
    if( gPipeMask != ALL_PIPE_OCCUPIED ) {
        for( int i=0; i < MAX_PROCESS_PIPES ; i++) {
            if( (gPipeMask & (~maskArray[i])) == 0 ) {
                gPipeMask |= (~maskArray[i]);
                index = i;
                OUTPUT("acquireUnusedPipe, index=%d\r\n", index);
                break;
            }
        }
    }
    return index;
}

void releasePipe(int index) {
    if( index >= 0 && index < MAX_PROCESS_PIPES ) {
         gPipeMask &= maskArray[index];
         OUTPUT("releasePipe, index=%d\r\n", index);
    }
}

typedef enum {
    kPipeReadNone,
    kPipeReadLen
}PIPE_READ_STATUS;

typedef struct client {
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
} Client;

void setnonblock(int fd) {
    int flags;    
    flags = fcntl(fd, F_GETFL);
    flags |= O_NONBLOCK;
    fcntl(fd, F_SETFL, flags);
}

void closeClientPipe(Client* client) {
    if( client->pipeIndex != -1 ) {
        event_del( &client->pipeInEvt);
        event_del( &client->pipeOutEvt);
        releasePipe(client->pipeIndex);
        client->pipeIndex = -1;
    }
}
void freeClientInBuf(Client* client) {
    free(client->bufIn);
    client->bufIn = NULL;
    client->bufLen = client->bufSize = 0;
    client->bufInStatus = kPipeReadNone;
}
void freeClient(Client* client) {
    //close events
    bufferevent_disable(client->httpEvt, EV_READ);
    bufferevent_free(client->httpEvt);
    client->httpEvt = NULL;

    //free pipe
    closeClientPipe( client );

    //free timer event
    if( client->timerEvt ) {
        evtimer_del(client->timerEvt);
        client->timerEvt = NULL;
    }
    //free buIn
    freeClientInBuf(client);
    
    //free bufOut buffer
    evbuffer_free(client->bufOut);
    client->bufOut = NULL;

    //close socket
    close(client->fdSocket);

    //free everything
    free(client);
}

void pipe_write_callback(int fd,
                         short ev,
                         void *arg)
{
    struct client *client = (struct client *)arg;
    if( client) {
        int len = evbuffer_get_length(client->bufOut);
        if( len > 0 ) {
            event_del( &client->pipeInEvt);
            while( len > 0 ) {
                int nbytes = evbuffer_write(client->bufOut, fd);
                if( nbytes <= 0 ) {
                    int netErrorNumber = errno;
                    if ( netErrorNumber == EAGAIN) {
                        OUTPUT("-----write again later----\r\n");
                        event_add(&client->pipeInEvt,
                                  NULL);
                    } else {
                        OUTPUT("process pipe write error!");
                        freeClient(client);
                    }
                    break;
                } else {
                    OUTPUT("process pipe written=%d!\n", nbytes);
                    evbuffer_drain(client->bufOut, nbytes);
                    len -= nbytes;
                }
            } 
        }
    }
}

void writeBuf(struct client *client, const char* buf, int len ) 
{
    struct evbuffer* evreturn = evbuffer_new();
    evbuffer_add(evreturn, buf, len);
    bufferevent_write_buffer(client->httpEvt, evreturn);
    evbuffer_free(evreturn);
}

static void timeout_handler(int sock, short which, void *arg) {
    struct client *client = (struct client *)arg;    
    if ( client && client->timerEvt && !evtimer_pending(client->timerEvt, NULL)) {
        freeClient(client);
    }
}

void startTimeoutTimer(struct client *client) {
    struct timeval tv;
    tv.tv_sec = CONN_TIMEOUT; //very fast close
    tv.tv_usec = 0;
    client->timerEvt = evtimer_new(gEvtBase, timeout_handler, client);
    evtimer_add(client->timerEvt, &tv);
}

void pipe_read_callback(int fd,
                        short ev,
                        void *arg)
{
    struct client *client = (struct client *)arg;
    if( client ) {
        switch( client->bufInStatus) {
        case kPipeReadNone: 
            {
                unsigned char lenStr[4];
                memset(lenStr, 0, 4);
                int nRead = read( fd, lenStr, 4);
                if( nRead <= 0 ) {
                    if ( errno == EAGAIN ) {
                        //try again
                        OUTPUT("-----read again later----\r\n");
                    } else {
                        OUTPUT("process pipe read error!");
                        freeClient(client);
                    }
                } else {
                    ASSERT( nRead == 4);
                    int jsonLen = 0;
                    memcpy(&jsonLen, lenStr, 4);
                    client->bufInStatus = kPipeReadLen;
                    if( jsonLen > 0 ) {
                        char jsonBuf[jsonLen];
                        int nRead = read( fd, jsonBuf, jsonLen);
                        if( nRead <= 0 ) {
                            if ( errno == EAGAIN ) {
                                //try again
                                OUTPUT("-----read again later----\r\n");
                            } else {
                                OUTPUT("process pipe read error!");
                                freeClient(client);
                            }
                        } else {
                            OUTPUT("process pipe read=%d!\n", nRead);
                            if( nRead == jsonLen ) {
                                writeBuf( client, TWO_HUNDRED_OK, strlen((char*)TWO_HUNDRED_OK));
                                writeBuf( client, jsonBuf, jsonLen);
                                closeClientPipe( client );
                                startTimeoutTimer( client );
                            } else {
                                client->bufIn = (char*)malloc(jsonLen);
                                client->bufSize = jsonLen;
                                client->bufLen = nRead;
                                memcpy(client->bufIn, jsonBuf, nRead);
                            }
                        }
                    } else {
                        writeBuf( client, FIVE_HUNDRED_ERROR, strlen((char*)FIVE_HUNDRED_ERROR));
                        closeClientPipe( client );
                        startTimeoutTimer( client );
                    }
                }
                break;
            }
        case kPipeReadLen: 
            {
                int remaining = client->bufSize-client->bufLen;
                char remBuf[ remaining ];
                int nRead = read( fd, remBuf, remaining);
                if( nRead <= 0 ) {
                    if ( errno == EAGAIN ) {
                        //try again
                        OUTPUT("-----read again later----\r\n");
                    } else {
                        OUTPUT("process pipe read error!");
                        freeClient(client);
                    }
                } else {
                    OUTPUT("process pipe read=%d!\n", nRead);
                    memcpy(client->bufIn+client->bufLen, remBuf, nRead);
                    client->bufLen += nRead;

                    if( nRead == remaining ) {
                        writeBuf( client, TWO_HUNDRED_OK, strlen((char*)TWO_HUNDRED_OK));
                        writeBuf( client, client->bufIn, client->bufSize);
                        freeClientInBuf(client);
                        closeClientPipe( client );
                        startTimeoutTimer( client );
                    }
                }
                break;
            }
        default:
            {
                ASSERT(0);
                break;
            }
        }
    }
}

void buf_read_callback(struct bufferevent *incoming,
                       void *arg)
{
    struct client *client = (struct client *)arg;
    if( client ) {
        int len = evbuffer_get_length(incoming->input);
        char req[len];
        int ret = evbuffer_remove(incoming->input, req, len);
        ASSERT(len == ret);
        if ( ret > 0 ) {
            char* startPos = strstr(req, LANDMARK_URL_PREFIX);
            char* endPos = strstr(req, LANDMARK_URL_SUFFIX);
            if( startPos && endPos ) {
                startPos += strlen(LANDMARK_URL_PREFIX);
                int urlLen =(endPos-startPos);
                char url[urlLen+sizeof(int)];
                memcpy(url, &urlLen, sizeof(int));
                memcpy(url+sizeof(int), startPos, urlLen);
                if( client->pipeIndex == -1 ) {
                    client->pipeIndex = acquireUnusedPipe();
                }
                if( client->pipeIndex != -1 ) {
                    ProcessPipe* pipe = gPipeMap[client->pipeIndex];
                    //register input
                    event_set(&client->pipeInEvt,
                              pipe->getInFd(),
                              EV_WRITE|EV_PERSIST,
                              pipe_write_callback,
                              client);
                    event_add(&client->pipeInEvt,
                              NULL);
                    
                    //register output
                    event_set(&client->pipeOutEvt,
                              pipe->getOutFd(),
                              EV_READ|EV_PERSIST,
                              pipe_read_callback,
                              client);
                    event_add(&client->pipeOutEvt,
                              NULL);                
                    
                    OUTPUT("registered processpipe, inFd=%d, outFd=%d, urllen=%d, url=%s\n", pipe->getInFd(), pipe->getOutFd(), urlLen, url+sizeof(int));
                    
                    //copy data for future write
                    evbuffer_add(client->bufOut, url, sizeof(url));
                } else {
                    writeBuf( client, FIVE_HUNDRED_THREE_ERROR, strlen((char*)FIVE_HUNDRED_THREE_ERROR));
                    startTimeoutTimer( client );
                }
            }
        } else {
            OUTPUT("----fatal error!");
        }
    }
}

void buf_write_callback(struct bufferevent *bev,
                        void *arg)
{
    //Not implemented, not interested in write ready state.
}

void buf_error_callback(struct bufferevent *bev,
                        short what,
                        void *arg)
{
    //this happens when the client disconnects.
    OUTPUT("---client disconnected and returned\n");

    struct client *client = (struct client *)arg;
    freeClient(client);
}

void accept_callback(int fd,
                     short ev,
                     void *arg)
{
    int client_fd;
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    struct client *client;

    client_fd = accept(fd,
                       (struct sockaddr *)&client_addr,
                       &client_len);
    if (client_fd < 0) {
        OUTPUT("Client: accept() failed");
        return;
    }
    
    setnonblock(client_fd);
    
    client = (struct client*)calloc(1, sizeof(*client));
    if (client == NULL) {
        OUTPUT("malloc failed");
    }
    client->fdSocket = client_fd;
    client->pipeIndex = -1; //unassigned.
    client->bufOut = evbuffer_new();
    
    client->httpEvt = bufferevent_new(client_fd,
                                     buf_read_callback,
                                     buf_write_callback,
                                     buf_error_callback,
                                     client);

    bufferevent_enable(client->httpEvt, EV_READ);
}

int main(int argc,
         char **argv)
{
    int socketlisten;
    struct sockaddr_in addresslisten;
    struct event accept_ev;
    int reuse = 1;

    Logger::initLog("Face2ServerMain");    

    //start hashmap, launch 10 processes
    for(int i = 0; i< MAX_PROCESS_PIPES; i++) {
        gPipeMap.insert(Int_Pipe_Pair(i, new ProcessPipe(PROCESS_LOCATION)));
    }
    gPipeMask = 0;

    //start event loop
    gEvtBase = event_init();
    
    socketlisten = socket(AF_INET, SOCK_STREAM, 0);
    
    if (socketlisten < 0) {
        OUTPUT("Failed to create listen socket");
        return 1;
    }
    
    memset(&addresslisten, 0, sizeof(addresslisten));
    
    addresslisten.sin_family = AF_INET;
    addresslisten.sin_addr.s_addr = INADDR_ANY;
    addresslisten.sin_port = htons(SERVER_PORT);

    setsockopt(socketlisten,
               SOL_SOCKET,
               SO_REUSEPORT|SO_REUSEADDR,
               &reuse,
               sizeof(reuse));
    
    setnonblock(socketlisten);

    if (bind(socketlisten,
             (struct sockaddr *)&addresslisten,
             sizeof(addresslisten)) < 0) {
        OUTPUT("Failed to bind");
        return 1;
    }
    
    if (listen(socketlisten, 5) < 0) {
        OUTPUT("Failed to listen to socket");
        return 1;
    }
    
    event_set(&accept_ev,
              socketlisten,
              EV_READ|EV_PERSIST,
              accept_callback,
              NULL);
    
    event_add(&accept_ev,
              NULL);
    
    event_dispatch();
    
    close(socketlisten);
    
    return 0;
}
