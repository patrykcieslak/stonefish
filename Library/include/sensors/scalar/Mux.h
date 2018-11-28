//
//  Mux.h
//  Stonefish
//
//  Created by Patryk Cieslak on 24/05/2014.
//  Copyright (c) 2014-2018 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Mux__
#define __Stonefish_Mux__

#include "sensors/ScalarSensor.h"

namespace sf
{

typedef struct
{
    ScalarSensor* sensor;
    unsigned short channel;
}
MuxComponent;

class Mux
{
public:
    Mux();
    ~Mux();
    
    bool AddComponent(ScalarSensor* s, unsigned short channel);
    
    MuxComponent* getComponent(unsigned int index);
    Scalar* getLastSample();
    unsigned int getNumOfComponents();
    
private:
    std::vector<MuxComponent> components;
};
    
}

#endif
