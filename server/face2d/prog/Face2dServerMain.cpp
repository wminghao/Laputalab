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
#include "Output.h"
#include "ProcessPipe.h"

#define SERVER_PORT 8080

const int MAX_BUFFER_IN_PER_CLIENT = 4096;
const int MAX_BUFFER_OUT_PER_CLIENT = 4096;

typedef struct client {
    //process pipe
    ProcessPipe* pipe;
    struct event pipeInEvt;
    struct event pipeOutEvt;

    //http client socket
    int fdSocket;
    struct bufferevent *httpEvt;

    //temp buffer used when doing I/O.
    unsigned char bufOut[MAX_BUFFER_OUT_PER_CLIENT];
    int bufOutLen;
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
    if( client->pipe ) {
        event_del( &client->pipeInEvt);
        event_del( &client->pipeOutEvt);
        delete( client->pipe );
        client->pipe = NULL;
    }
    close(client->fdSocket);
    free(client);
}

void pipe_write_callback(int fd,
                         short ev,
                         void *arg)
{
    struct client *client = (struct client *)arg;
    if( client) {
        while( client->bufOutLen > 0 ) {
            int nbytes = write(fd, client->bufOut, client->bufOutLen);
            if( nbytes <= 0 ) {
                int netErrorNumber = errno;
                if ( netErrorNumber == EAGAIN) {
                    OUTPUT("-----write again later----\r\n");
                } else {
                    OUTPUT("process pipe write error!");
                    freeClient(client);
                }
                break;
            } else {
                OUTPUT("process pipe write done. total written=%d!\n", client->bufOutLen);
                int remainingTotal = client->bufOutLen - nbytes;
                if( remainingTotal > 0 ) {
                    memmove( client->bufOut, client->bufOut+nbytes, remainingTotal);
                }
                client->bufOutLen = remainingTotal;
            }
        } 
    }
}

void pipe_read_callback(int fd,
                        short ev,
                        void *arg)
{
    struct client *client = (struct client *)arg;
    if( client ) {
        unsigned char bufIn[MAX_BUFFER_IN_PER_CLIENT];
        int nRead = read( fd, bufIn, MAX_BUFFER_IN_PER_CLIENT);
        if( nRead <= 0 ) {
            if ( errno == EAGAIN ) {
                //try again
                OUTPUT("-----read again later----\r\n");
            } else {
                OUTPUT("process pipe read error!");
                freeClient(client);
            }
        } else {
            OUTPUT("process pipe read done. total read=%d!\n", nRead);
            struct evbuffer* evreturn = evbuffer_new();
            evbuffer_add_printf(evreturn,"You said %s\n", bufIn);
            bufferevent_write_buffer(client->httpEvt, evreturn);
            evbuffer_free(evreturn);
        }
    }
}

void buf_read_callback(struct bufferevent *incoming,
                       void *arg)
{
    struct client *client = (struct client *)arg;
    if( client ) {
        int len = evbuffer_get_length(incoming->input);
        char *req = evbuffer_readline(incoming->input);
        if (req != NULL) {
            if( !client->pipe ) {
                client->pipe = new ProcessPipe();
            }
            if( client->pipe ) {
                //register input
                event_set(&client->pipeInEvt,
                          client->pipe->getInFd(),
                          EV_WRITE|EV_PERSIST,
                          pipe_write_callback,
                          client);
                event_add(&client->pipeInEvt,
                          NULL);                
                
                //register output
                event_set(&client->pipeOutEvt,
                          client->pipe->getOutFd(),
                          EV_READ|EV_PERSIST,
                          pipe_read_callback,
                          client);
                event_add(&client->pipeOutEvt,
                          NULL);                
                
                OUTPUT("registered processpipe, inFd=%d, outFd=%d\n", client->pipe->getInFd(), client->pipe->getOutFd());

                //TODO copy partial data
                //copy data for future write
                memcpy(client->bufOut, req, len);
                client->bufOutLen = len;
            }
            free(req);
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
