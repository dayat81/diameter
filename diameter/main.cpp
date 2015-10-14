/* A simple server in the internet domain using TCP
 The port number is passed as an argument */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "diameter.h"
#include "avputil.h"


void error(const char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
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
    
    char resp[l+4];
    char* r=resp;
    //bzero(next, l);
    n = read(newsockfd,body,l);
    if (n < 0) error("ERROR reading from socket");
    char* b=body;
    diameter d=diameter(h,b,l);
    d.compose(r);
    //get avp
    d.populateHeader();
    avputil util=avputil();
    avp oh = d.getAVP(264,0);
    printf("origin host len : %i\n",oh.len);
    if(oh.len!=0){
        std::cout<<"decoded : "<<util.decodeAsString(oh)<<std::endl;
    }
//    int codes[4]={443,0,444,0};
//    int* code=codes;
    avp sid=d.getAVP(443,0);
    printf("sid len : %i\n",sid.len);
    if(sid.len!=0){
        d.b=d.b-sid.len;
        avp iddata=d.getAVP(444, 0);
        printf("iddata len : %i\n",iddata.len);
        std::cout<<"decoded : "<<util.decodeAsString(iddata)<<std::endl;
    }
    //d.dump();
    printf("\n");

    n = write(newsockfd,resp,l+4);
    if (n < 0) error("ERROR writing to socket");
    close(newsockfd);
    close(sockfd);
    return 0;
}