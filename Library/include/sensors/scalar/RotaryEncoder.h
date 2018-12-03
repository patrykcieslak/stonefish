//
//  RotaryEncoder.h
//  Stonefish
//
//  Created by Patryk Cieslak on 05/07/2014.
//  Copyright (c) 2014-2018 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_RotaryEncoder__
#define __Stonefish_RotaryEncoder__

#include "sensors/scalar/JointSensor.h"

namespace sf
{
    class Motor;
    class Thruster;
    
    //!
    class RotaryEncoder : public JointSensor
    {
    public:
        RotaryEncoder(std::string uniqueName, Scalar frequency = Scalar(-1), int historyLength = -1);
        
        void AttachToMotor(Motor* m);
        void AttachToThruster(Thruster* th);
        
        virtual void InternalUpdate(Scalar dt);
        virtual void Reset();
        
    protected:
        Scalar GetRawAngle();
        Scalar GetRawAngularVelocity();
        
        Motor* motor;
        Thruster* thrust;
        Scalar angle;
        Scalar lastAngle;
    };
}

#endif
