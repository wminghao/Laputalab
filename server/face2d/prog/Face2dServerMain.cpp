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
#include "utility/Output.h"
#include "ProcessPipe.h"
#include "PipeTable.h"
#include "Client.h"
#include "PendingTask.h"

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

PipeTable* gPipeTable;
PendingTaskTable* gPendingTasks;
struct event_base * gEvtBase;

void setnonblock(int fd) {
    int flags;    
    flags = fcntl(fd, F_GETFL);
    flags |= O_NONBLOCK;
    fcntl(fd, F_SETFL, flags);
}
void pipe_read_callback(int fd,short ev,void *arg);
void pipe_write_callback(int fd,short ev,void *arg);

static void nextPendingTask() {
    Client* nextClient = NULL;
    SmartPtr<PendingTask> pendingTask = gPendingTasks->getNext(nextClient);
    if( nextClient != NULL ) {
        if(nextClient->tryToEnablePipe(pendingTask->getUrlStr(), pendingTask->getUrlLen(), pipe_read_callback, pipe_write_callback) ) {
            OUTPUT("remove pending task, client=0x%x\n", nextClient);
            gPendingTasks->removeNext();
        }
    }
}

static void deleteClient(Client* client) {
    gPendingTasks->removeTask(client);
    delete(client);
}

static void timeout_handler(int sock, short which, void *arg){
    Client *client = (Client *)arg;    
    if ( client ) {
        OUTPUT("close and deleted client");
        deleteClient(client);
    }
}

void pipe_write_callback(int fd,
                         short ev,
                         void *arg)
{
    Client *client = (Client *)arg;
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
                        event_add(&client->pipeInEvt, NULL);
                    } else {
                        OUTPUT("process pipe write error!");
                        deleteClient(client);
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

void pipe_read_callback(int fd,
                        short ev,
                        void *arg)
{
    Client *client = (Client *)arg;
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
                        deleteClient(client);
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
                                deleteClient(client);
                            }
                        } else {
                            OUTPUT("process pipe read=%d!\n", nRead);
                            if( nRead == jsonLen ) {
                                client->writeBuf(TWO_HUNDRED_OK, strlen((char*)TWO_HUNDRED_OK));
                                client->writeBuf(jsonBuf, jsonLen);
                                client->closePipe();
                                client->startTimeoutTimer(gEvtBase, timeout_handler);
                                nextPendingTask(); //start the next task
                            } else {
                                client->bufIn = (char*)malloc(jsonLen);
                                client->bufSize = jsonLen;
                                client->bufLen = nRead;
                                memcpy(client->bufIn, jsonBuf, nRead);
                            }
                        }
                    } else {
                        client->writeBuf(FIVE_HUNDRED_ERROR, strlen((char*)FIVE_HUNDRED_ERROR));
                        client->closePipe();
                        client->startTimeoutTimer(gEvtBase, timeout_handler);
                        nextPendingTask(); //start the next task
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
                        deleteClient(client);
                    }
                } else {
                    OUTPUT("process pipe read=%d!\n", nRead);
                    memcpy(client->bufIn+client->bufLen, remBuf, nRead);
                    client->bufLen += nRead;

                    if( nRead == remaining ) {
                        client->writeBuf(TWO_HUNDRED_OK, strlen((char*)TWO_HUNDRED_OK));
                        client->writeBuf(client->bufIn, client->bufSize);
                        client->freeInBuf();
                        client->closePipe();
                        client->startTimeoutTimer(gEvtBase, timeout_handler);
                        nextPendingTask(); //start the next task
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
    Client *client = (Client *)arg;
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
                if( !client->tryToEnablePipe( url, sizeof(url), pipe_read_callback, pipe_write_callback) ) {
                    OUTPUT("add pending task, client=0x%x, urllen=%d, url=%s\n", client, urlLen, url+sizeof(int));
                    gPendingTasks->addTask(client, url, sizeof(url));
                }
            }
        } else {
            OUTPUT("----fatal error!");
            freeClient(client);
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
    deleteClient((Client *)arg);
}

void accept_callback(int fd,
                     short ev,
                     void *arg)
{
    int client_fd;
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);


    client_fd = accept(fd,
                       (struct sockaddr *)&client_addr,
                       &client_len);
    if (client_fd < 0) {
        OUTPUT("Client: accept() failed");
        return;
    }
    
    setnonblock(client_fd);
    
    Client* client = new Client(client_fd, 
                                buf_read_callback,
                                buf_write_callback,
                                buf_error_callback,
                                gPipeTable);
    if (client == NULL) {
        OUTPUT("client alloc failed");
        return;
    }
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
    gPipeTable = new PipeTable(PROCESS_LOCATION);

    //pending tasks table
    gPendingTasks = new PendingTaskTable();

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

    delete(gPipeTable);
    delete(gPendingTasks);
    return 0;
}
