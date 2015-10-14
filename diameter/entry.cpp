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
#include <iostream>
entry::entry(){
    
}

diameter entry::process(diameter d){
    //diameter r=d;
    
    avputil util=avputil();
    char f=0x40;
    std::string ori ="vmclient.myrealm.example";
    //printf("size : %i\n",ori.size());
    avp o=util.encodeString(264,0,f,ori);
    //o.dump();
    d.populateHeader();
    char head[4];
    head[0]=d.version;
    
    char cflags=d.cflags^0x80;
    //printf(" avp len %i",o.len);
    int l_resp=o.len+20;
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
    head[1]=l_byte[0];
    head[2]=l_byte[1];
    head[3]=l_byte[2];
    char* h=head;
    //printf(" msg len %i ",l_resp);
    char body[l_resp-4];
    
    body[0]=cflags;
    //printf(" ccode %02X %02X %02X ",*d.ccode,*(d.ccode+1),*(d.ccode+2));
    body[1]=*d.ccode;
    body[2]=*(d.ccode+1);
    body[3]=*(d.ccode+2);
    //printf(" copy ccode %02X %02X %02X \n",body[1],body[2],body[3]);
    //copy appid hbh e2e to body
    i=0;
    while (i<12) {
        body[i+4]=*d.appId;
        d.appId++;
        i++;
    }
    //copy avp to body
    i=0;
    while(i<o.len){
        body[i+16]=*o.val;
        i++;
        o.val++;
    }
    char* b=body;
    diameter answer=diameter(h, b, l_resp-4);
    //answer.dump();
    //printf(" cflags %02X ",cflags);
    //printf(" ccode %02X %02X %02X ",*d.ccode,*(d.ccode+1),*(d.ccode+2));
    
    //get avp
//    
//    
//    avp oh = d.getAVP(264,0);
//    printf("origin host len : %i\n",oh.len);
//    if(oh.len!=0){
//        std::cout<<"decoded : "<<util.decodeAsString(oh)<<std::endl;
//    }
//    avp sid=d.getAVP(443,0);
//    printf("sid len : %i\n",sid.len);
//    if(sid.len!=0){
//        //getbyavp
//        avp iddata_=util.getAVP(444, 0, sid);
//        printf("iddata_ len : %i\n",iddata_.len);
//        std::cout<<"decoded : "<<util.decodeAsString(iddata_)<<std::endl;
//    }
    //d.dump();
    printf("\n");
    
    return answer;
}