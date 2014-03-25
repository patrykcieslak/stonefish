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
    Sample(ushort nDimensions, btScalar* values);
    Sample(const Sample& other);
    ~Sample();
    
    btScalar getTimestamp();
    btScalar getValue(ushort dimension);
    std::shared_ptr<std::vector<btScalar>> getData();
    
private:
    btScalar timestamp;
    ushort nDim;
    btScalar* data;
};

#endif
