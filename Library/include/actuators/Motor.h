//
//  Motor.h
//  Stonefish
//
//  Created by Patryk Cieslak on 15/09/2015.
//  Copyright (c) 2015-2017 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Motor__
#define __Stonefish_Motor__

#include "entities/FeatherstoneEntity.h"
#include "joints/RevoluteJoint.h"
#include "actuators/Actuator.h"

class Motor : public Actuator
{
public:
    Motor(std::string uniqueName);
    
    void AttachToJoint(RevoluteJoint* revolute);
    void AttachToJoint(FeatherstoneEntity* mb, unsigned int jointId);
    
    virtual void Update(btScalar dt);
    virtual void setIntensity(btScalar value);
	virtual btScalar getTorque();
    virtual btScalar getAngle();
	virtual btScalar getAngularVelocity();
    ActuatorType getType();
    
protected:
	//states
    btScalar torque;
	
private:
    //output
    RevoluteJoint* revoluteOutput;
    FeatherstoneEntity* multibodyOutput;
    unsigned int multibodyJoint;
};




#endif
