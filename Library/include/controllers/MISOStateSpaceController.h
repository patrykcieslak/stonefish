//
//  MISOStateSpaceController.h
//  Stonefish
//
//  Created by Patryk Cieslak on 24/05/2014.
//  Copyright (c) 2014-2018 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_MISOStateSpaceController__
#define __Stonefish_MISOStateSpaceController__

#include "controllers/FeedbackController.h"

namespace sf
{
    class Mux;
    class DCMotor;
    
    //! MISO controller for controlling SIMO systems.
    class MISOStateSpaceController : public FeedbackController
    {
    public:
        MISOStateSpaceController(std::string uniqueName, Mux* inputs, DCMotor* output, Scalar maxOutput, Scalar frequency = Scalar(-1));
        ~MISOStateSpaceController();
        
        void Reset();
        
        void SetGains(const std::vector<Scalar>& g);
        
    private:
        void Tick(Scalar dt);
        
        Mux* input;
        DCMotor* output;
        Scalar maxOutput;
        std::vector<Scalar> gains;
    };
}

#endif
