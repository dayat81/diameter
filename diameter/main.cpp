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
#include <assert.h>
#include "rocksdb/db.h"
#include "entry.h"

#define PORT    "3868" /* Port to listen on */
#define BACKLOG     10  /* Passed to listen() */

//this class maintain socket list
class Callee : public CallbackInterface
{
public:
    int socket;
    rocksdb::DB* db;
    // The callback function that Caller will call.
    void cbiCallbackFunction(std::string host)
    {
        printf("  Callee::cbiCallbackFunction() inside callback\n");
        std::cout<<host<<","<<socket<<std::endl;
        
        rocksdb::Status status = db->Put(rocksdb::WriteOptions(), host, std::to_string(socket));

    }
};

void *handle(void *);
void *handlecmd(void *);
void *handlecommand(void *);
rocksdb::DB* db;
rocksdb::Options options;
int main(void)
{
    options.create_if_missing = true;
    rocksdb::Status status = rocksdb::DB::Open(options, "/tmp/testdb", &db);
    assert(status.ok());
    
    int sock,sock1;
    
    struct addrinfo hints, *res,hints1,*res1;
    int reuseaddr = 1; /* True */
    
    /* Get the address info */
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    if (getaddrinfo(NULL, PORT, &hints, &res) != 0) {
        perror("getaddrinfo");
        return 1;
    }

    /* Get the address provisioning socket info */
    memset(&hints1, 0, sizeof hints1);
    hints1.ai_family = AF_INET;
    hints1.ai_socktype = SOCK_STREAM;
    if (getaddrinfo(NULL, "1234", &hints1, &res1) != 0) {
        perror("getaddrinfo");
        return 1;
    }
    
    /* Create the socket */
    sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sock == -1) {
        perror("socket");
        return 1;
    }
    
    /* Create the socket for provisioning*/
    sock1 = socket(res1->ai_family, res1->ai_socktype, res1->ai_protocol);
    if (sock1 == -1) {
        perror("socket");
        return 1;
    }
    
    /* Enable the socket to reuse the address */
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(int)) == -1) {
        perror("setsockopt");
        return 1;
    }
    
    /* Enable the socket to reuse the address */
    if (setsockopt(sock1, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(int)) == -1) {
        perror("setsockopt");
        return 1;
    }
    
    /* Bind to the address */
    if (bind(sock, res->ai_addr, res->ai_addrlen) == -1) {
        perror("bind");
        return 1;
    }
    
    /* Bind to the address */
    if (bind(sock1, res1->ai_addr, res1->ai_addrlen) == -1) {
        perror("bind1");
        return 1;
    }
    
    /* Listen */
    if (listen(sock, BACKLOG) == -1) {
        perror("listen");
        return 1;
    }

    /* Listen */
    if (listen(sock1, BACKLOG) == -1) {
        perror("listen1");
        return 1;
    }
    
    freeaddrinfo(res);
    freeaddrinfo(res1);
    pthread_t thread;
    int iret = pthread_create( &thread, NULL, handlecmd, (void*) &sock1);
    if(iret)
    {
        fprintf(stderr,"Error - pthread_create() return code: %d\n",iret);
        exit(EXIT_FAILURE);
    }

    /* Main loop */
    while (1) {
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

void *handlecmd(void *socket){
    while(1){
        struct sockaddr cli_addr;
        socklen_t clilen;
        int sock = *(int*)socket;
        int newsock = accept(sock, &cli_addr, &clilen);
        if (newsock == -1) {
            perror("accept");
            return 0;
        }
        printf("cmd connected\n");
        pthread_t thread1;
        int iret1 = pthread_create( &thread1, NULL, handlecommand, (void*) &newsock);
        if(iret1)
        {
            fprintf(stderr,"Error - pthread_create() return code: %d\n",iret1);
            exit(EXIT_FAILURE);
        }
    }
    return 0;
}

void *handlecommand(void *sock){
    int newsock = *(int*)sock;
    int bytes;
    char cClientMessage[599];
    
    while((bytes = recv(newsock, cClientMessage, sizeof(cClientMessage), 0)) > 0)
    {
        char* chars_array = strtok(cClientMessage, "#:");
        int i=0;
        char* params[3];
        while(chars_array)
        {
            //        MessageBox(NULL, subchar_array, NULL, NULL);
            std::cout << chars_array << '\n';
            params[i]=chars_array;
            i++;
            chars_array = strtok(NULL, "#:");
        }
        //printf("%s\n",cClientMessage);
        if( memcmp( params[0], "peer", strlen( "peer") ) == 0 &&memcmp( params[1], "all", strlen( "all") ) == 0 ) {
            //printf("dump peer here\n");
            rocksdb::Iterator* it = db->NewIterator(rocksdb::ReadOptions());
            for (it->SeekToFirst(); it->Valid(); it->Next()) {
                std::cout << it->key().ToString() << ": " << it->value().ToString() << std::endl;
            }
            assert(it->status().ok());
        }else if (memcmp( params[0], "send", strlen( "send") ) == 0){
            printf("send to socket %s msg %s \n",params[1],params[2]);
            int n = write(atoi(params[1]),params[2],10);
        }
    }
    
    if(bytes == 0)
    {
        //socket was gracefully closed
    }
    else if(bytes < 0)
    {
        //socket error occurred
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
    callee.db=db;
    e.connectCallback(&callee);
    diameter reply=e.process(d);
    
    char resp[reply.len+4];
    char* r=resp;
    reply.compose(r);
    
    n = write(newsock,resp,reply.len+4);
    return 0;
}
