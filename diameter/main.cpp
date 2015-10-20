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
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "entry.h"


#define BACKLOG     10  /* Passed to listen() */
using namespace rapidjson;
//this class maintain socket list
class Callee : public CallbackInterface
{
public:
    int socket;
    rocksdb::DB* db;
    // The callback function that Caller will call.
    void cbiCallbackFunction(std::string host)
    {
        //printf("  Callee::cbiCallbackFunction() inside callback\n");
        //std::cout<<host<<","<<socket<<std::endl;
        
        rocksdb::Status status = db->Put(rocksdb::WriteOptions(), host, std::to_string(socket));

    }
};
//void remove_escape(char *string);
void *handle(void *);
void *handlecmd(void *);
void *handlecommand(void *);
rocksdb::DB* db;
rocksdb::Options options;
int main(void)
{
    options.create_if_missing = true;
    rocksdb::Status status = rocksdb::DB::Open(options, DB_PATH, &db);
    assert(status.ok());
    
    int sock,sock1;
    
    struct addrinfo hints, *res,hints1,*res1;
    int reuseaddr = 1; /* True */
    
    /* Get the address info */
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    if (getaddrinfo(HOST_IP, PORT, &hints, &res) != 0) {
        perror("getaddrinfo");
        return 1;
    }

    /* Get the address provisioning socket info */
    memset(&hints1, 0, sizeof hints1);
    hints1.ai_family = AF_INET;
    hints1.ai_socktype = SOCK_STREAM;
    if (getaddrinfo(HOST_IP, CMDPORT, &hints1, &res1) != 0) {
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
        //printf("cmd connected\n");
        pthread_t thread1;
        int iret1 = pthread_create( &thread1, NULL, handlecommand, (void*) &newsock);
        if(iret1)
        {
            fprintf(stderr,"Error - pthread_create() return code: %d\n",iret1);
            exit(EXIT_FAILURE);
        }
    }
    pthread_exit(NULL);
    return 0;
}

char* to_char(const std::string& string)
{
    char* return_string = new char[string.length() + 1];
    strcpy(return_string, string.c_str());
    
    return return_string;
}

int getsocketid(char* msid){
    //get session id
    std::string val;
    strcat(msid, "_sess");
    rocksdb::Status status = db->Get(rocksdb::ReadOptions(),msid, &val);
    std::cout<<val<<std::endl;
    char* peer=strtok(to_char(val), "#;");
    //printf("peer %s\n",peer);
    status = db->Get(rocksdb::ReadOptions(),peer, &val);
    std::cout<<val<<std::endl;
    
    return atoi(to_char(val));
}

void *handlecommand(void *sock){
    int newsock = *(int*)sock;
    int bytes;
    char cClientMessage[32];
    char result[1024];
    int prompt=write(newsock, "pcrf>", 5);
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
        if( memcmp( params[0], "show", strlen( "show") ) == 0 &&memcmp( params[1], "all", strlen( "all") ) == 0 ) {
            //printf("dump peer here\n");
            rocksdb::Iterator* it = db->NewIterator(rocksdb::ReadOptions());
            char result[1024];
            bzero(result, 1024);
            for (it->SeekToFirst(); it->Valid(); it->Next()) {
                //std::cout << it->key().ToString() << ": " << it->value().ToString() << std::endl;
                char* key = to_char(it->key().ToString());
                char* val = to_char(it->value().ToString());
                strcat(result, key);
                strcat(result, ":");
                strcat(result, val);
                strcat(result, "\n");
            }
            assert(it->status().ok());
            strcat(result, "pcrf>");
            int res=write(newsock, result, strlen(result));
        }else if(memcmp( params[0], "quit", strlen( "quit") ) == 0){
            close(newsock);
        
        }else if (memcmp( params[0], "rar", strlen( "rar") ) == 0){
            //printf("send rar to %s \n",params[1]);
            //getrar here
            entry e=entry();
            e.db=db;
            char* msg;
            diameter reply=e.createRAR(params[1]);
            if(reply.len>0){
                //reply.dump();
                char resp[reply.len+4];
                char* r=resp;
                reply.compose(r);
                
                //find socket here
                int socketmsid=getsocketid(params[1]);
                
                int w = write(socketmsid,resp,reply.len+4);
                if(w<=0){
                    //fail write
                    msg="rar is failed";
                }else{
                    msg="rar is sent";
                }
            }else{
                msg="rar is not required";
            }
            bzero(result, 1024);
            strcat(result, msg);
            strcat(result, "\npcrf>");
            int res=write(newsock, result, strlen(result));
        }else if( memcmp( params[0], "add", strlen( "add") ) == 0 &&memcmp( params[1], "msid", strlen( "msid") ) == 0 ) {
            //char result[1024];
            bzero(result, 1024);
            std::string val;
            char* info="_rarinfo";
            char rarinfo[strlen(params[2])+strlen(info)];
            strcpy(rarinfo,params[2]); // copy string one into the result.
            strcat(rarinfo,info); // append string two to the result.
            //cek if default exist the copy to msid
            std::string valdef;
            rocksdb::Status status = db->Get(rocksdb::ReadOptions(),"default", &valdef);
            if(valdef==""){
                valdef="{\"acg\":[]}";
            }
            status = db->Put(rocksdb::WriteOptions(),params[2], valdef);
            status = db->Put(rocksdb::WriteOptions(),rarinfo, "{\"addacg\":[],\"delacg\":[]}");
            std::cout<<val<<std::endl;
            char* value = to_char(val);
            strcat(result, value);
            strcat(result, "OK\npcrf>");
            int res=write(newsock, result, strlen(result));
        }else if( memcmp( params[0], "del", strlen( "del") ) == 0 &&memcmp( params[1], "msid", strlen( "msid") ) == 0 ) {
            //char result[1024];
            bzero(result, 1024);
            //std::string val;
            char* info="_rarinfo";
            char rarinfo[strlen(params[2])+strlen(info)];
            strcpy(rarinfo,params[2]); // copy string one into the result.
            strcat(rarinfo,info); // append string two to the result.
            rocksdb::Status status = db->Delete(rocksdb::WriteOptions(),params[2]);
            status = db->Delete(rocksdb::WriteOptions(),rarinfo);
            strcat(result, "OK\npcrf>");
            int res=write(newsock, result, strlen(result));
        }else if( memcmp( params[0], "show", strlen( "show") ) == 0 &&memcmp( params[1], "msid", strlen( "msid") ) == 0 ) {
            //char result[1024];
            bzero(result, 1024);
            std::string val;
            rocksdb::Status status = db->Get(rocksdb::ReadOptions(),params[2], &val);
            std::cout<<val<<std::endl;
            char* value = to_char(val);
            strcat(result, value);
            strcat(result, "\npcrf>");
            int res=write(newsock, result, strlen(result));
        }else if( memcmp( params[0], "delacg", strlen( "delacg") ) == 0) {
//            char* msid=params[2];
//            remove_escape(msid);
//            printf("show msid %s\n",msid);
            std::string val,val1;
            rocksdb::Status status = db->Get(rocksdb::ReadOptions(),params[2], &val);
            std::cout<<val<<std::endl;
            char json[val.size()+1];//as 1 char space for null is also required
            strcpy(json, val.c_str());
            Document dom;
            //printf("Original json:\n%s\n", json);
            char buffer[sizeof(json)];
            memcpy(buffer, json, sizeof(json));
            if (dom.ParseInsitu<0>(buffer).HasParseError())
                printf("error parsing\n");
            Value& a = dom["acg"];
            assert(a.IsArray());
            for (Value::ConstValueIterator itr = a.Begin(); itr != a.End(); ++itr){
                const char* acg=itr->GetString();
                //printf("acg %s\n",acg);
                if(strcmp(acg, params[1]) == 0){
                    //printf("delete %s\n",params[1]);
                    a.Erase(itr);
                    //write rarinfo
                    char* info="_rarinfo";
                    char rarinfo[strlen(params[2])+strlen(info)];
                    strcpy(rarinfo,params[2]); // copy string one into the result.
                    strcat(rarinfo,info); // append string two to the result.
                    //printf("rarinfo %s\n",rarinfo);
                    status = db->Get(rocksdb::ReadOptions(),rarinfo, &val1);
                    std::cout<<val1<<std::endl;
                    char json1[val1.size()+1];//as 1 char space for null is also required
                    strcpy(json1, val1.c_str());
                    Document dom1;
                    //printf("Original json:\n%s\n", json);
                    char buffer1[sizeof(json1)];
                    memcpy(buffer1, json1, sizeof(json1));
                    if (dom1.ParseInsitu<0>(buffer1).HasParseError())
                        printf("error parsing\n");
                    Value& a1 = dom1["delacg"];
                    assert(a1.IsArray());
                    Document::AllocatorType& allocator1 = dom1.GetAllocator();
                    Value newacg1;
                    a1.PushBack(newacg1.SetString(params[1], strlen(params[1])), allocator1);
                    StringBuffer strbuf1;
                    Writer<StringBuffer> writer1(strbuf1);
                    dom1.Accept(writer1);
                    status = db->Put(rocksdb::WriteOptions(),rarinfo, strbuf1.GetString());
                    break;
                }
            }
            //printf("Updated json:\n");
            
            // Convert JSON document to string
            StringBuffer strbuf;
            Writer<StringBuffer> writer(strbuf);
            dom.Accept(writer);
            // string str = buffer.GetString();
            //printf("--\n%s\n--\n", strbuf.GetString());
            status = db->Put(rocksdb::WriteOptions(),params[2], strbuf.GetString());
            //char result[1024];
            bzero(result, 1024);
            strcat(result, to_char(strbuf.GetString()));
            strcat(result, "\npcrf>");
            int res=write(newsock, result, strlen(result));

        }else if( memcmp( params[0], "addacg", strlen( "addacg") ) == 0) {
            std::string val,val1;
            rocksdb::Status status = db->Get(rocksdb::ReadOptions(),params[2], &val);
            //std::cout<<val<<std::endl;
            char json[val.size()+1];//as 1 char space for null is also required
            strcpy(json, val.c_str());
            Document dom;
            //printf("Original json:\n%s\n", json);
            char buffer[sizeof(json)];
            memcpy(buffer, json, sizeof(json));
            if (dom.ParseInsitu<0>(buffer).HasParseError())
                printf("error parsing\n");
            Value& a = dom["acg"];
            assert(a.IsArray());
            Document::AllocatorType& allocator = dom.GetAllocator();
            Value newacg;
            a.PushBack(newacg.SetString(params[1], strlen(params[1])), allocator);
            //write rarinfo
            char* info="_rarinfo";
            char rarinfo[strlen(params[2])+strlen(info)];
            strcpy(rarinfo,params[2]); // copy string one into the result.
            strcat(rarinfo,info); // append string two to the result.
            //printf("rarinfo %s\n",rarinfo);
            status = db->Get(rocksdb::ReadOptions(),rarinfo, &val1);
            std::cout<<val1<<std::endl;
            char json1[val1.size()+1];//as 1 char space for null is also required
            strcpy(json1, val1.c_str());
            Document dom1;
            //printf("Original json:\n%s\n", json);
            char buffer1[sizeof(json1)];
            memcpy(buffer1, json1, sizeof(json1));
            if (dom1.ParseInsitu<0>(buffer1).HasParseError())
                printf("error parsing\n");
            Value& a1 = dom1["addacg"];
            assert(a1.IsArray());
            Document::AllocatorType& allocator1 = dom1.GetAllocator();
            Value newacg1;
            a1.PushBack(newacg1.SetString(params[1], strlen(params[1])), allocator1);
            StringBuffer strbuf1;
            Writer<StringBuffer> writer1(strbuf1);
            dom1.Accept(writer1);
            status = db->Put(rocksdb::WriteOptions(),rarinfo, strbuf1.GetString());
            
            //printf("Updated json:\n");
            // Convert JSON document to string
            StringBuffer strbuf;
            Writer<StringBuffer> writer(strbuf);
            dom.Accept(writer);
            // string str = buffer.GetString();
            //printf("--\n%s\n--\n", strbuf.GetString());
            status = db->Put(rocksdb::WriteOptions(),params[2], strbuf.GetString());
            char result[1024];
            bzero(result, 1024);
            strcat(result, to_char(strbuf.GetString()));
            strcat(result, "\npcrf>");
            int res=write(newsock, result, strlen(result));
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
    pthread_exit(NULL);
    return 0;
}

void *handle(void *sock){
    int newsock = *(int*)sock;
    char* h=new char[4];
    int n;
    while((n=read(newsock,h,4))>0){
        //char* h=head;
        
        int32_t l =(((0x00 & 0xff) << 24) | ((*(h+1) & 0xff) << 16)| ((*(h+2) & 0xff) << 8) | ((*(h+3) & 0xff)))-4;
        //printf("len: %zu\n",l);
        
        //char body[l];
        char* b=new char[l];
        n = read(newsock,b,l);
        //char* b=new char[l];
        diameter d=diameter(h,b,l);
        entry e=entry();
        Callee callee;
        callee.socket=newsock;
        callee.db=db;
        e.connectCallback(&callee);
        e.db=db;
        
        diameter reply=e.process(d);
        delete b;
        if(reply.len>0){    //isrequest, need answer
            //char resp[reply.len+4];
            char* r=new char[reply.len+4];
            reply.compose(r);
            delete reply.h;
            delete reply.b;
            int w = write(newsock,r,reply.len+4);
            if(w<=0){
                //fail write
            }
            delete r;
        }
    }
    delete h;
    pthread_exit(NULL);
  if(n == 0)
  {
      //socket was gracefully closed
  }
  else if(n < 0)
  {
      //socket error occurred
  }
    return 0;
}
