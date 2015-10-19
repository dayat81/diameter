//
//  entry.cpp
//  diameter
//
//  Created by hidayat on 10/14/15.
//  Copyright © 2015 hidayat. All rights reserved.
//

#include <stdio.h>
#include "entry.h"
#include "avputil.h"
#include "logic.h"
#include <iostream>
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include <string>
#include <sstream>
#include <vector>

using namespace rapidjson;
entry::entry(){
}
void getUnable2Comply(diameter d,avp* &allavp,int &l,int &total){
    avputil util=avputil();
    
    //read avp
    avp ori_host=d.getAVP(264, 0);
    printf("ori len %i \n",ori_host.len);
    if(ori_host.len>0){
        std::cout<<util.decodeAsString(ori_host)<<std::endl;
    }
    
    char f=0x40;
    std::string ori =ORIGIN_HOST;
    //printf("size : %i\n",ori.size());
    avp o=util.encodeString(264,0,f,ori);
    //o.dump();
    //printf("\n");
    avp id_t=util.encodeInt32(450, 0, 0x40, 1);
    //id_t.dump();
    //printf("\n");
    avp id_d=util.encodeString(444, 0, 0x40, "628119105569");
    //id_d.dump();
    avp* listavp[2]={&id_t,&id_d};
    avp sid=util.encodeAVP(443, 0, 0x40, listavp, 2);
    
    avp id_t1=util.encodeInt32(450, 0, 0x40, 0);
    avp id_d1=util.encodeString(444, 0, 0x40, "51010628119105569");
    avp* listavp1[2]={&id_t1,&id_d1};
    avp sid1=util.encodeAVP(443, 0, 0x40, listavp1, 2);
    
    //sid.dump();
    //printf("\n");
    total=sid.len+o.len+sid1.len;
    l=3;
    allavp=new avp[l];
    allavp[0]=o;
    allavp[1]=sid;
    allavp[2]=sid1;
}
std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}


std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, elems);
    return elems;
}
void entry::getRAR(char* msid,avp* &allavp,int &l,int &total){
    avputil util=avputil();
    //get sessionid of the msid
    char* sesinfo="_sess";
    char sessinfo[strlen(msid)+strlen(sesinfo)];
    strcpy(sessinfo,msid); // copy string one into the result.
    strcat(sessinfo,sesinfo); // append string two to the result.
    std::string valses;
    rocksdb::Status status = entry::db->Get(rocksdb::ReadOptions(),sessinfo, &valses);
    std::cout<<sessinfo<<" == "<<valses<<"=="<<std::endl;
    
    char f=0x40;
    avp sessid=util.encodeString(263, 0, f, valses);
    avp o=util.encodeString(264,0,f,ORIGIN_HOST);
    avp realm=util.encodeString(296,0,f,ORIGIN_REALM);
    avp authappid=util.encodeInt32(258, 0, f, 16777238);
    std::string peer=split(valses, ';')[0];
    avp dh=util.encodeString(293, 0, f, peer);
    avp rartype=util.encodeInt32(285, 0, f, 0);
    //read rar_info
    char* info="_rarinfo";
    char rarinfo[strlen(msid)+strlen(info)];
    strcpy(rarinfo,msid); // copy string one into the result.
    strcat(rarinfo,info); // append string two to the result.
    std::string val;
    status = entry::db->Get(rocksdb::ReadOptions(),rarinfo, &val);
    std::cout<<rarinfo<<" == "<<val<<std::endl;
    Document dom;
    dom.Parse(val.c_str());
    
    avp cr_install=avp(0, 0);
    avp cr_remove=avp(0, 0);
    l=6;
    int i=l;
    total=o.len+sessid.len+realm.len+authappid.len+dh.len+rartype.len;
    const Value& a = dom["addacg"];
    assert(a.IsArray());
    //printf("cek addacg\n");
    if(a.Size()>0){
        avp* acg=new avp[a.Size()];
        for (SizeType i = 0; i < a.Size(); i++){ // Uses SizeType instead of size_t
            
            avp temp=util.encodeString(1005, 10415, 0xC0, a[i].GetString());
            temp.dump();
            //printf("\n");
            *acg=temp;
            acg++;
        }
        acg=acg-a.Size();
        cr_install=util.encodeAVP(1001, 10415, 0xC0, acg, a.Size());
        total=total+cr_install.len;
        l++;
    }
    const Value& del = dom["delacg"];
    assert(del.IsArray());
    //printf("cek delacg\n");
    if(del.Size()>0){
        printf("delsize %i\n",del.Size());
        avp* delacg=new avp[del.Size()];
        for (SizeType i = 0; i < del.Size(); i++){ // Uses SizeType instead of size_t
            
            avp temp=util.encodeString(1005, 10415, 0xC0, del[i].GetString());
            temp.dump();
            //printf("\n");
            *delacg=temp;
            delacg++;
        }
        delacg=delacg-del.Size();
        cr_remove=util.encodeAVP(1002, 10415, 0xC0, delacg, del.Size());
        l++;
        total=total+cr_remove.len;
    }
    //reset rar_info
    status = db->Put(rocksdb::WriteOptions(), rarinfo, "{\"delacg\":[],\"addacg\":[]}");
    allavp=new avp[l];
    allavp[0]=sessid;
    allavp[1]=o;
    allavp[2]=realm;
    allavp[3]=authappid;
    allavp[4]=dh;
    allavp[5]=rartype;
    if(cr_install.len>0){
        allavp[i]=cr_install;
        i++;
    }
    if(cr_remove.len>0){
        allavp[i]=cr_remove;
    }
}
void getDWA(avp* &allavp,int &l,int &total){
    avputil util=avputil();
    char f=0x40;
    avp o=util.encodeString(264,0,f,ORIGIN_HOST);
    avp realm=util.encodeString(296,0,f,ORIGIN_REALM);
    avp rc=util.encodeInt32(268, 0, f, 2001);
    total=o.len+realm.len+rc.len;
    allavp=new avp[l];
    allavp[0]=o;
    allavp[1]=realm;
    allavp[2]=rc;
}
void getCEA(diameter d,avp* &allavp,int &l,int &total,std::string &host){
    avputil util=avputil();
    
    //read avp
    avp ori_host=d.getAVP(264, 0);
    printf("ori len %i \n",ori_host.len);
    if(ori_host.len>0){
        //std::cout<<util.decodeAsString(ori_host)<<std::endl;
        host=util.decodeAsString(ori_host);
    }
    
    char f=0x40;
    avp o=util.encodeString(264,0,f,ORIGIN_HOST);
    avp realm=util.encodeString(296,0,f,ORIGIN_REALM);
    avp vid=util.encodeInt32(266, 0, f, 0);
    avp pn=util.encodeString(269, 0, f, "simple PCRF");
    unsigned int ipval[4]={10,195,84,157};
    avp ip=util.encodeIP(257, 0, f, ipval);
    avp authappid=util.encodeInt32(258, 0, f, 16777238);
    avp svid=util.encodeInt32(265, 0, f, 10415);
    avp svid1=util.encodeInt32(265, 0, f, 193);
    avp rc=util.encodeInt32(268, 0, f, 2001);
    avp osid=util.encodeInt32(278, 0, f, 1);
    //ip.dump();
    //vid.dump();
    //sid.dump();
    //printf("\n");
    total=o.len+realm.len+vid.len+pn.len+ip.len+authappid.len+svid.len+svid1.len+rc.len+osid.len;
    l=10;
    allavp=new avp[l];
    allavp[0]=o;
    allavp[1]=realm;
    allavp[2]=vid;
    allavp[3]=pn;
    allavp[4]=ip;
    allavp[5]=authappid;
    allavp[6]=svid;
    allavp[7]=svid1;
    allavp[8]=rc;
    allavp[9]=osid;
}
diameter entry::createRAR(char* msid){
    printf("create RAR\n");
    char* h=new char[4];
    *h=0x01;
    avp* allavp=new avp[1];
    int l,total;
    getRAR(msid,allavp, l, total);
    if(l==1){
        return diameter(0,0,0);
    }
    int l_resp=20+total;
    char *ptr1 = (char*)&l_resp;
    char l_byte[3];
    char* lp=l_byte;
    ptr1=ptr1+2;
    int i=0;
    while(i<3){
        *lp=*ptr1;
        lp++;
        ptr1--;
        i++;
    }
    //printf(" lbyte %02X %02X %02X ",l_byte[0],l_byte[1],l_byte[2]);
    *(h+1)=l_byte[0];
    *(h+2)=l_byte[1];
    *(h+3)=l_byte[2];
    char *b=new char[l_resp-4];
    
    *b=0x80;
    //printf(" ccode %02X %02X %02X ",*d.ccode,*(d.ccode+1),*(d.ccode+2));
    *(b+1)=0x00;
    *(b+2)=0x01;
    *(b+3)=0x02;
    //printf(" copy ccode %02X %02X %02X \n",body[1],body[2],body[3]);
    //copy appid hbh e2e to body
    i=0;
    while (i<12) {
        *(b+i+4)=0x00;
        i++;
    }
    b=b+16;
    for (i=0; i<l; i++) {
        //copy avp
        char *temp=allavp[i].val;
        //allavp[i].dump();
        //printf("\n");
        for (int j=0; j<allavp[i].len; j++) {
            *b=*temp;
            b++;
            temp++;
        }
    }
    b=b-l_resp+4;
    diameter answer=diameter(h,b,l_resp-4);
    //answer.dump();
    return answer;
}
diameter entry::process(diameter d){
    d.populateHeader();
    int reqbit=(0x80&d.cflags);
    if(reqbit==0){
        return diameter(0, 0, 0);
    }
    //printf("reqbit : %i\n",reqbit);
    int ccode=((*(d.ccode) & 0xff) << 16)| ((*(d.ccode+1) & 0xff) << 8) | ((*(d.ccode+2)& 0xff));
   int i=0;
    logic lojik=logic();
    lojik.db=db;
    avp* allavp=new avp[1];
    int l;
    int total;
    std::string host="";
    //CALL LOGIC HERE
    //lojik.getResult(d, allavp, l,total);
    if (ccode==257) {
        getCEA(d, allavp, l, total,host);
        //if cea success, add sock peer to list
        test(host);
    }else if(ccode==272){//ccr
        lojik.getCCA(d, allavp, l,total);
    }else if(ccode==280){//watchdog
        getDWA( allavp, l, total);
    }else{
        getUnable2Comply(d, allavp, l, total);
    }

    char* h=new char[4];
    *h=d.version;
    
    char cflags=d.cflags^0x80;
    //printf(" avp len %i",o.len);
    //int l_resp=o.len+20+sid.len;
    int l_resp=20+total;

    char *ptr1 = (char*)&l_resp;
    char l_byte[3];
    char* lp=l_byte;
    ptr1=ptr1+2;
    i=0;
    while(i<3){
         *lp=*ptr1;
         lp++;
         ptr1--;
         i++;
    }
    //printf(" lbyte %02X %02X %02X ",l_byte[0],l_byte[1],l_byte[2]);
    *(h+1)=l_byte[0];
    *(h+2)=l_byte[1];
    *(h+3)=l_byte[2];
    //char* h=head;
    //printf(" msg len %i ",l_resp);
    char *b=new char[l_resp-4];
    
    *b=cflags;
    //printf(" ccode %02X %02X %02X ",*d.ccode,*(d.ccode+1),*(d.ccode+2));
    *(b+1)=*d.ccode;
    *(b+2)=*(d.ccode+1);
    *(b+3)=*(d.ccode+2);
    //printf(" copy ccode %02X %02X %02X \n",body[1],body[2],body[3]);
    //copy appid hbh e2e to body
    i=0;
    while (i<12) {
        *(b+i+4)=*d.appId;
        d.appId++;
        i++;
    }
    b=b+16;
    for (i=0; i<l; i++) {
        //copy avp
        char *temp=allavp[i].val;
        //allavp[i].dump();
        //printf("\n");
        for (int j=0; j<allavp[i].len; j++) {
            *b=*temp;
            b++;
            temp++;
        }
    }
    b=b-l_resp+4;

    diameter answer=diameter(h, b, l_resp-4);
    //answer.dump();
    
    return answer;
}

void entry::connectCallback(CallbackInterface *cb)
{
    entry::m_cb = cb;
}

// Test the callback to make sure it works.
void entry::test(std::string host)
{
    printf("Caller::test() calling callback...\n");
    entry::m_cb -> cbiCallbackFunction(host);
}
