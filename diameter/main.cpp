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
#include <sys/mman.h>
#include "entry.h"

#define PORT    "3868" /* Port to listen on */
#define BACKLOG     10  /* Passed to listen() */

//this class maintain socket list
class Callee : public CallbackInterface
{
public:
    int socket;
    // The callback function that Caller will call.
    void cbiCallbackFunction(int sock,std::string host)
    {
        printf("  Callee::cbiCallbackFunction() inside callback\n");
        std::cout<<host<<" "<<sock<<std::endl;
//        char r[1]={'a'};
//        int res=write(socket,r,1);
    }
};

/* Signal handler to reap zombie processes */
static void wait_for_child(int sig)
{
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

void handle(int newsock,int* num)
{
    /* recv(), send(), close() */
    char head[4];
    int n = read(newsock,head,4);
    char* h=head;
    
    int32_t l =(((0x00 & 0xff) << 24) | ((head[1] & 0xff) << 16)| ((head[2] & 0xff) << 8) | ((head[3] & 0xff)))-4;
    printf("len: %zu\n",l);
    
    char body[l];
    n = read(newsock,body,l);
    char* b=body;
    diameter d=diameter(h,b,l);
    printf("num : %i\n",*num);
    entry e=entry(*num);
    Callee callee;
    callee.socket=newsock;
    e.connectCallback(&callee);
    diameter reply=e.process(d);

    char resp[reply.len+4];
    char* r=resp;
    reply.compose(r);
    
    n = write(newsock,resp,reply.len+4);
}

int main(void)
{
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
    int *num = (int*)mmap(NULL, sizeof(int), PROT_READ|PROT_WRITE, MAP_ANON|MAP_SHARED, -1, 0);
    *num = 0;
    /* Main loop */
    while (1) {
        struct sockaddr cli_addr;
        socklen_t clilen;
        
        int newsock = accept(sock, &cli_addr, &clilen);
        int pid;
        
        if (newsock == -1) {
            perror("accept");
            return 0;
        }
        
        pid = fork();
        if (pid == 0) {
            /* In child process */
            close(sock);
            *num=*num+1;
            printf("handle num : %i\n",*num);
            handle(newsock,num);
            return 0;
        }
        else {
            /* Parent process */
            if (pid == -1) {
                perror("fork");
                return 1;
            }
            else {
                close(newsock);
            }
        }
    }
    
    close(sock);
    
    return 0;
}
