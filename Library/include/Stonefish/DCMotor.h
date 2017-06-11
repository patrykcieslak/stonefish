//
//  DCMotor.h
//  Stonefish
//
//  Created by Patryk Cieslak on 1/11/13.
//  Copyright (c) 2013-2017 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_DCMotor__
#define __Stonefish_DCMotor__

#include "Motor.h"

class DCMotor : public Motor
{
public:
    DCMotor(std::string uniqueName, RevoluteJoint* revolute, btScalar motorR, btScalar motorL, btScalar motorKe, btScalar motorKt, btScalar friction);
    DCMotor(std::string uniqueName, FeatherstoneEntity* mb, unsigned int child, btScalar motorR, btScalar motorL, btScalar motorKe, btScalar motorKt, btScalar friction);
    
	//Motor
    void Update(btScalar dt);
    btVector3 Render();
	void setIntensity(btScalar value);
    btScalar getTorque();
    
	//DC motor
	void SetupGearbox(bool enable, btScalar ratio, btScalar efficiency);
	void setVoltage(btScalar volt);
	btScalar getCurrent();
	btScalar getVoltage();
    btScalar getKe();
    btScalar getKt();
    btScalar getGearRatio();
    btScalar getR();
    
private:
    //inputs
    btScalar V;
    
    //states
    btScalar I;
    
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
