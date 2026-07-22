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
//  DCMotor.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 1/11/13.
//  Copyright (c) 2013-2026 Patryk Cieslak. All rights reserved.
//

#include "actuators/DCMotor.h"

#include "core/DeviceFactory.h"
#include "joints/RevoluteJoint.h"
#include "entities/FeatherstoneEntity.h"

namespace sf
{

DCMotor::DCMotor(const std::string& uniqueName, Scalar motorR, Scalar motorL, Scalar motorKe, Scalar motorKt, Scalar friction) : JointActuator(uniqueName)
{
    //Params
    R_ = motorR;
    L_ = motorL;
    Ke_ = motorKe;
    Kt_ = motorKt;
    B_ = friction;
    gearEnabled_ = false;
    gearEff_ = Scalar(1.);
    gearRatio_ = Scalar(1.);
    
    //Internal states
    I_ = Scalar(0.);
    V_ = Scalar(0.);
    lastVoverL_ = Scalar(0.);
    torque_ = Scalar(0.);
    setVoltageLimit(-1); // No limit
}

JointActuatorType DCMotor::getJointActuatorType() const
{
    return JointActuatorType::DCMOTOR;
}

Scalar DCMotor::getKe() const
{
    return Ke_;
}

Scalar DCMotor::getKt() const
{
    return Kt_;
}

Scalar DCMotor::getR() const
{
    return R_;
}

Scalar DCMotor::getL() const
{
    return L_;
}

Scalar DCMotor::getGearRatio() const
{
    return gearRatio_;
}

void DCMotor::setVoltage(Scalar v)
{
    if(limit_ > Scalar(0)) // Limitted
        V_ = v < -limit_ ? -limit_ : (v > limit_ ? limit_ : v);
    else
        V_ = v;
    ResetWatchdog();
}

void DCMotor::setVoltageLimit(Scalar v)
{
    limit_ = v;
}

Scalar DCMotor::getVoltage() const
{
    return V_;
}

Scalar DCMotor::getTorque() const
{
    return torque_;
}

Scalar DCMotor::getCurrent() const
{
    return I_;
}

Scalar DCMotor::getAngle() const
{
    if(j_ != nullptr && j_->getType() == JointType::REVOLUTE)
    {
        return static_cast<RevoluteJoint*>(j_)->getAngle() * gearRatio_;
    }
    else if(fe_ != nullptr)
    {
        Scalar angle;
        btMultibodyLink::eFeatherstoneJointType jt = btMultibodyLink::eInvalid;
        fe_->getJointPosition(jId_, angle, jt);
        
        if(jt == btMultibodyLink::eRevolute)
            return angle * gearRatio_;
        else
            return Scalar(0);
    }
    else
        return Scalar(0);
}

Scalar DCMotor::getAngularVelocity() const
{
    if(j_ != nullptr && j_->getType() == JointType::REVOLUTE)
    {
        return static_cast<RevoluteJoint*>(j_)->getAngularVelocity() * gearRatio_;
    }
    else if(fe_ != nullptr)
    {
        Scalar angularV;
        btMultibodyLink::eFeatherstoneJointType jt = btMultibodyLink::eInvalid;
        fe_->getJointVelocity(jId_, angularV, jt);
        
        if(jt == btMultibodyLink::eRevolute)
            return angularV * gearRatio_;
        else
            return Scalar(0);
    }
    else
        return Scalar(0);
}

void DCMotor::Update(Scalar dt)
{
    Actuator::Update(dt);

    //Get joint angular velocity in radians
    Scalar aVelocity = getAngularVelocity();
    
    //Calculate internal state and output
    torque_ = (I_ * Kt_ - aVelocity * B_) * gearRatio_ * gearEff_;
    Scalar VoverL = (V_ - aVelocity * Ke_ * 9.5493 - I_ * R_)/L_;
	I_ += VoverL * dt;
	
	//Hack to avoid system blowup when the motor starts (shortcut)
	if((btFabs(I_) > btFabs(V_/R_)) && (I_*V_ > Scalar(0)))
		I_ = V_/R_;
        
	//I += Scalar(0.5) * (VoverL + lastVoverL) * dt; //Integration (mid-point)
    //lastVoverL = VoverL;
    
	//Drive the joint    
    if(j_ != nullptr && j_->getType() == JointType::REVOLUTE)
        static_cast<RevoluteJoint*>(j_)->ApplyTorque(torque_);
    else if(fe_ != nullptr)
        fe_->DriveJoint(jId_, torque_);
}

void DCMotor::SetupGearbox(bool enable, Scalar ratio, Scalar efficiency)
{
    gearEnabled_ = enable;
    gearRatio_ = ratio > 0.0 ? ratio : 1.0;
    gearEff_ = efficiency > 0.0 ? (efficiency <= 1.0 ? efficiency : 1.0) : 1.0;
}

void DCMotor::WatchdogTimeout()
{
    setVoltage(Scalar(0.));
}

// Statics

ConstructInfo DCMotor::getConstructInfo()
{
    ConstructInfo info;
    ConstructInfoNode node;
    
    // Specs
    node.optional = false;
    node.attributes.insert({"R", {ConstructInfoValueType::SCALAR, false}});
    node.attributes.insert({"L", {ConstructInfoValueType::SCALAR, false}});
    node.attributes.insert({"ke", {ConstructInfoValueType::SCALAR, false}});
    node.attributes.insert({"kt", {ConstructInfoValueType::SCALAR, false}});
    node.attributes.insert({"b", {ConstructInfoValueType::SCALAR, false}});
    info.nodes.insert({"specs", node});

    // Limits
    node.optional = true;
    node.attributes.clear();
    node.attributes.insert({"max_voltage", {ConstructInfoValueType::SCALAR, false}});
    info.nodes.insert({"limits", node});

    return info;
}

std::unique_ptr<DCMotor> DCMotor::Construct(const std::string& uniqueName, ConstructInfo& info)
{
    // Required
    Scalar R = std::get<Scalar>(info.nodes.at("specs").attributes.at("R").value);
    Scalar L = std::get<Scalar>(info.nodes.at("specs").attributes.at("L").value);
    Scalar ke = std::get<Scalar>(info.nodes.at("specs").attributes.at("ke").value);
    Scalar kt = std::get<Scalar>(info.nodes.at("specs").attributes.at("kt").value);
    Scalar b = std::get<Scalar>(info.nodes.at("specs").attributes.at("b").value);

    // Optional
    Scalar maxVoltage(-1.);
    ConstructInfoValue& value = info.nodes.at("limits").attributes.at("max_voltage");
    if (value.valid)
        maxVoltage = std::get<Scalar>(value.value);

    // Construct
    std::unique_ptr<DCMotor> actuator = std::make_unique<DCMotor>(uniqueName, R, L, ke, kt, b);
    actuator->setVoltageLimit(maxVoltage);

    return actuator;
}

REGISTER_ACTUATOR("dc_motor", DCMotor)

}