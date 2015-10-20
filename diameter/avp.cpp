//
//  avp.cpp
//  diameter
//
//  Created by hidayat on 10/14/15.
//  Copyright © 2015 hidayat. All rights reserved.
//

#include "avp.h"
#include <stdio.h>
avp::avp(char* v,int len){
    avp::len=len;
    
    avp::val=v;
}
avp::avp(){
    
}

void avp::dump(){
    int i=0;
    while (i<len) {
        printf("%02X ",*val);
        val++;
        i++;
    }
    val=val-len;
}

char* avp::copy(){
    char* res=new char[len];
    int i=0;
    while (i<len) {
        //printf("%02X ",*val);
        *res=*val;
        res++;
        val++;
        i++;
    }
    val=val-len;
    res=res-len;
    
    return res;
}