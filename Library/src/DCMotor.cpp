//
//  DCMotor.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 1/11/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#include "DCMotor.h"

DCMotor::DCMotor(std::string uniqueName, RevoluteJoint* revolute, btScalar motorR, btScalar motorL, btScalar motorKe, btScalar motorKm, btScalar friction) : Actuator(uniqueName)
{
    //Params
    output = revolute;
    R = motorR;
    L = motorL;
    Ke = motorKe;
    Km = motorKm;
    B = friction;
    gearEnabled = false;
    gearEff = 1.0;
    gearRatio = 1.0;
    
    //Internal states
    I = btScalar(0);
    V = btScalar(0);
    torque = btScalar(0);
}

DCMotor::~DCMotor()
{
}

void DCMotor::setVoltage(btScalar volt)
{
    V = volt;
}

btScalar DCMotor::getTorque()
{
    return UnitSystem::GetLength(torque);
}

ActuatorType DCMotor::getType()
{
    return DC_MOTOR;
}

btVector3 DCMotor::Render()
{
    return btVector3(0.f,0.f,0.f);
}

void DCMotor::SetupGearbox(bool enable, btScalar ratio, btScalar efficiency)
{
    gearEnabled = enable;
    gearRatio = ratio > 0.0 ? ratio : 1.0;
    gearEff = efficiency > 0.0 ? (efficiency <= 1.0 ? efficiency : 1.0) : 1.0;
}

void DCMotor::Update(btScalar dt)
{
    //Get joint angular velocity in radians
    btScalar aVelocity = UnitSystem::SetAngle(output->getAngularVelocity());
    
    //Calculate internal state and output
    if(gearEnabled)
    {
        aVelocity *= gearRatio;
        I = (V - aVelocity * Ke * 9.5493)/R;
        torque = (I * Km - aVelocity * B) * gearRatio * gearEff;
    }
    else
    {
        I = (V - aVelocity * Ke * 9.5493)/R;
        torque = I * Km - aVelocity * B;
    }
    
    //Drive the revolute joint
    output->applyTorque(UnitSystem::GetTorque(btVector3(torque,0,0)).x());
}