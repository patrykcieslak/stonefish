//
//  ServoMotor.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 08/01/2019.
//  Copyright (c) 2019 Patryk Cieslak. All rights reserved.
//

#include "actuators/ServoMotor.h"

#include "entities/FeatherstoneEntity.h"
#include "joints/Joint.h"
#include "joints/RevoluteJoint.h"

namespace sf
{

ServoMotor::ServoMotor(std::string uniqueName, Scalar positionGain, Scalar velocityGain, Scalar maxTorque) : JointActuator(uniqueName)
{
    Kp = positionGain > Scalar(0) ? positionGain : Scalar(1);
    Kv = velocityGain > Scalar(0) ? velocityGain : Scalar(1);
    tauMax = maxTorque > Scalar(0) ? maxTorque : Scalar(0);
    pSetpoint = Scalar(0);
    vSetpoint = Scalar(0);
    velocityMode = false;
}
    
Scalar ServoMotor::getPosition()
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
    
Scalar ServoMotor::getVelocity()
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
    
Scalar ServoMotor::getEffort()
{
    if(fe != NULL)
        return fe->getMotorForceTorque(jId);
    else
        return Scalar(0);
}

void ServoMotor::AttachToJoint(FeatherstoneEntity* multibody, unsigned int jointId)
{
    JointActuator::AttachToJoint(multibody, jointId);
    
    if(fe != NULL) //Actuator succesfully attached?
    {
        fe->AddJointMotor(jId, tauMax);
        fe->MotorPositionSetpoint(jId, btScalar(0), Kp);
        fe->MotorVelocitySetpoint(jId, btScalar(0), Kv);
    }
}
    
void ServoMotor::Update(Scalar dt)
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
        if(velocityMode) //Velocity mode
        {
            fe->MotorPositionSetpoint(jId, Scalar(0), Scalar(0));
            fe->MotorVelocitySetpoint(jId, vSetpoint, Kv);
        }
        else //Position mode
        {
            fe->MotorPositionSetpoint(jId, pSetpoint, Kp);
            fe->MotorVelocitySetpoint(jId, vSetpoint, Kv);
        }
    }
}

void ServoMotor::setDesiredPosition(Scalar pos)
{
    pSetpoint = pos;
    vSetpoint = Scalar(0);
    velocityMode = false;
}

void ServoMotor::setDesiredVelocity(Scalar vel)
{
    vSetpoint = vel;
    velocityMode = true;
}

}
