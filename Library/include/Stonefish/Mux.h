//
//  Mux.h
//  Stonefish
//
//  Created by Patryk Cieslak on 24/05/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Mux__
#define __Stonefish_Mux__

#include "Sensor.h"

typedef struct
{
    Sensor* sensor;
    unsigned short dim;
}
MuxComponent;

class Mux
{
public:
    Mux();
    ~Mux();
    
    bool AddComponent(Sensor* s, unsigned short dim);
    
    MuxComponent* getComponent(unsigned int index);
    btScalar* getLastSample();
    unsigned int getNumOfComponents();
    
private:
    std::vector<MuxComponent> components;
};

#endif
