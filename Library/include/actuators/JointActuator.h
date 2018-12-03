//
//  JointActuator.h
//  Stonefish
//
//  Created by Patryk Cieślak on 23/11/2018.
//  Copyright (c) 2018 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_JointActuator__
#define __Stonefish_JointActuator__

#include "actuators/Actuator.h"

namespace sf
{
    class FeatherstoneEntity;
    class Joint;
    
    //!
    class JointActuator : public Actuator
    {
    public:
        JointActuator(std::string uniqueName);
        
        void AttachToJoint(FeatherstoneEntity* multibody, unsigned int jointId);
        void AttachToJoint(Joint* joint);
        ActuatorType getType();
        
    protected:
        FeatherstoneEntity* fe;
        unsigned int jId;
        Joint* j;
    };
}

#endif
