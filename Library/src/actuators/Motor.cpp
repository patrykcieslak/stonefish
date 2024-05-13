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
//  Copyright (c) 2015-2023 Patryk Cieslak. All rights reserved.
//

#include "actuators/Motor.h"

#include "joints/RevoluteJoint.h"
#include "entities/FeatherstoneEntity.h"

namespace sf
{

Motor::Motor(std::string uniqueName) : JointActuator(uniqueName)
{
    torque = Scalar(0);
}

ActuatorType Motor::getType() const
{
    return ActuatorType::MOTOR;
}

void Motor::setIntensity(Scalar tau)
{
    torque = tau;
    ResetWatchdog();
}

Scalar Motor::getTorque() const
{
    return torque;
}

Scalar Motor::getAngle() const
{
    if(j != nullptr && j->getType() == JointType::REVOLUTE)
    {
        return ((RevoluteJoint*)j)->getAngle();
    }
    else if(fe != nullptr)
    {
        Scalar angle;
        btMultibodyLink::eFeatherstoneJointType jt = btMultibodyLink::eInvalid;
        fe->getJointPosition(jId, angle, jt);
        
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
    if(j != nullptr && j->getType() == JointType::REVOLUTE)
    {
        return ((RevoluteJoint*)j)->getAngularVelocity();
    }
    else if(fe != nullptr)
    {
        Scalar angularV;
        btMultibodyLink::eFeatherstoneJointType jt = btMultibodyLink::eInvalid;
        fe->getJointVelocity(jId, angularV, jt);
        
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

    if(j != nullptr && j->getType() == JointType::REVOLUTE)
        ((RevoluteJoint*)j)->ApplyTorque(torque);
    else if(fe != nullptr)
        fe->DriveJoint(jId, torque);
}

void Motor::WatchdogTimeout()
{
    setIntensity(Scalar(0));
}

}
