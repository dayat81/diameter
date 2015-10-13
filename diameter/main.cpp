/* A simple server in the internet domain using TCP
 The port number is passed as an argument */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>


void error(const char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
    int sockfd, newsockfd, portno;
    socklen_t clilen;
    char buffer[4];
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
    bzero(buffer,4);
    n = read(newsockfd,buffer,4);
    if (n < 0) error("ERROR reading from socket");
    char* b=buffer;
    int i=0;
    while (i<4) {
        printf("%02X ",*b&0xff);
        i++;
        b++;
    }
    printf("\n");
    int32_t l =(((0x00 & 0xff) << 24) | ((buffer[1] & 0xff) << 16)| ((buffer[2] & 0xff) << 8) | ((buffer[3] & 0xff)))-4;
    printf("len: %zu\n",l);
    char next[l];
    //bzero(next, l);
    n = read(newsockfd,next,l);
    if (n < 0) error("ERROR reading from socket");
    char* p=next;
    i=0;
    while (i<l) {
        printf("%02X ",*p&0xff);
        i++;
        p++;
    }
     printf("\n");
    n = write(newsockfd,"I got your message",18);
    if (n < 0) error("ERROR writing to socket");
    close(newsockfd);
    close(sockfd);
    return 0;
}