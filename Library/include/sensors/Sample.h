//
//  Sample.h
//  Stonefish
//
//  Created by Patryk Cieslak on 23/03/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Sample__
#define __Stonefish_Sample__

#include "StonefishCommon.h"

namespace sf
{

class Sample
{
public:
    Sample(unsigned short nDimensions, Scalar* values);
    Sample(const Sample& other);
    ~Sample();
    
    Scalar getTimestamp();
    Scalar getValue(unsigned short dimension);
    std::vector<Scalar> getData();
    
    Scalar timestamp;
    unsigned short nDim;
    Scalar* data;
};
    
}

#endif
