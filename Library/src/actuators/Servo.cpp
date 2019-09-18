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
//  Copyright (c) 2019 Patryk Cieslak. All rights reserved.
//

#include "actuators/Servo.h"

#include "entities/FeatherstoneEntity.h"
#include "joints/Joint.h"
#include "joints/RevoluteJoint.h"

namespace sf
{

Servo::Servo(std::string uniqueName, Scalar positionGain, Scalar velocityGain, Scalar maxTorque) : JointActuator(uniqueName)
{
    Kp = positionGain > Scalar(0) ? positionGain : Scalar(1);
    Kv = velocityGain > Scalar(0) ? velocityGain : Scalar(1);
    tauMax = maxTorque > Scalar(0) ? maxTorque : Scalar(0);
    pSetpoint = Scalar(0);
    vSetpoint = Scalar(0);
    mode = ServoControlMode::POSITION_CTRL;
}

ActuatorType Servo::getType()
{
    return ActuatorType::ACTUATOR_SERVO;
}    

void Servo::setControlMode(ServoControlMode m)
{
    mode = m;
}

void Servo::setDesiredPosition(Scalar pos)
{
    pSetpoint = pos;
}

void Servo::setDesiredVelocity(Scalar vel)
{
    vSetpoint = vel;
}

void Servo::setDesiredTorque(Scalar tau)
{
    vSetpoint = tau > Scalar(0) ? Scalar(1000) : (tau < Scalar(0) ? Scalar(-1000) : Scalar(0));
    tau = tau < -tauMax ? -tauMax : (tau > tauMax ? tauMax : tau);
    
    if(fe != NULL)
    {
        fe->setMaxMotorForceTorque(jId, tau);
    }
}
    
Scalar Servo::getPosition()
{
    if(j != NULL)
    {
        switch(j->getType())
        {
            case JointType::JOINT_REVOLUTE:
                return ((RevoluteJoint*)j)->getAngle();
            
            default:
                return Scalar(0);
        }
    }
    else if(fe != NULL)
    {
        Scalar pos;
        btMultibodyLink::eFeatherstoneJointType jt = btMultibodyLink::eInvalid;
        fe->getJointPosition(jId, pos, jt);
        return pos;
    }
    else
        return Scalar(0);
}
    
Scalar Servo::getVelocity()
{
    if(j != NULL)
    {
        switch(j->getType())
        {
            case JointType::JOINT_REVOLUTE:
                return ((RevoluteJoint*)j)->getAngularVelocity();
            
            default:
                return Scalar(0);
        }
    }
    else if(fe != NULL)
    {
        Scalar vel;
        btMultibodyLink::eFeatherstoneJointType jt = btMultibodyLink::eInvalid;
        fe->getJointVelocity(jId, vel, jt);
        return vel;
    }
    else
        return Scalar(0);
}
    
Scalar Servo::getEffort()
{
    if(fe != NULL)
        return fe->getMotorForceTorque(jId);
    else
        return Scalar(0);
}

void Servo::AttachToJoint(FeatherstoneEntity* multibody, unsigned int jointId)
{
    JointActuator::AttachToJoint(multibody, jointId);
    
    if(fe != NULL) //Actuator succesfully attached?
    {
        fe->AddJointMotor(jId, tauMax);
        fe->MotorPositionSetpoint(jId, btScalar(0), Kp);
        fe->MotorVelocitySetpoint(jId, btScalar(0), Kv);
    }
}
    
void Servo::Update(Scalar dt)
{
    if(j != NULL)
    {
        //Compute torque using a simple proportional controller
        
        
        //Apply torque
        switch(j->getType())
        {
            case JointType::JOINT_REVOLUTE:
                //((RevoluteJoint*)j)->ApplyTorque(torque);
                break;
            
            default:
                break;
        }
    }
    else if(fe != NULL)
    {
        //Use internal multibody motors
        switch(mode)
        {
            case POSITION_CTRL: 
                fe->MotorPositionSetpoint(jId, pSetpoint, Kp);
                fe->MotorVelocitySetpoint(jId, vSetpoint, Kv);
                break;
                
            case VELOCITY_CTRL:
            case TORQUE_CTRL:
                fe->MotorPositionSetpoint(jId, Scalar(0), Scalar(0));
                fe->MotorVelocitySetpoint(jId, vSetpoint, Kv);
                break;
        }
    }
}

}
