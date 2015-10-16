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
#include <functional>

class entry{
public:
    entry();
    diameter process(diameter d);
    
    // call function with one extrar (int by value) last parameter
    template < typename FN, typename... ARGS >
    void mylibfun_add_tail( int a, int b, FN&& fn, ARGS&&... args )
    {
        if( a<b )
        {
            const int extra_param = a + b ;
            // call function with an additional int as the last argument
            std::bind( std::forward<FN>(fn), std::forward<ARGS>(args)..., extra_param )() ;
        }
    }
};

#endif /* entry_h */ 
