//
//  RotaryEncoder.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 05/07/2014.
//  Copyright (c) 2014-2017 Patryk Cieslak. All rights reserved.
//

#include "sensors/scalar/RotaryEncoder.h"

#include "joints/RevoluteJoint.h"
#include "utils/UnitSystem.h"

using namespace sf;

RotaryEncoder::RotaryEncoder(std::string uniqueName, Scalar frequency, int historyLength) : JointSensor(uniqueName, frequency, historyLength)
{
    angle = lastAngle = Scalar(0);
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

Scalar RotaryEncoder::GetRawAngle()
{
    if(j != NULL && j->getType() == JointType::JOINT_REVOLUTE)
    {
        return ((RevoluteJoint*)j)->getAngle();
    }
    else if(fe != NULL)
    {
        Scalar mbAngle;
        btMultibodyLink::eFeatherstoneJointType jt = btMultibodyLink::eInvalid;
        fe->getJointPosition(jId, mbAngle, jt);
        
        if(jt == btMultibodyLink::eRevolute)
            return mbAngle;
        else
            return Scalar(0);
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
        return Scalar(0);
}

Scalar RotaryEncoder::GetRawAngularVelocity()
{
    if(j != NULL && j->getType() == JointType::JOINT_REVOLUTE)
    {
        return ((RevoluteJoint*)j)->getAngularVelocity();
    }
    else if(fe != NULL)
    {
        Scalar mbAV;
        btMultibodyLink::eFeatherstoneJointType jt = btMultibodyLink::eInvalid;
        fe->getJointVelocity(jId, mbAV, jt);
        
        if(jt == btMultibodyLink::eRevolute)
            return mbAV;
        else
            return Scalar(0);
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
        return Scalar(0);
}

void RotaryEncoder::InternalUpdate(Scalar dt)
{
    //new angle
    Scalar actualAngle = GetRawAngle();
    
    //accumulate
    if(lastAngle * actualAngle < Scalar(0.))
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
    Scalar angularVelocity = GetRawAngularVelocity();
    
    //record sample
    Scalar m[2];
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
