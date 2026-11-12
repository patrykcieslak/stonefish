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
//  Copyright (c) 2015-2026 Patryk Cieslak. All rights reserved.
//

#include "actuators/Motor.h"

#include "core/DeviceFactory.h"
#include "joints/RevoluteJoint.h"
#include "entities/FeatherstoneEntity.h"

namespace sf
{

Motor::Motor(std::string_view uniqueName) : JointActuator(uniqueName)
{
    torque_ = Scalar(0);
    SetTorqueLimit(-1); // No limit
}

JointActuatorType Motor::GetJointActuatorType() const
{
    return JointActuatorType::MOTOR;
}

void Motor::SetTorqueLimit(Scalar tau)
{
    limit_ = tau;
}

void Motor::SetTorque(Scalar tau)
{
    if(limit_ > Scalar(0)) // Limitted
        torque_ = tau < -limit_ ? -limit_ : (tau > limit_ ? limit_ : tau);
    else
        torque_ = tau;
    ResetWatchdog();
}

Scalar Motor::GetTorque() const
{
    return torque_;
}

Scalar Motor::GetAngle() const
{
    if(j_ != nullptr && j_->GetType() == JointType::REVOLUTE)
    {
        return static_cast<RevoluteJoint*>(j_)->GetAngle();
    }
    else if(fe_ != nullptr)
    {
        Scalar angle;
        btMultibodyLink::eFeatherstoneJointType jt = btMultibodyLink::eInvalid;
        fe_->GetJointPosition(jId_, angle, jt);
        
        if(jt == btMultibodyLink::eRevolute)
            return angle;
        else
            return Scalar(0);
    }
    else
        return Scalar(0);
}

Scalar Motor::GetAngularVelocity() const
{
    if(j_ != nullptr && j_->GetType() == JointType::REVOLUTE)
    {
        return static_cast<RevoluteJoint*>(j_)->GetAngularVelocity();
    }
    else if(fe_ != nullptr)
    {
        Scalar angularV;
        btMultibodyLink::eFeatherstoneJointType jt = btMultibodyLink::eInvalid;
        fe_->GetJointVelocity(jId_, angularV, jt);
        
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

    if(j_ != nullptr && j_->GetType() == JointType::REVOLUTE)
        static_cast<RevoluteJoint*>(j_)->ApplyTorque(torque_);
    else if(fe_ != nullptr)
        fe_->DriveJoint(jId_, torque_);
}

void Motor::WatchdogTimeout()
{
    SetTorque(Scalar(0));
}

// Statics

ConstructInfo Motor::GetConstructInfo()
{
    ConstructInfo info;
    ConstructInfoNode node;
    
    // Limits
    node.optional = true;
    node.attributes.insert({"max_torque", {ConstructInfoValueType::SCALAR, false}});
    info.nodes.insert({"limits", node});

    return info;
}

std::unique_ptr<Motor> Motor::Construct(std::string_view uniqueName, ConstructInfo& info)
{
    // Optional
    Scalar maxTorque(-1.);
    ConstructInfoValue& value = info.nodes.at("limits").attributes.at("max_torque");
    if (value.valid)
        maxTorque = std::get<Scalar>(value.value);

    // Construct
    std::unique_ptr<Motor> actuator = std::make_unique<Motor>(uniqueName);
    actuator->SetTorqueLimit(maxTorque);

    return actuator;
}

REGISTER_ACTUATOR("motor", Motor)

}
