//
//  DCMotor.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 1/11/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#include "DCMotor.h"

#pragma mark Constructors
DCMotor::DCMotor(std::string uniqueName, RevoluteJoint* revolute, btScalar motorR, btScalar motorL, btScalar motorKe, btScalar motorKt, btScalar friction) : Actuator(uniqueName)
{
    //Params
    revoluteOutput = revolute;
    multibodyOutput = NULL;
    multibodyChild = 0;
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
    torque = btScalar(0.);
    lastVoverL = btScalar(0.);
}

DCMotor::DCMotor(std::string uniqueName, FeatherstoneEntity* mb, unsigned int child, btScalar motorR, btScalar motorL, btScalar motorKe, btScalar motorKt, btScalar friction) : Actuator(uniqueName)
{
    //Params
    revoluteOutput = NULL;
    multibodyOutput = mb;
    multibodyChild = child;
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
    torque = btScalar(0.);
    lastVoverL = btScalar(0.);
}

#pragma mark - Accessors
ActuatorType DCMotor::getType()
{
    return ACTUATOR_MOTOR;
}

btScalar DCMotor::getKe()
{
    return Ke;
}

btScalar DCMotor::getKt()
{
    return Kt;
}

btScalar DCMotor::getGearRatio()
{
    return gearRatio;
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

btScalar DCMotor::getAngularVelocity()
{
    if(multibodyOutput == NULL)
    {
        return revoluteOutput->getAngularVelocity();
    }
    else
    {
        btScalar angularV = btScalar(0.);
        btMultibodyLink::eFeatherstoneJointType jt = btMultibodyLink::eInvalid;
        multibodyOutput->getJointVelocity(multibodyChild, angularV, jt);
        
        if(jt == btMultibodyLink::eRevolute)
            return angularV;
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
    btScalar aVelocity = getAngularVelocity();
    
    //Calculate internal state and output
    if(gearEnabled)
    {
        torque = (I * Kt - aVelocity * gearRatio * B) * gearRatio * gearEff;
    
        btScalar VoverL = (V - aVelocity * gearRatio * Ke * 9.5493 - I * R)/L;
        //I += btScalar(0.5) * (VoverL + lastVoverL) * dt; //Integration (mid-point)
        I += VoverL * dt;
        lastVoverL = VoverL;
        
        //I = (V - aVelocity * Ke * 9.5493)/R;
        //torque = (I * Km - aVelocity * B) * gearRatio * gearEff;

    }
    else
    {
        btScalar VoverL = (V - aVelocity * Ke * 9.5493 - I * R)/L;
        I += btScalar(0.5) * (VoverL + lastVoverL) * dt; //Integration (mid-point)
        lastVoverL = VoverL;
        torque = I * Kt - aVelocity * B;
        
        //I = (V - aVelocity * Ke * 9.5493)/R;
        //torque = I * Km - aVelocity * B;
    }
    
    //Drive the joint
    if(multibodyOutput == NULL)
        revoluteOutput->ApplyTorque(UnitSystem::GetTorque(torque));
    else
        multibodyOutput->DriveJoint(multibodyChild, UnitSystem::GetTorque(torque));
}

#pragma mark - DCMotor
void DCMotor::SetupGearbox(bool enable, btScalar ratio, btScalar efficiency)
{
    gearEnabled = enable;
    gearRatio = ratio > 0.0 ? ratio : 1.0;
    gearEff = efficiency > 0.0 ? (efficiency <= 1.0 ? efficiency : 1.0) : 1.0;
}