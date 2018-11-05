//
//  Mux.h
//  Stonefish
//
//  Created by Patryk Cieslak on 24/05/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Mux__
#define __Stonefish_Mux__

#include "SimpleSensor.h"

typedef struct
{
    SimpleSensor* sensor;
    unsigned short channel;
}
MuxComponent;

class Mux
{
public:
    Mux();
    ~Mux();
    
    bool AddComponent(SimpleSensor* s, unsigned short channel);
    
    MuxComponent* getComponent(unsigned int index);
    btScalar* getLastSample();
    unsigned int getNumOfComponents();
    
private:
    std::vector<MuxComponent> components;
};

#endif
