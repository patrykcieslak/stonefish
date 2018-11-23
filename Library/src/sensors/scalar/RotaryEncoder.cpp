//
//  RotaryEncoder.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 05/07/2014.
//  Copyright (c) 2014-2017 Patryk Cieslak. All rights reserved.
//

#include "sensors/scalar/RotaryEncoder.h"

#include "joints/RevoluteJoint.h"

using namespace sf;

RotaryEncoder::RotaryEncoder(std::string uniqueName, btScalar frequency, int historyLength) : JointSensor(uniqueName, frequency, historyLength)
{
    angle = lastAngle = btScalar(0);
    motor = NULL;
    thrust = NULL;
    channels.push_back(SensorChannel("Angle", QUANTITY_ANGLE));
    channels.push_back(SensorChannel("Angular velocity", QUANTITY_ANGULAR_VELOCITY));
}

void RotaryEncoder::AttachToMotor(Motor* m)
{
    if(m != NULL)
        motor = m;
}

void RotaryEncoder::AttachToThruster(Thruster* th)
{
    if(th != NULL)
        thrust = th;
}

btScalar RotaryEncoder::GetRawAngle()
{
    if(j != NULL && j->getType() == JointType::JOINT_REVOLUTE)
    {
        return ((RevoluteJoint*)j)->getAngle();
    }
    else if(fe != NULL)
    {
        btScalar mbAngle;
        btMultibodyLink::eFeatherstoneJointType jt = btMultibodyLink::eInvalid;
        fe->getJointPosition(jId, mbAngle, jt);
        
        if(jt == btMultibodyLink::eRevolute)
            return mbAngle;
        else
            return btScalar(0);
    }
    else if(motor != NULL)
    {
        return motor->getAngle();
    }
    else if(thrust != NULL)
    {
        return thrust->getAngle();
    }
    else
        return btScalar(0);
}

btScalar RotaryEncoder::GetRawAngularVelocity()
{
    if(j != NULL && j->getType() == JointType::JOINT_REVOLUTE)
    {
        return ((RevoluteJoint*)j)->getAngularVelocity();
    }
    else if(fe != NULL)
    {
        btScalar mbAV;
        btMultibodyLink::eFeatherstoneJointType jt = btMultibodyLink::eInvalid;
        fe->getJointVelocity(jId, mbAV, jt);
        
        if(jt == btMultibodyLink::eRevolute)
            return mbAV;
        else
            return btScalar(0);
    }
    else if(motor != NULL)
    {
        return motor->getAngularVelocity();
    }
    else if(thrust != NULL)
    {
        return thrust->getOmega();
    }
    else
        return btScalar(0);
}

void RotaryEncoder::InternalUpdate(btScalar dt)
{
    //new angle
    btScalar actualAngle = GetRawAngle();
    
    //accumulate
    if(lastAngle * actualAngle < btScalar(0.))
    {
        if(lastAngle > M_PI_4)
            angle += ((actualAngle + FULL_ANGLE) - lastAngle);
        else if(lastAngle < -M_PI_4)
            angle += (actualAngle - (lastAngle + FULL_ANGLE));
        else
            angle += (actualAngle-lastAngle);
    }
    else
        angle += (actualAngle-lastAngle);
    
    lastAngle = actualAngle;
    
    //angular velocity
    btScalar angularVelocity = GetRawAngularVelocity();
    
    //record sample
    btScalar m[2];
    m[0] = angle;
    m[1] = angularVelocity;
    Sample s(2, m);
    AddSampleToHistory(s);
}

void RotaryEncoder::Reset()
{
    angle = lastAngle = GetRawAngle();
    ScalarSensor::Reset();
}
