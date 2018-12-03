//
//  SignalMux.h
//  Stonefish
//
//  Created by Patryk Cieslak on 12/07/2014.
//  Copyright (c) 2014-2018 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_SignalMux__
#define __Stonefish_SignalMux__

#include "StonefishCommon.h"

namespace sf
{
    class SignalGenerator;
    
    //! Signal multiplexer - combines signal generators.
    class SignalMux
    {
    public:
        SignalMux();
        virtual ~SignalMux();
        
        void AddSignalGenerator(SignalGenerator* sg);
        std::vector<Scalar> ValuesAtTime(Scalar t);
        unsigned int getNumOfSignals();
        
    private:
        std::vector<SignalGenerator*> signalGens;
    };
}
    
#endif
