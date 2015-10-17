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
#include "rocksdb/db.h"

class logic{
public:
    rocksdb::DB* db;
    logic();
    void getResult(diameter d,avp* &allavp,int &l,int &total);
    void getCEA(diameter d,avp* &allavp,int &l,int &total,std::string &host);
    void getCCA(diameter d,avp* &allavp,int &l,int &total);
    void getUnable2Comply(diameter d,avp* &allavp,int &l,int &total);
};

#endif /* logic_h */ 
