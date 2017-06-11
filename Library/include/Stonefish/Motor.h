//
//  Motor.h
//  Stonefish
//
//  Created by Patryk Cieslak on 15/09/2015.
//  Copyright (c) 2015-2017 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Motor__
#define __Stonefish_Motor__

#include "Actuator.h"
#include "RevoluteJoint.h"
#include "FeatherstoneEntity.h"

class Motor : public Actuator
{
public:
    Motor(std::string uniqueName, RevoluteJoint* revolute);
    Motor(std::string uniqueName, FeatherstoneEntity* mb, unsigned int child);
    
    virtual void Update(btScalar dt);
    virtual btVector3 Render();
    virtual void setIntensity(btScalar value);
	virtual btScalar getTorque();
	
	btScalar getAngularVelocity();
    ActuatorType getType();
    
protected:
	//states
    btScalar torque;
	
private:
    //output
    RevoluteJoint* revoluteOutput;
    FeatherstoneEntity* multibodyOutput;
    unsigned int multibodyChild;
};




#endif
