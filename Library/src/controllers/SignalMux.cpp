//
//  SignalMux.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 12/07/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#include "controllers/SignalMux.h"

SignalMux::SignalMux()
{
}

SignalMux::~SignalMux()
{
    for(unsigned int i = 0; i < signalGens.size(); ++i)
        delete signalGens[i];
    
    signalGens.clear();
}

void SignalMux::AddSignalGenerator(SignalGenerator* sg)
{
    if(sg != NULL)
        signalGens.push_back(sg);
}

std::vector<btScalar> SignalMux::ValuesAtTime(btScalar t)
{
    std::vector<btScalar> values(signalGens.size());
    
    for(unsigned int i = 0; i < signalGens.size(); ++i)
        values[i] = signalGens[i]->ValueAtTime(t);
    
    return values;
}

unsigned int SignalMux::getNumOfSignals()
{
    return (unsigned int)signalGens.size();
}
