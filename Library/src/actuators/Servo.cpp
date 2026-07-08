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
//  Servo.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 08/01/2019.
//  Copyright (c) 2019-2026 Patryk Cieslak. All rights reserved.
//

#include "actuators/Servo.h"

#include "core/SimulationApp.h"
#include "entities/FeatherstoneEntity.h"
#include "joints/Joint.h"
#include "joints/RevoluteJoint.h"

namespace sf
{

Servo::Servo(const std::string& uniqueName, Scalar positionGain, Scalar velocityGain, Scalar maxTorque) : JointActuator(uniqueName)
{
    Kp_ = btFabs(positionGain);
    Kv_ = btFabs(velocityGain);
    tauMax_ = btFabs(maxTorque);
    pSetpoint_ = Scalar(0);
    vSetpoint_ = Scalar(0);
    vLimit_ = Scalar(-1); // No limit
    mode_ = ServoControlMode::VELOCITY;
}

ActuatorType Servo::getType() const
{
    return ActuatorType::SERVO;
}    

void Servo::setControlMode(ServoControlMode m)
{
    mode_ = m;
}

void Servo::setDesiredPosition(Scalar pos)
{
    if(btFuzzyZero(pSetpoint_ - pos)) //Check if setpoint changed
        return;
    
    if(fe_ != nullptr)
    {
        FeatherstoneJoint jnt = fe_->getJoint(jId_);
        if(jnt.lowerLimit < jnt.upperLimit) //Does it have joint limits?
            pos = btClamped(pos, jnt.lowerLimit, jnt.upperLimit);
    }
    
    pSetpoint_ = pos;
}

void Servo::setDesiredVelocity(Scalar vel)
{
    if(btFuzzyZero(vSetpoint_ - vel)) //Check if setpoint changed
        return;
        
    if(btFuzzyZero(vel))
        pSetpoint_ = getPosition();

    if(vLimit_ > Scalar(0))
        vSetpoint_ = btClamped(vel, -vLimit_, vLimit_);
    else
        vSetpoint_ = vel;

    ResetWatchdog();
}

void Servo::setMaxVelocity(Scalar vel)
{
    vLimit_ = vel;
}

void Servo::setMaxTorque(Scalar tau)
{
    tauMax_ = tau;
    if(fe_ != nullptr)
        fe_->setMaxMotorForceTorque(jId_, tauMax_);
}

Scalar Servo::getDesiredPosition() const
{
    return pSetpoint_;
}
        
Scalar Servo::getDesiredVelocity() const
{
    return vSetpoint_;
}
    
Scalar Servo::getPosition() const
{
    if(j_ != nullptr)
    {
        switch(j_->getType())
        {
            case JointType::REVOLUTE:
                return ((RevoluteJoint*)j_)->getAngle();
            
            default:
                return Scalar(0);
        }
    }
    else if(fe_ != nullptr)
    {
        Scalar pos;
        btMultibodyLink::eFeatherstoneJointType jt = btMultibodyLink::eInvalid;
        fe_->getJointPosition(jId_, pos, jt);
        return pos;
    }
    else
        return Scalar(0);
}
    
Scalar Servo::getVelocity() const
{
    if(j_ != nullptr)
    {
        switch(j_->getType())
        {
            case JointType::REVOLUTE:
                return ((RevoluteJoint*)j_)->getAngularVelocity();
            
            default:
                return Scalar(0);
        }
    }
    else if(fe_ != nullptr)
    {
        Scalar vel;
        btMultibodyLink::eFeatherstoneJointType jt = btMultibodyLink::eInvalid;
        fe_->getJointVelocity(jId_, vel, jt);
        return vel;
    }
    else
        return Scalar(0);
}
    
Scalar Servo::getEffort() const
{
    if(fe_ != nullptr)
        return fe_->getMotorForceTorque(jId_);
    else
        return Scalar(0);
}

void Servo::AttachToJoint(FeatherstoneEntity* multibody, unsigned int jointId)
{
    JointActuator::AttachToJoint(multibody, jointId);
    
    if(fe_ != nullptr) //Actuator succesfully attached?
    {
        fe_->AddJointMotor(jId_, tauMax_);
        fe_->MotorPositionSetpoint(jId_, Scalar(0), Kp_);
        fe_->MotorVelocitySetpoint(jId_, Scalar(0), Kv_);
    }
}

void Servo::AttachToJoint(Joint* joint)
{
    JointActuator::AttachToJoint(joint);
    
    if(j_ != nullptr)
    {
        switch(j_->getType())
        {
            case JointType::REVOLUTE:
            {
                pSetpoint_ = getPosition();
                ((RevoluteJoint*)j_)->EnableMotor(true, tauMax_);
            }
                break;

            default:
                break;
        }
    }
}

void Servo::Update(Scalar dt)
{
    Actuator::Update(dt);

    if(j_ != nullptr)
    {
        Scalar vSetpoint2;
        if(mode_ == ServoControlMode::POSITION || btFuzzyZero(vSetpoint_))
        {
            Scalar err = pSetpoint_ - getPosition();
            vSetpoint2 = Kp_ * err;
            if(vLimit_ > Scalar(0))
                vSetpoint2 = btClamped(vSetpoint2, -vLimit_, vLimit_);
        }
        else
        {
            //vSetpoint2 = vSetpoint;
            Scalar err = vSetpoint_ - getVelocity();  
            vSetpoint2 = Kv_ * err + vSetpoint_;
            if(vLimit_ > Scalar(0))
                vSetpoint2 = btClamped(vSetpoint2, -vLimit_, vLimit_);
        }

        switch(j_->getType())
        {
            case JointType::REVOLUTE:
                ((RevoluteJoint*)j_)->setMotorVelocity(vSetpoint2);
                break;
            
            default:
                break;
        }
    }
    else if(fe_ != nullptr)
    {
        //Use internal multibody motors
        switch(mode_)
        {
            case ServoControlMode::POSITION: 
            {
                Scalar err = pSetpoint_ - getPosition();
                if(vLimit_ > Scalar(0)               // If velocity is limited 
                   && btFabs(err) > vLimit_ * dt)    // and position error could result in crossing this limit
                {
                    fe_->MotorPositionSetpoint(jId_, Scalar(0), Scalar(0));
                    fe_->MotorVelocitySetpoint(jId_, err > Scalar(0) ? vLimit_ : -vLimit_, Kv_);
                }
                else
                {
                    fe_->MotorPositionSetpoint(jId_, pSetpoint_, Kp_);
                    fe_->MotorVelocitySetpoint(jId_, Scalar(0), Kv_);
                }
            }
                break;
                
            case ServoControlMode::VELOCITY:
            case ServoControlMode::TORQUE:
            {
                if(btFuzzyZero(vSetpoint_))
                { 
                    fe_->MotorPositionSetpoint(jId_, pSetpoint_, Kp_);
                    fe_->MotorVelocitySetpoint(jId_, Scalar(0), Kv_);
                }
                else
                {
                    Scalar vSetpoint2 = vSetpoint_;
                    
                    //Do not allow to cross limits by changing velocity setpoint
                    FeatherstoneJoint jnt = fe_->getJoint(jId_);
                    if(jnt.lowerLimit < jnt.upperLimit)
                    {
                        Scalar jpos = getPosition();
                        Scalar jpos2 = jpos + vSetpoint2 * dt;
                        vSetpoint2 = jpos2 < jnt.lowerLimit ? (jnt.lowerLimit - jpos)/dt : (jpos2 > jnt.upperLimit ? (jnt.upperLimit - jpos)/dt : vSetpoint2);
                    }
                    
                    fe_->MotorPositionSetpoint(jId_, Scalar(0), Scalar(0));
                    fe_->MotorVelocitySetpoint(jId_, vSetpoint2, Kv_);
                }
            }
                break;
        }
    }
}

void Servo::WatchdogTimeout()
{
    if(mode_ == ServoControlMode::VELOCITY)
        setDesiredVelocity(Scalar(0));
}

}
