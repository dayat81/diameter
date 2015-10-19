//
//  logic.cpp
//  diameter
//
//  Created by hidayat on 10/15/15.
//  Copyright © 2015 hidayat. All rights reserved.
//

#include <stdio.h>
#include <iostream>
#include "logic.h"
#include "avputil.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

using namespace rapidjson;
logic::logic(){
    //
}

void logic::getResult(diameter d,avp* &allavp,int &l,int &total){
    avputil util=avputil();
    
    //read avp
    avp ori_host=d.getAVP(264, 0);
    //printf("ori len %i \n",ori_host.len);
    if(ori_host.len>0){
        std::cout<<util.decodeAsString(ori_host)<<std::endl;
    }
    
    char f=0x40;
    std::string ori ="vmclient.myrealm.example";
    //printf("size : %i\n",ori.size());
    avp o=util.encodeString(264,0,f,ori);
    //o.dump();
    //printf("\n");
    avp id_t=util.encodeInt32(450, 0, 0x40, 1);
    //id_t.dump();
    //printf("\n");
    avp id_d=util.encodeString(444, 0, 0x40, "628119105569");
    //id_d.dump();
    avp* listavp[2]={&id_t,&id_d};
    avp sid=util.encodeAVP(443, 0, 0x40, listavp, 2);
    
    avp id_t1=util.encodeInt32(450, 0, 0x40, 0);
    avp id_d1=util.encodeString(444, 0, 0x40, "51010628119105569");
    avp* listavp1[2]={&id_t1,&id_d1};
    avp sid1=util.encodeAVP(443, 0, 0x40, listavp1, 2);
    
    //sid.dump();
    //printf("\n");
    total=sid.len+o.len+sid1.len;
    l=3;
    allavp=new avp[l];
    allavp[0]=o;
    allavp[1]=sid;
    allavp[2]=sid1;
}

void logic::getCCA(diameter d,avp* &allavp,int &l,int &total){
    avputil util=avputil();
    avp cca_sessid=d.copyAVP(263, 0);
    std::string sessidval="";
    avp sessid=d.getAVP(263, 0);
    if(sessid.len>0){
        sessidval=util.decodeAsString(sessid);
    }
    avp cca_req_type=d.copyAVP(416, 0);
    int req_type=0;
    avp reqtype=d.getAVP(416, 0);
    if (reqtype.len>0) {
        req_type=util.decodeAsInt(reqtype);
    }
    avp cca_req_num=d.copyAVP(415, 0);
    if(req_type==1){
        //read avp msid
        bool exit=false;
        std::string msidstring="";
        while(!exit){
            avp msid=d.getAVP(443, 0);
            //printf("msid len %i \n",msid.len);
            if(msid.len>0){
                avp msidtype=util.getAVP(450, 0, msid);
                //printf("msidtype len %i \n",msidtype.len);
                if(msidtype.len>0){
                    int type=util.decodeAsInt(msidtype);
                    //printf("decoded : %i\n",type);
                    if(type==0){
                        exit=true;
                        avp msiddata=util.getAVP(444, 0, msid);
                        msidstring=util.decodeAsString(msiddata);
                    }
                }
            }else{//avp not found
                exit=true;
            }
        }
        std::cout<<msidstring<<std::endl;
        std::string msidsesinfo=msidstring;
        std::string msidrarinfo=msidstring;
        //store sessid,msid
        
        rocksdb::Status status = db->Put(rocksdb::WriteOptions(), sessidval, msidstring);
        status = db->Put(rocksdb::WriteOptions(), msidsesinfo.append("_sess"), sessidval);
        status = db->Put(rocksdb::WriteOptions(), msidrarinfo.append("_rarinfo"), "{\"addacg\":[],\"delacg\":[]}");
        std::string val;
        status = db->Get(rocksdb::ReadOptions(), msidstring, &val);
        
        std::cout<<val<<std::endl;
        Document dom;
        dom.Parse(val.c_str());
        const Value& a = dom["acg"];
        assert(a.IsArray());
        avp cr_install=avp(0,0);
        if(a.Size()>0){
            avp* acg=new avp[a.Size()];
            for (SizeType i = 0; i < a.Size(); i++){ // Uses SizeType instead of size_t
                //printf("a[%d] = %s\n", i, a[i].GetString());   //map to charging-rule-name-avp
                avp temp=util.encodeString(1004, 10415, 0xC0, a[i].GetString());
                temp.dump();
                //printf("\n");
                *acg=temp;
                acg++;
            }
            acg=acg-a.Size();
            cr_install=util.encodeAVP(1001, 10415, 0xC0, acg, a.Size());
        }
        char f=0x40;
        avp o=util.encodeString(264,0,f,ORIGIN_HOST);
        avp realm=util.encodeString(296,0,f,ORIGIN_REALM);
        avp authappid=util.encodeInt32(258, 0, f, 16777238);
        avp rc=util.encodeInt32(268, 0, f, 2001);
        avp flid=util.encodeInt32(629, 10415, 0xc0, 1);
//        printf("========\n");
//        flid.dump();
//        printf("\n");
        avp fl=util.encodeInt32(630, 10415, 0xc0, 3);
//        fl.dump();
//        printf("\n");
        avp vid=util.encodeInt32(266, 0, f, 10415);
//        vid.dump();
//        printf("\n");
        avp* list_fl[3]={&vid,&flid,&fl};
        avp sf=util.encodeAVP(628, 10415, 0xc0, list_fl, 3);
//        sf.dump();
//        printf("======\n");
        total=cca_sessid.len+o.len+realm.len+cca_req_type.len+cca_req_num.len+authappid.len+rc.len+sf.len;
        l=8;
        if(cr_install.len>0){
            total=total+cr_install.len;
            l++;
        }
        allavp=new avp[l];
        allavp[0]=cca_sessid;
        allavp[1]=o;
        allavp[2]=realm;
        allavp[3]=cca_req_type;
        allavp[4]=cca_req_num;
        allavp[5]=authappid;
        allavp[6]=rc;
        allavp[7]=sf;
        if(cr_install.len>0){
            allavp[l-1]=cr_install;
        }
    }
}


