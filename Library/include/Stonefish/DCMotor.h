//
//  DCMotor.h
//  Stonefish
//
//  Created by Patryk Cieslak on 1/11/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_DCMotor__
#define __Stonefish_DCMotor__

#include "Actuator.h"
#include "RevoluteJoint.h"
#include "FeatherstoneEntity.h"

class DCMotor : public Actuator
{
public:
    DCMotor(std::string uniqueName, RevoluteJoint* revolute, btScalar motorR, btScalar motorL, btScalar motorKe, btScalar motorKt, btScalar friction);
    DCMotor(std::string uniqueName, FeatherstoneEntity* mb, unsigned int child, btScalar motorR, btScalar motorL, btScalar motorKe, btScalar motorKt, btScalar friction);
    
    void Update(btScalar dt);
    btVector3 Render();
    void SetupGearbox(bool enable, btScalar ratio, btScalar efficiency);
    
    void setVoltage(btScalar volt);
    btScalar getVoltage();
    btScalar getTorque();
    btScalar getCurrent();
    ActuatorType getType();
    
    btScalar getKe();
    btScalar getKt();
    btScalar getGearRatio();
    
private:
    btScalar getAngularVelocity();
    
    //output
    RevoluteJoint* revoluteOutput;
    FeatherstoneEntity* multibodyOutput;
    unsigned int multibodyChild;
    
    //inputs
    btScalar V;
    
    //states
    btScalar I;
    btScalar torque;
    
    //motor params
    btScalar R;
    btScalar L;
    btScalar Ke;
    btScalar Kt;
    btScalar B;
    bool gearEnabled;
    btScalar gearRatio;
    btScalar gearEff;
    
    //integral
    btScalar lastVoverL;
};



#endif
