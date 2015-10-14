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
};

#endif /* avputil_h */
