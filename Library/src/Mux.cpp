//
//  Mux.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 24/05/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#include <sensors/Mux.h>

Mux::Mux()
{
}

Mux::~Mux()
{
    components.clear();
}

bool Mux::AddComponent(SimpleSensor* s, unsigned short channel)
{
    if(s == NULL)
        return false;
    
    if(channel >= s->getNumOfChannels())
        return false;
    
    MuxComponent cmp;
    cmp.sensor = s;
    cmp.channel = channel;
    components.push_back(cmp);
    return true;
}

MuxComponent* Mux::getComponent(unsigned int index)
{
    if(index < components.size())
        return &components[index];
    
    return NULL;
}

btScalar* Mux::getLastSample()
{
    btScalar* sample = new btScalar[components.size()];
    for(unsigned int i = 0; i < components.size(); ++i)
        sample[i] = components[i].sensor->getLastSample().getValue(components[i].channel);
    return sample;
}

unsigned int Mux::getNumOfComponents()
{
    return (unsigned int)components.size();
}