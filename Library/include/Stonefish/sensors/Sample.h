//
//  Sample.h
//  Stonefish
//
//  Created by Patryk Cieslak on 23/03/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Sample__
#define __Stonefish_Sample__

#include "common.h"

class Sample
{
public:
    Sample(unsigned short nDimensions, btScalar* values);
    Sample(const Sample& other);
    ~Sample();
    
    btScalar getTimestamp();
    btScalar getValue(unsigned short dimension);
    std::vector<btScalar> getData();
    
    btScalar timestamp;
    unsigned short nDim;
    btScalar* data;
};

#endif
