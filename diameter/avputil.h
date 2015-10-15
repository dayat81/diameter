//
//  avputil.h
//  diameter
//
//  Created by hidayat on 10/14/15.
//  Copyright © 2015 hidayat. All rights reserved.
//

#ifndef avputil_h
#define avputil_h

#include "avp.h"
#include <string>

class avputil{
public:
    avputil();
    
    std::string decodeAsString(avp a);
    int decodeAsInt(avp a);
    
    avp getAVP(int acode,int vcode,avp a);
    
    avp encodeString(int acode,int vcode,char flags,std::string value);
    avp encodeInt32(int acode,int vcode,char flags,int value);
    //avp encodeAVP(int acode,int vcode,avp list[]);
};

#endif /* avputil_h */
