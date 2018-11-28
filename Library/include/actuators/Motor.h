//
//  Motor.h
//  Stonefish
//
//  Created by Patryk Cieslak on 15/09/2015.
//  Copyright (c) 2015-2017 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Motor__
#define __Stonefish_Motor__

#include "actuators/JointActuator.h"

namespace sf
{

class Motor : public JointActuator
{
public:
    Motor(std::string uniqueName);
    
    virtual void Update(Scalar dt);
    virtual void setIntensity(Scalar value);
	virtual Scalar getTorque();
    virtual Scalar getAngle();
	virtual Scalar getAngularVelocity();
    
protected:
	Scalar torque;
};

}

#endif
