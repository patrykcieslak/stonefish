/*    
    This file is a part of Stonefish.

    Stonefish is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Stonefish is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

//
//  RotaryEncoder.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 05/07/2014.
//  Copyright (c) 2014-2018 Patryk Cieslak. All rights reserved.
//

#include "sensors/scalar/RotaryEncoder.h"

#include "sensors/Sample.h"
#include "joints/RevoluteJoint.h"
#include "utils/UnitSystem.h"
#include "entities/FeatherstoneEntity.h"
#include "actuators/Motor.h"
#include "actuators/Thruster.h"

namespace sf
{

RotaryEncoder::RotaryEncoder(std::string uniqueName, Scalar frequency, int historyLength) : JointSensor(uniqueName, frequency, historyLength)
{
    angle = lastAngle = Scalar(0);
    motor = NULL;
    thrust = NULL;
    channels.push_back(SensorChannel("Angle", QuantityType::ANGLE));
    channels.push_back(SensorChannel("Angular velocity", QuantityType::ANGULAR_VELOCITY));
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

ScalarSensorType RotaryEncoder::getScalarSensorType()
{
    return ScalarSensorType::ENCODER;
}

}
