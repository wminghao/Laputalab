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
#define SERVER_PORT 8080

//process pipe table.
const int MAX_PROCESS_PIPES = 10; //max 32 instances at the same time.
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

//return index
int acquireUnusedPipe() {
    int index = -1;
    if( gPipeMask != ALL_PIPE_OCCUPIED ) {
        for( int i=0; i < 32 ; i++) {
            if( (gPipeMask & (~maskArray[i])) == 0 ) {
                gPipeMask |= (~maskArray[i]);
                index = i;
                break;
            }
        }
    }
    return index;
}

void releasePipe(int index) {
    if( index >= 0 && index < 32 ) {
         gPipeMask &= maskArray[index];
    }
}

typedef enum {
    kPipeReadNone,
    kPipeReadLen,
    kPipeReadJson
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

void freeClient(Client* client) {
    bufferevent_disable(client->httpEvt, EV_READ);
    bufferevent_free(client->httpEvt);
    if( client->pipeIndex != -1 ) {
        event_del( &client->pipeInEvt);
        event_del( &client->pipeOutEvt);
        releasePipe(client->pipeIndex);
        client->pipeIndex = -1;
    }
    close(client->fdSocket);
    evbuffer_free(client->bufOut);
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

void writeBuf(struct client *client, char* buf, int len ) 
{
    struct evbuffer* evreturn = evbuffer_new();
    evbuffer_add(evreturn, buf, len);
    bufferevent_write_buffer(client->httpEvt, evreturn);
    evbuffer_free(evreturn);
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
                    int jsonLen = 0;
                    memcpy(&jsonLen, lenStr, 4);
                    client->bufInStatus = kPipeReadLen;
                    
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
                            writeBuf( client, jsonBuf, jsonLen);
                            client->bufInStatus = kPipeReadJson;
                        } else {
                            client->bufIn = (char*)malloc(jsonLen);
                            client->bufSize = jsonLen;
                            client->bufLen = nRead;
                            memcpy(client->bufIn, jsonBuf, nRead);
                        }
                    }
                }
                break;
            }
        case kPipeReadLen: 
            {
                int remaining = client->bufSize-client->bufLen;
                char jsonBuf[ remaining ];
                int nRead = read( fd, jsonBuf, remaining);
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
                    memcpy(client->bufIn+client->bufLen, jsonBuf, nRead);
                    client->bufLen += nRead;

                    if( nRead == remaining ) {
                        writeBuf( client, client->bufIn, client->bufSize);
                        client->bufInStatus = kPipeReadJson;
                        free(client->bufIn);
                        client->bufIn = NULL;
                        client->bufLen = client->bufSize = 0;
                    }
                }
                break;
            }
        case kPipeReadJson:
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
            char* startPos = strstr(req, "GET /getlandmark?url=");
            char* endPos = strstr(req, "\r\n");
            if( startPos && endPos ) {
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
        gPipeMap.insert(Int_Pipe_Pair(i, new ProcessPipe()));
    }
    gPipeMask = 0;

    //start event loop
    event_init();
    
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
