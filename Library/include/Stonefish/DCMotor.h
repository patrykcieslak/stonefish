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

class DCMotor : public Actuator
{
public:
    DCMotor(RevoluteJoint* revolute, btScalar motorR, btScalar motorL, btScalar motorKe, btScalar motorKm, btScalar friction, btScalar motorEfficiency);
    ~DCMotor();
    
    ActuatorType getType();
    void Render();
    void SetInput(btScalar* inputValues);
    void Update(btScalar dt);
    
    void SetGearbox(bool enable, btScalar ratio, btScalar efficiency);
    
private:
    //output
    RevoluteJoint* output;
    
    //inputs
    btScalar V;
    
    //states
    btScalar I;
    
    //motor params
    btScalar R;
    btScalar L;
    btScalar Ke;
    btScalar Km;
    btScalar B;
    btScalar eff;
    bool gearEnabled;
    btScalar gearRatio;
    btScalar gearEff;
};



#endif
