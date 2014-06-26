//
//  DCMotor.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 1/11/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#include "DCMotor.h"

#pragma mark Constructors
DCMotor::DCMotor(std::string uniqueName, RevoluteJoint* revolute, btScalar motorR, btScalar motorL, btScalar motorKe, btScalar motorKm, btScalar friction) : Actuator(uniqueName)
{
    //Params
    revoluteOutput = revolute;
    multibodyOutput = NULL;
    multibodyChild = 0;
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

DCMotor::DCMotor(std::string uniqueName, FeatherstoneEntity* mb, unsigned int child, btScalar motorR, btScalar motorL, btScalar motorKe, btScalar motorKm, btScalar friction) : Actuator(uniqueName)
{
    //Params
    revoluteOutput = NULL;
    multibodyOutput = mb;
    multibodyChild = child;
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

#pragma mark - Destructor
DCMotor::~DCMotor()
{
}

#pragma mark - Accessors
ActuatorType DCMotor::getType()
{
    return ACTUATOR_DCMOTOR;
}

void DCMotor::setVoltage(btScalar volt)
{
    V = volt;
}

btScalar DCMotor::getTorque()
{
    return UnitSystem::GetLength(torque);
}

btScalar DCMotor::getCurrent()
{
    return I;
}

btScalar DCMotor::getRawAngularVelocity()
{
    if(multibodyOutput == NULL)
    {
        return UnitSystem::SetAngle(revoluteOutput->getAngularVelocity());
    }
    else
    {
        btScalar angularV = btScalar(0.);
        btMultibodyLink::eFeatherstoneJointType jt = btMultibodyLink::eInvalid;
        multibodyOutput->getJointVelocity(multibodyChild, angularV, jt);
        
        if(jt == btMultibodyLink::eRevolute)
            return UnitSystem::SetAngle(angularV);
        else
            return btScalar(0.);
    }
}

#pragma mark - Actuator
btVector3 DCMotor::Render()
{
    return btVector3(0.f,0.f,0.f);
}

void DCMotor::Update(btScalar dt)
{
    //Get joint angular velocity in radians
    btScalar aVelocity = getRawAngularVelocity();
    
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
    
    //Drive the joint
    if(multibodyOutput == NULL)
        revoluteOutput->ApplyTorque(UnitSystem::GetTorque(btVector3(torque,0,0)).x());
    else
        multibodyOutput->DriveJoint(multibodyChild, UnitSystem::GetTorque(btVector3(torque,0,0)).x());
}

#pragma mark - DCMotor
void DCMotor::SetupGearbox(bool enable, btScalar ratio, btScalar efficiency)
{
    gearEnabled = enable;
    gearRatio = ratio > 0.0 ? ratio : 1.0;
    gearEff = efficiency > 0.0 ? (efficiency <= 1.0 ? efficiency : 1.0) : 1.0;
}