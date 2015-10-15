//
//  logic.h
//  diameter
//
//  Created by hidayat on 10/15/15.
//  Copyright © 2015 hidayat. All rights reserved.
//

#ifndef logic_h
#define logic_h
#include "diameter.h"

class logic{
public:
    logic();
    void getResult(diameter d,avp* &allavp,int &l,int &total);
};

#endif /* logic_h */
