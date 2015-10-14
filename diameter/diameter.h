//
//  diameter.h
//  diameter
//
//  Created by hidayat on 10/14/15.
//  Copyright © 2015 hidayat. All rights reserved.
//

#ifndef diameter_h
#define diameter_h

class diameter{
public:
    char* h;
    char* b;
    int len;
    int curr;
    
    diameter(char* h,char* b,int l);
    void compose(char* res);
    void dump();
};

#endif /* diameter_h */
