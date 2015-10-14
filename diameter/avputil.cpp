//
//  avputil.cpp
//  diameter
//
//  Created by hidayat on 10/14/15.
//  Copyright © 2015 hidayat. All rights reserved.
//

#include <stdio.h>
#include "avputil.h"

avputil::avputil(){
    //helper class
}

std::string avputil::decodeAsString(avp a){
    char res[a.len];
    int x=0;
    while(x<a.len){
        res[x]=*a.val;
        x++;
        a.val++;
    }
    res[a.len]='\0';
    return res;
}