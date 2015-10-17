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
entry::entry(){
}

diameter entry::process(diameter d){
    
    //entry screening header : command code, appId
    //avp screening/processing to logic.cpp
    
    d.populateHeader();
    int ccode=((*(d.ccode) & 0xff) << 16)| ((*(d.ccode+1) & 0xff) << 8) | ((*(d.ccode+2)& 0xff));
    //printf("printf ccode : %i\n",ccode);
    //cek header here 

//    
   int i=0;
//
//    avp sid=util.encodeAVP(443, 0, 0x40, listavp, 2);
//    
//    avp* allavp[2]={&o,&sid};
    logic lojik=logic();
    avp* allavp=new avp[1];
    int l;
    int total;
    std::string host="";
    //CALL LOGIC HERE
    //lojik.getResult(d, allavp, l,total);
    if (ccode==257) {
        lojik.getCEA(d, allavp, l, total,host);
        //if cea success, add sock peer to list
//        time_t rawtime;
//        struct tm * timeinfo;
//        char buffer[80];
//        
//        time (&rawtime);
//        timeinfo = localtime(&rawtime);
//        
//        strftime(buffer,80,"%d-%m-%Y %I:%M:%S",timeinfo);
//        std::string str(buffer);
        test(host);
    }else{
        lojik.getUnable2Comply(d, allavp, l, total);
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
