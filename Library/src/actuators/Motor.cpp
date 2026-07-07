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
//  Motor.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 15/09/2015.
//  Copyright (c) 2015-2024 Patryk Cieslak. All rights reserved.
//

#include "actuators/Motor.h"

#include "joints/RevoluteJoint.h"
#include "entities/FeatherstoneEntity.h"

namespace sf
{

Motor::Motor(std::string uniqueName) : JointActuator(uniqueName)
{
    torque_ = Scalar(0);
    setTorqueLimits(1, -1); // No limits
}

ActuatorType Motor::getType() const
{
    return ActuatorType::MOTOR;
}

void Motor::setTorqueLimits(Scalar lower, Scalar upper)
{
    limits_.first = lower;
    limits_.second = upper;
}

void Motor::setCommand(Scalar tau)
{
    torque_ = tau;
    if(limits_.second > limits_.first) // Limitted
        torque_ = tau < limits_.first ? limits_.first : (tau > limits_.second ? limits_.second : tau);
    else
        torque_ = tau;
    ResetWatchdog();
}

Scalar Motor::getTorque() const
{
    return torque_;
}

Scalar Motor::getAngle() const
{
    if(j_ != nullptr && j_->getType() == JointType::REVOLUTE)
    {
        return ((RevoluteJoint*)j_)->getAngle();
    }
    else if(fe_ != nullptr)
    {
        Scalar angle;
        btMultibodyLink::eFeatherstoneJointType jt = btMultibodyLink::eInvalid;
        fe_->getJointPosition(jId_, angle, jt);
        
        if(jt == btMultibodyLink::eRevolute)
            return angle;
        else
            return Scalar(0);
    }
    else
        return Scalar(0);
}

Scalar Motor::getAngularVelocity() const
{
    if(j_ != nullptr && j_->getType() == JointType::REVOLUTE)
    {
        return ((RevoluteJoint*)j_)->getAngularVelocity();
    }
    else if(fe_ != nullptr)
    {
        Scalar angularV;
        btMultibodyLink::eFeatherstoneJointType jt = btMultibodyLink::eInvalid;
        fe_->getJointVelocity(jId_, angularV, jt);
        
        if(jt == btMultibodyLink::eRevolute)
            return angularV;
        else
            return Scalar(0);
    }
    else
        return Scalar(0);
}

void Motor::Update(Scalar dt)
{
    Actuator::Update(dt);

    if(j_ != nullptr && j_->getType() == JointType::REVOLUTE)
        ((RevoluteJoint*)j_)->ApplyTorque(torque_);
    else if(fe_ != nullptr)
        fe_->DriveJoint(jId_, torque_);
}

void Motor::WatchdogTimeout()
{
    setCommand(Scalar(0));
}

}
