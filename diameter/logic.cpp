//
//  logic.cpp
//  diameter
//
//  Created by hidayat on 10/15/15.
//  Copyright © 2015 hidayat. All rights reserved.
//

#include <stdio.h>
#include "logic.h"
#include "avputil.h"

logic::logic(){
    
}

void logic::getResult(diameter d,avp* &allavp,int &l,int &total){
    avputil util=avputil();
    char f=0x40;
    std::string ori ="vmclient.myrealm.example";
    //printf("size : %i\n",ori.size());
    avp o=util.encodeString(264,0,f,ori);
    o.dump();
    printf("\n");
    avp id_t=util.encodeInt32(450, 0, 0x40, 1);
    //id_t.dump();
    //printf("\n");
    avp id_d=util.encodeString(444, 0, 0x40, "628119105569");
    //id_d.dump();
    avp* listavp[2]={&id_t,&id_d};
    
    //int i=0;
    
    avp sid=util.encodeAVP(443, 0, 0x40, listavp, 2);
    sid.dump();
    printf("\n");
    total=sid.len+o.len;
    l=2;
    allavp=new avp[2];
    allavp[0]=o;
    allavp[1]=sid;
}