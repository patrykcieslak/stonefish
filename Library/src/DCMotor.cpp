//
//  DCMotor.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 1/11/13.
//  Copyright (c) 2013-2017 Patryk Cieslak. All rights reserved.
//

#include "DCMotor.h"

DCMotor::DCMotor(std::string uniqueName, RevoluteJoint* revolute, btScalar motorR, btScalar motorL, btScalar motorKe, btScalar motorKt, btScalar friction) : Motor(uniqueName, revolute)
{
    //Params
    R = motorR;
    L = motorL;
    Ke = motorKe;
    Kt = motorKt;
    B = friction;
    gearEnabled = false;
    gearEff = btScalar(1.);
    gearRatio = btScalar(1.);
    
    //Internal states
    I = btScalar(0.);
    V = btScalar(0.);
    lastVoverL = btScalar(0.);
}

DCMotor::DCMotor(std::string uniqueName, FeatherstoneEntity* mb, unsigned int child, btScalar motorR, btScalar motorL, btScalar motorKe, btScalar motorKt, btScalar friction) : Motor(uniqueName, mb, child)
{
    //Params
    R = motorR;
    L = motorL;
    Ke = motorKe;
    Kt = motorKt;
    B = friction;
    gearEnabled = false;
    gearEff = btScalar(1.);
    gearRatio = btScalar(1.);
    
    //Internal states
    I = btScalar(0.);
    V = btScalar(0.);
    lastVoverL = btScalar(0.);
}

btScalar DCMotor::getKe()
{
    return Ke;
}

btScalar DCMotor::getKt()
{
    return Kt;
}

btScalar DCMotor::getR()
{
    return R;
}

btScalar DCMotor::getGearRatio()
{
    return gearRatio;
}

void DCMotor::setIntensity(btScalar value)
{
    setVoltage(value);
}

void DCMotor::setVoltage(btScalar volt)
{
    V = volt;
}

btScalar DCMotor::getVoltage()
{
    return V;
}

btScalar DCMotor::getTorque()
{
    return torque;
}

btScalar DCMotor::getCurrent()
{
    return I;
}

void DCMotor::Update(btScalar dt)
{
    //Get joint angular velocity in radians
    btScalar aVelocity = getAngularVelocity();
    
    //Calculate internal state and output
    if(gearEnabled)
    {
        torque = (I * Kt - aVelocity * gearRatio * B) * gearRatio * gearEff;
        btScalar VoverL = (V - aVelocity * gearRatio * Ke * 9.5493 - I * R)/L;
		I += VoverL * dt;
	
		//Hack to avoid system blowup when the motor starts (shortcut)
		if((btFabs(I) > btFabs(V/R)) && (I*V > btScalar(0)))
			I = V/R;
        
		//I += btScalar(0.5) * (VoverL + lastVoverL) * dt; //Integration (mid-point)
        //lastVoverL = VoverL;
    }
    else
    {
        torque = I * Kt - aVelocity * gearRatio * B;
		btScalar VoverL = (V - aVelocity * Ke * 9.5493 - I * R)/L;
		I += VoverL * dt;
		
		//Hack to avoid system blowup when the motor starts (shortcut)
		if((btFabs(I) > btFabs(V/R)) && (I*V > btScalar(0)))
			I = V/R;
			
	    //I += btScalar(0.5) * (VoverL + lastVoverL) * dt; //Integration (mid-point)
        //lastVoverL = VoverL;
    }
    
	//Drive the joint
	Motor::Update(dt);
}

void DCMotor::SetupGearbox(bool enable, btScalar ratio, btScalar efficiency)
{
    gearEnabled = enable;
    gearRatio = ratio > 0.0 ? ratio : 1.0;
    gearEff = efficiency > 0.0 ? (efficiency <= 1.0 ? efficiency : 1.0) : 1.0;
}