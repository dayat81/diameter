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
#define ORIGIN_HOST    "redir.sapctest.lte.xl.co.id"
#define ORIGIN_REALM   "xltest.id"
class logic{
public:
    rocksdb::DB* db;
    logic();
    void getResult(diameter d,avp* &allavp,int &l,int &total);
    void getCCA(diameter d,avp* &allavp,int &l,int &total);
};

#endif /* logic_h */ 
