//
//  DCMotor.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 1/11/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#include "DCMotor.h"

#include <math.h>

DCMotor::DCMotor(RevoluteJoint* revolute, btScalar motorR, btScalar motorL, btScalar motorKe, btScalar motorKm, btScalar friction, btScalar motorEfficiency)
{
    output = revolute;
    I = 0.0;
    V = 0.0;
    
    R = motorR > 0.0 ? motorR : 1.0;
    L = motorL > 0.0 ? motorL : 1.0;
    Ke = motorKe > 0.0 ? motorKe : 1.0;
    Km = motorKm > 0.0 ? motorKm : 1.0;
    B = friction >= 0.0 ? friction : 0.0;
    eff = motorEfficiency > 0.0 ? (motorEfficiency <= 1.0 ? motorEfficiency : 1.0) : 1.0;
    gearEnabled = false;
    gearEff = 1.0;
    gearRatio = 1.0;
}

DCMotor::~DCMotor()
{
    output = NULL;
}

void DCMotor::SetGearbox(bool enable, btScalar ratio, btScalar efficiency)
{
    gearEnabled = enable;
    gearRatio = ratio > 0.0 ? ratio : 1.0;
    gearEff = efficiency > 0.0 ? (efficiency <= 1.0 ? efficiency : 1.0) : 1.0;
}

Actuator::ActuatorType DCMotor::getType()
{
    return DCMOTOR;
}

void DCMotor::Render()
{
    if(isRenderable())
    {
    }
}

void DCMotor::SetInput(btScalar *inputValues)
{
    V = inputValues[0];
}

void DCMotor::Update(btScalar dt)
{
    /*btScalar aVelocity = output->getAngularVelocity();
    btScalar torque;
    
    if(gearEnabled)
    {
        I = (V-aVelocity*gearRatio*gearEff*Ke*9.5493-I*R)/L*dt;
        torque = (I*Km*eff-aVelocity*gearRatio*gearEff*B)*gearRatio*gearEff;
    }
    else
    {
        I = (V-aVelocity*Ke*9.5493-I*R)/L*dt;
        torque = I*Km*eff-aVelocity*B;
    }
    
    output->applyTorque(torque);*/
}
