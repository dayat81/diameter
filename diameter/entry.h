//
//  entry.h
//  diameter
//
//  Created by hidayat on 10/14/15.
//  Copyright © 2015 hidayat. All rights reserved.
//

#ifndef entry_h
#define entry_h

#include "diameter.h"
class CallbackInterface
{
public:
    // The prefix "cbi" is to prevent naming clashes.
    virtual int cbiCallbackFunction(int,int) = 0;
};
class entry{
private:
    // The callback provided by the client via connectCallback().
    CallbackInterface *m_cb;
public:
    entry(int sock);
    int sock;
    diameter process(diameter d);
    void connectCallback(CallbackInterface *cb);
    void test();
};

#endif /* entry_h */ 
