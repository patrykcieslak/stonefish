//
//  DCMotor.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 1/11/13.
//  Copyright (c) 2013-2017 Patryk Cieslak. All rights reserved.
//

#include "actuators/DCMotor.h"

using namespace sf;

DCMotor::DCMotor(std::string uniqueName, Scalar motorR, Scalar motorL, Scalar motorKe, Scalar motorKt, Scalar friction) : Motor(uniqueName)
{
    //Params
    R = motorR;
    L = motorL;
    Ke = motorKe;
    Kt = motorKt;
    B = friction;
    gearEnabled = false;
    gearEff = Scalar(1.);
    gearRatio = Scalar(1.);
    
    //Internal states
    I = Scalar(0.);
    V = Scalar(0.);
    lastVoverL = Scalar(0.);
}

Scalar DCMotor::getKe()
{
    return Ke;
}

Scalar DCMotor::getKt()
{
    return Kt;
}

Scalar DCMotor::getR()
{
    return R;
}

Scalar DCMotor::getGearRatio()
{
    return gearRatio;
}

void DCMotor::setIntensity(Scalar value)
{
    setVoltage(value);
}

void DCMotor::setVoltage(Scalar volt)
{
    V = volt;
}

Scalar DCMotor::getVoltage()
{
    return V;
}

Scalar DCMotor::getTorque()
{
    return torque;
}

Scalar DCMotor::getCurrent()
{
    return I;
}

Scalar DCMotor::getAngle()
{
    Scalar angle = Motor::getAngle();
    return angle * gearRatio;
}

Scalar DCMotor::getAngularVelocity()
{
    Scalar angularV = Motor::getAngularVelocity();
    return angularV * gearRatio;
}

void DCMotor::Update(Scalar dt)
{
    //Get joint angular velocity in radians
    Scalar aVelocity = getAngularVelocity();
    
    //Calculate internal state and output
    torque = (I * Kt - aVelocity * B) * gearRatio * gearEff;
    Scalar VoverL = (V - aVelocity * Ke * 9.5493 - I * R)/L;
	I += VoverL * dt;
	
	//Hack to avoid system blowup when the motor starts (shortcut)
	if((btFabs(I) > btFabs(V/R)) && (I*V > Scalar(0)))
		I = V/R;
        
	//I += Scalar(0.5) * (VoverL + lastVoverL) * dt; //Integration (mid-point)
    //lastVoverL = VoverL;
    
	//Drive the joint
    Motor::Update(dt);
}

void DCMotor::SetupGearbox(bool enable, Scalar ratio, Scalar efficiency)
{
    gearEnabled = enable;
    gearRatio = ratio > 0.0 ? ratio : 1.0;
    gearEff = efficiency > 0.0 ? (efficiency <= 1.0 ? efficiency : 1.0) : 1.0;
}
