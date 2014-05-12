//
//  BeltJoint.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 28/03/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#include "BeltJoint.h"

BeltJoint::BeltJoint(std::string uniqueName, SolidEntity* solidA, SolidEntity* solidB, const btVector3& axisA, const btVector3& axisB, btScalar ratio) : Joint(uniqueName, false)
{
    btVector3 newAxisB = -axisB; // Belt -> same direction of rotation
    gearRatio = ratio;
    
    btRigidBody* bodyA = solidA->getRigidBody();
    btRigidBody* bodyB = solidB->getRigidBody();
    btVector3 axisInA = bodyA->getCenterOfMassTransform().getBasis().inverse() * axisA;
    btVector3 axisInB = bodyB->getCenterOfMassTransform().getBasis().inverse() * newAxisB;
    
    btGearConstraint* gear = new btGearConstraint(*bodyA, *bodyB, axisInA, axisInB, gearRatio);
    setConstraint(gear);
}

BeltJoint::~BeltJoint()
{
}

JointType BeltJoint::getType()
{
    return BELT;
}

void BeltJoint::ApplyDamping()
{
}

btVector3 BeltJoint::Render()
{
    return btVector3();
}

btScalar BeltJoint::getRatio()
{
    return gearRatio;
}