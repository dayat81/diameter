/* A simple server in the internet domain using TCP
 The port number is passed as an argument */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <map>
#include "entry.h"

//this class maintain socket list
class Callee : public CallbackInterface
{
public:
    std::map<std::string, int> m;
    // The callback function that Caller will call.
    void cbiCallbackFunction(int sock,std::string host)
    {
        printf("  Callee::cbiCallbackFunction() inside callback\n");
        std::cout<<sock<<" "<<host<<std::endl;
        m[host] = sock;
    }
};
void error(const char *msg)
{
    perror(msg);
    exit(1);
}
//void foo( std::string& str, char ch,int sock, int sz,std::string host )
//{
//    std::cout << "foo( '" << str << "', '" << ch << "', " << sz << " )\n"<<host<<" "<<sock<<std::endl ;
//    str += std::string( sz, ch ) ;
//}
int main(int argc, char *argv[])
{
    Callee callee;
    int sockfd, newsockfd, portno;
    socklen_t clilen;
    char head[4];
    struct sockaddr_in serv_addr, cli_addr;
    int n;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");
    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = 3868;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
     int reuseaddr = 1; /* True */
    /* Enable the socket to reuse the address */
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(int)) == -1) {
        perror("setsockopt");
        return 1;
    }
    if (bind(sockfd, (struct sockaddr *) &serv_addr,
             sizeof(serv_addr)) < 0)
        error("ERROR on binding");
    listen(sockfd,5);
    clilen = sizeof(cli_addr);
    newsockfd = accept(sockfd,
                       (struct sockaddr *) &cli_addr,
                       &clilen);
    if (newsockfd < 0)
        error("ERROR on accept");
    //bzero(buffer,4);
    n = read(newsockfd,head,4);
    if (n < 0) error("ERROR reading from socket");
    char* h=head;

    int32_t l =(((0x00 & 0xff) << 24) | ((head[1] & 0xff) << 16)| ((head[2] & 0xff) << 8) | ((head[3] & 0xff)))-4;
    //printf("len: %zu\n",l);
    char body[l];
    
    //bzero(next, l);
    n = read(newsockfd,body,l);
    if (n < 0) error("ERROR reading from socket");
    char* b=body;
    diameter d=diameter(h,b,l);
    
    entry e=entry(newsockfd);
    e.connectCallback(&callee);
    diameter reply=e.process(d);
    //reply.dump();
    //printf("\n");
    std::map<std::string, int>::iterator it;
    
    for ( it = callee.m.begin(); it != callee.m.end(); it++ )
    {
        std::cout << it->first  // string (key)
        << ':'
        << it->second   // string's value
        << std::endl ;
    }
    std::string str = "hello world" ;
    //reply.mylibfun_add_tail( 1, 4, foo, std::ref(str), '!' ,newsockfd) ;
    std::cout << "str: '" << str << " "<<newsockfd<<"'\n------------------------\n" ;
    
    char resp[reply.len+4];
    char* r=resp;
    reply.compose(r);
    
    n = write(newsockfd,resp,reply.len+4);
    if (n < 0) error("ERROR writing to socket");
    close(newsockfd);
    close(sockfd);
    return 0;
}