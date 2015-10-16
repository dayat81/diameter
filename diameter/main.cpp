/*
 *  A forked server
 *  by Martin Broadhurst (www.martinbroadhurst.com)
 */

#include <stdio.h>
#include <string.h> /* memset() */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <netdb.h>
#include <iostream>
#include <vector>
#include <pthread.h>
#include "entry.h"

#define PORT    "3868" /* Port to listen on */
#define BACKLOG     10  /* Passed to listen() */

struct socketpeer {
    int socket;
    std::string peer;
} ;
//std::vector<socketpeer> *vect;
//this class maintain socket list
class Callee : public CallbackInterface
{
public:
    int socket;
    std::vector<socketpeer> *vect;
    // The callback function that Caller will call.
    void cbiCallbackFunction(std::string host)
    {
        printf("  Callee::cbiCallbackFunction() inside callback\n");
        std::cout<<host<<","<<socket<<std::endl;
//        socketpeer mine;
//        mine.peer=host;
//        mine.socket=socket;
//        vect->push_back(mine);
        //        char r[1]={'a'};
        //        int res=write(socket,r,1);
    }
};

/* Signal handler to reap zombie processes */
static void wait_for_child(int sig)
{
    while (waitpid(-1, NULL, WNOHANG) > 0);
}
void *handle(void *);
int main(void)
{
    std::vector<socketpeer> *vect = new std::vector<socketpeer>;
    
    int sock;
    struct sigaction sa;
    struct addrinfo hints, *res;
    int reuseaddr = 1; /* True */
    
    /* Get the address info */
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    if (getaddrinfo(NULL, PORT, &hints, &res) != 0) {
        perror("getaddrinfo");
        return 1;
    }
    
    /* Create the socket */
    sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sock == -1) {
        perror("socket");
        return 1;
    }
    
    /* Enable the socket to reuse the address */
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(int)) == -1) {
        perror("setsockopt");
        return 1;
    }
    
    /* Bind to the address */
    if (bind(sock, res->ai_addr, res->ai_addrlen) == -1) {
        perror("bind");
        return 1;
    }
    
    /* Listen */
    if (listen(sock, BACKLOG) == -1) {
        perror("listen");
        return 1;
    }
    
    freeaddrinfo(res);
    
    /* Set up the signal handler */
    sa.sa_handler = wait_for_child;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        return 1;
    }
    /* Main loop */
    while (1) {
        for (std::vector<socketpeer>::iterator it = vect->begin() ; it != vect->end(); ++it)
            std::cout << ' oi ' << it->peer;
        std::cout << '\n';
        struct sockaddr cli_addr;
        socklen_t clilen;
        
        int newsock = accept(sock, &cli_addr, &clilen);
        
        if (newsock == -1) {
            perror("accept");
            return 0;
        }
        pthread_t thread1;
        int iret1 = pthread_create( &thread1, NULL, handle, (void*) &newsock);
        if(iret1)
        {
            fprintf(stderr,"Error - pthread_create() return code: %d\n",iret1);
            exit(EXIT_FAILURE);
        }
        
    }
    
    return 0;
}

void *handle(void *sock){
    int newsock = *(int*)sock;
    char head[4];
    int n = read(newsock,head,4);
    char* h=head;
    
    int32_t l =(((0x00 & 0xff) << 24) | ((head[1] & 0xff) << 16)| ((head[2] & 0xff) << 8) | ((head[3] & 0xff)))-4;
    //printf("len: %zu\n",l);
    
    char body[l];
    n = read(newsock,body,l);
    char* b=body;
    diameter d=diameter(h,b,l);
    entry e=entry();
    Callee callee;
    callee.socket=newsock;
    //callee.vect=vect;
    e.connectCallback(&callee);
    diameter reply=e.process(d);
    
    char resp[reply.len+4];
    char* r=resp;
    reply.compose(r);
    
    n = write(newsock,resp,reply.len+4);
    return 0;
}
