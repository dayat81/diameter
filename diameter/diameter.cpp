//
//  diameter.cpp
//  diameter
//
//  Created by hidayat on 10/14/15.
//  Copyright © 2015 hidayat. All rights reserved.
//

#include "diameter.h"
#include <stdio.h>

diameter::diameter(char* h,char* b,int l){   //create empty
    diameter::h=h;
    diameter::b=b;
    diameter::len=l;
}

void diameter::compose(char* r){
    int i=0;
    while (i<4) {
        *r=*h;
        r++;
        h++;
        i++;
    }
    i=0;
    while (i<len) {
        *r=*b;
        b++;
        i++;
        r++;
    }
}

void diameter::dump(){
    int i=0;
    while (i<4) {
        printf("%02X ",*h&0xff);
        i++;
        h++;
    }
    h=h-4;
    i=0;
    while (i<len) {
        printf("%02X ",*b&0xff);
        i++;
        b++;
    }
    b=b-len;
}