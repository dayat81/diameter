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

int avputil::decodeAsInt(avp a){
    int i=a.len-1;
    int res=(*a.val&0xff<<(8*i));
    a.val++;
    i--;
    while (i>=0) {
        res=res|(*a.val&0xff<<(8*i));
        i--;
        a.val++;
    }
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

char* return_buffer(const std::string& string)
{
    char* return_string = new char[string.length() + 1];
    strcpy(return_string, string.c_str());
    
    return return_string;
}

avp avputil::encodeString(int acode, int vcode, char flags, std::string value){
    int l=value.length()+8;
    if(vcode!=0){
        l=l+8;
    }
    //char res[l];
    char* resp=new char[l];//res;
    char* buffer = return_buffer(value);
    
    
    char *ptr = (char*)&acode;
    ptr=ptr+3;
    unsigned int i=0;
    while(i<4){
        *resp=*ptr;
        resp++;
        ptr--;
        i++;
    }
    *resp=flags;
    //resp=resp-4;
    resp++;
    //	 char *msg = new char[4];
    //	 for(int i=0;i<4;++i, ++ptr)
    //	    msg[3-i] = *ptr;
    //resp=resp-4;
    
    char *ptr1 = (char*)&l;
    ptr1=ptr1+2;
    i=0;
    while(i<3){
        *resp=*ptr1;
        resp++;
        ptr1--;
        i++;
    }
    //	 resp=resp-8;	//for display
    i=0;
    while(i<value.length()){
        *resp=*buffer;
        resp++;
        buffer++;
        i++;
    }
    resp=resp-l;
    //	 char *msg1 = new char[4];
    //	 for(int i=0;i<3;++i, ++ptr1)
    //	    msg1[2-i] = *ptr1;
    
    avp a=avp(resp,l);
    return a;
}

avp avputil::encodeInt32(int acode, int vcode, char flags, int value){
    
    int l=12;
    if(vcode!=0){
        l=l+8;
    }
    //char res[l];
    char* resp=new char[l];//res;
    
    
    
    char *ptr = (char*)&acode;
    ptr=ptr+3;
    unsigned int i=0;
    while(i<4){
        *resp=*ptr;
        resp++;
        ptr--;
        i++;
    }
    *resp=flags;
    //resp=resp-4;
    resp++;
    //	 char *msg = new char[4];
    //	 for(int i=0;i<4;++i, ++ptr)
    //	    msg[3-i] = *ptr;
    //resp=resp-4;
    
    char *ptr1 = (char*)&l;
    ptr1=ptr1+2;
    i=0;
    while(i<3){
        *resp=*ptr1;
        resp++;
        ptr1--;
        i++;
    }
    //	 resp=resp-8;	//for display
    
    char bytes[4];
    //
    bytes[0] = (value >> 24) & 0xFF;
    bytes[1] = (value >> 16) & 0xFF;
    bytes[2] = (value >> 8) & 0xFF;
    bytes[3] = value & 0xFF;
    char* b=bytes;
    i=0;
    while(i<4){
        *resp=*b;
        resp++;
        b++;
        i++;
    }
    resp=resp-l;
    //	 char *msg1 = new char[4];
    //	 for(int i=0;i<3;++i, ++ptr1)
    //	    msg1[2-i] = *ptr1;
    
    avp a=avp(resp,l);
    return a;
    

//    avp a=avp(b,4);
//    
//    return a;
}
//
avp avputil::encodeAVP(int acode, int vcode,char flags, avp* list[],int l){
    //avp a=avp(0,0);
    int totallen=8;
    for (int i=0; i<l; i++) {
        //count total length
        totallen=totallen+list[i]->len;
    }
    if(vcode!=0){
        totallen=totallen+8;
    }
    char* resp=new char[totallen];//res;
    char *ptr = (char*)&acode;
    ptr=ptr+3;
    int i=0;
    while(i<4){
        *resp=*ptr;
        resp++;
        ptr--;
        i++;
    }
    *resp=flags;
    //resp=resp-4;
    resp++;
    //	 char *msg = new char[4];
    //	 for(int i=0;i<4;++i, ++ptr)
    //	    msg[3-i] = *ptr;
    //resp=resp-4;
    
    char *ptr1 = (char*)&totallen;
    ptr1=ptr1+2;
    i=0;
    while(i<3){
        *resp=*ptr1;
        resp++;
        ptr1--;
        i++;
    }
    //copy avps to rest of bytes
    for (i=0; i<l; i++) {
        //copy avp
        char *temp=list[i]->val;
        //list[i]->dump();
        //printf("\n");
        for (int j=0; j<list[i]->len; j++) {
            *resp=*temp;
            resp++;
            temp++;
        }
    }
    resp=resp-totallen;
    avp a=avp(resp,totallen);
    //printf("\n");
    //a.dump();
    return a;
}