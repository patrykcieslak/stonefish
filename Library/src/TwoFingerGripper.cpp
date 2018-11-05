//
//  TwoFingerGripper.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 06/07/18.
//  Copyright(c) 2018 Patryk Cieslak. All rights reserved.
//

#include <entities/systems/TwoFingerGripper.h>

#include <core/SimulationApp.h>

TwoFingerGripper::TwoFingerGripper(std::string uniqueName, Manipulator* m, SolidEntity* hand, SolidEntity* finger1, SolidEntity* finger2, const btVector3& pivotA, const btVector3& pivotB, const btVector3& axis, btScalar openAngle, btScalar closingTorque) : Gripper(uniqueName, m)
{
    btMultiBodyDynamicsWorld* world = SimulationApp::getApp()->getSimulationManager()->getDynamicsWorld();
    btScalar maxImpulse = btScalar(1)/SimulationApp::getApp()->getSimulationManager()->getStepsPerSecond() * UnitSystem::SetTorque(closingTorque);
    open = UnitSystem::SetAngle(openAngle)/btScalar(2);
 
    mechanism = new FeatherstoneEntity(uniqueName + "/FE", 3, hand, world, false);
    mechanism->setSelfCollision(false);
    mechanism->AddLink(finger1, btTransform::getIdentity(), world);
    mechanism->AddLink(finger2, btTransform::getIdentity(), world);
    mechanism->AddRevoluteJoint(uniqueName + "/FE/FingerJoint1", 0, 1, pivotA, axis);
    mechanism->AddRevoluteJoint(uniqueName + "/FE/FingerJoint2", 0, 2, pivotB, axis);
    mechanism->AddJointMotor(0, maxImpulse);
    mechanism->AddJointMotor(1, maxImpulse);
    mechanism->MotorVelocitySetpoint(0, 0.0, 1.0);
    mechanism->MotorVelocitySetpoint(1, 0.0, 1.0);
    SetState(openFrac);
}

void TwoFingerGripper::SetState(btScalar openFraction)
{
    openFrac = openFraction > 1.0 ? 1.0 : (openFraction < 0.0 ? 0.0 : openFraction);
    mechanism->MotorPositionSetpoint(0, openFrac*open, 1.0);
    mechanism->MotorPositionSetpoint(1, -openFrac*open, 1.0);
}