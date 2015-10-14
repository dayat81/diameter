//
//  entry.cpp
//  diameter
//
//  Created by hidayat on 10/14/15.
//  Copyright © 2015 hidayat. All rights reserved.
//

#include <stdio.h>
#include "entry.h"
#include "avputil.h"
#include <iostream>
entry::entry(){
    
}

diameter entry::process(diameter d){
    diameter r=d;
    //get avp
    d.populateHeader();
    avputil util=avputil();
    avp oh = d.getAVP(264,0);
    printf("origin host len : %i\n",oh.len);
    if(oh.len!=0){
        std::cout<<"decoded : "<<util.decodeAsString(oh)<<std::endl;
    }
    avp sid=d.getAVP(443,0);
    printf("sid len : %i\n",sid.len);
    if(sid.len!=0){
        //getbyavp
        avp iddata_=util.getAVP(444, 0, sid);
        printf("iddata_ len : %i\n",iddata_.len);
        std::cout<<"decoded : "<<util.decodeAsString(iddata_)<<std::endl;
    }
    //d.dump();
    printf("\n");
    
    return r;
}