//
//  SignalMux.h
//  Stonefish
//
//  Created by Patryk Cieslak on 12/07/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_SignalMux__
#define __Stonefish_SignalMux__

#include "controllers/SignalGenerator.h"

/*! Signal multiplexer - combines signal generators */
class SignalMux
{
public:
    SignalMux();
    virtual ~SignalMux();
    
    void AddSignalGenerator(SignalGenerator* sg);
    std::vector<btScalar> ValuesAtTime(btScalar t);
    unsigned int getNumOfSignals();
    
private:
    std::vector<SignalGenerator*> signalGens;
};

#endif
