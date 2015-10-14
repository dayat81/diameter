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

avp avputil::getAVP(int acode, int vcode, avp a){
    int cc=0;
    char* p=a.val;
//    int j=0;
//    while (j<4) {
//        printf("%02X ",*p&0xff);
//        p++;
//        j++;
//    }
//    p=p-4;
    avp r=avp(0,0);
    bool found=false;
    int avpcode;
    int vendorbit;
    int lavpval;
    int padding;
    int vendorcode=0;
    while (cc<a.len&&!found) {
        avpcode=(((*p& 0xff) << 24) | ((*(p+1) & 0xff) << 16)| ((*(p+2) & 0xff) << 8) | ((*(p+3)& 0xff)));
        cc=cc+4;
        p=p+4;
        vendorbit=(0x80&*p);
        cc=cc+1;
        p=p+1;
        lavpval=(((0x00 & 0xff) << 24) | ((*p & 0xff) << 16)| ((*(p+1) & 0xff) << 8) | ((*(p+2) & 0xff)))-8;
        //8=avp header length
        cc=cc+3;
        p=p+3;
        if(vendorbit!=0){
            vendorcode=(((*p& 0xff) << 24) | ((*(p+1) & 0xff) << 16)| ((*(p+2) & 0xff) << 8) | ((*(p+3) & 0xff)));
            cc=cc+4;
            p=p+4;
            lavpval=lavpval-4;  //actual avp value length
        }
        if(acode==avpcode&&vcode==vendorcode){
            r=avp(p,lavpval);   //point to avp val
            found=true;
        }
        //goto next avp
        p=p+lavpval;
        cc=cc+lavpval;
        padding=lavpval%4;
        if(padding!=0){
            padding=4-padding;
        }
        p=p+padding;
        cc=cc+padding;
    }
    return r;
}