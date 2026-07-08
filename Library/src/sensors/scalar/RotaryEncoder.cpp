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
//  Copyright (c) 2014-2026 Patryk Cieslak. All rights reserved.
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

RotaryEncoder::RotaryEncoder(const std::string& uniqueName, Scalar frequency, int historyLength) : JointSensor(uniqueName, frequency, historyLength)
{
    angle_ = lastAngle_ = Scalar(0);
    motor_ = nullptr;
    thrust_ = nullptr;
    channels_.push_back(SensorChannel("Angle", QuantityType::ANGLE));
    channels_.push_back(SensorChannel("Angular velocity", QuantityType::ANGULAR_VELOCITY));
}

void RotaryEncoder::AttachToMotor(Motor* m)
{
    if(m != nullptr)
        motor_ = m;
}

void RotaryEncoder::AttachToThruster(Thruster* th)
{
    if(th != nullptr)
        thrust_ = th;
}

Scalar RotaryEncoder::GetRawAngle()
{
    if(j_ != nullptr && j_->getType() == JointType::REVOLUTE)
    {
        return ((RevoluteJoint*)j_)->getAngle();
    }
    else if(fe_ != nullptr)
    {
        Scalar mbAngle;
        btMultibodyLink::eFeatherstoneJointType jt = btMultibodyLink::eInvalid;
        fe_->getJointPosition(jId_, mbAngle, jt);
        
        if(jt == btMultibodyLink::eRevolute)
            return mbAngle;
        else
            return Scalar(0);
    }
    else if(motor_ != nullptr)
    {
        return motor_->getAngle();
    }
    else if(thrust_ != nullptr)
    {
        return thrust_->getAngle();
    }
    else
        return Scalar(0);
}

Scalar RotaryEncoder::GetRawAngularVelocity()
{
    if(j_ != nullptr && j_->getType() == JointType::REVOLUTE)
    {
        return ((RevoluteJoint*)j_)->getAngularVelocity();
    }
    else if(fe_ != nullptr)
    {
        Scalar mbAV;
        btMultibodyLink::eFeatherstoneJointType jt = btMultibodyLink::eInvalid;
        fe_->getJointVelocity(jId_, mbAV, jt);
        
        if(jt == btMultibodyLink::eRevolute)
            return mbAV;
        else
            return Scalar(0);
    }
    else if(motor_ != nullptr)
    {
        return motor_->getAngularVelocity();
    }
    else if(thrust_ != nullptr)
    {
        return thrust_->getOmega();
    }
    else
        return Scalar(0);
}

void RotaryEncoder::InternalUpdate(Scalar dt)
{
    //new angle
    Scalar actualAngle = GetRawAngle();
    
    //accumulate
    Scalar angle0 = angle_;

    if(lastAngle_ * actualAngle < Scalar(0))
    {
        if(lastAngle_ > M_PI_4)
            angle_ += ((actualAngle + 2 * M_PI) - lastAngle_);
        else if(lastAngle_ < -M_PI_4)
            angle_ += (actualAngle - (lastAngle_ + 2 * M_PI));
        else
            angle_ += (actualAngle - lastAngle_);
    }
    else
        angle_ += (actualAngle - lastAngle_);
    
    lastAngle_ = actualAngle;
    
    //angular velocity
    Scalar angularVelocity = (angle_ - angle0)/dt; // Less noisy than reading raw velocity
    
    //record sample
    AddSampleToHistory(std::make_unique<Sample>(
        std::vector<Scalar>({angle_, angularVelocity})
    ));
}

void RotaryEncoder::Reset()
{
    angle_ = lastAngle_ = GetRawAngle();
    ScalarSensor::Reset();
}

ScalarSensorType RotaryEncoder::getScalarSensorType() const
{
    return ScalarSensorType::ENCODER;
}

}
