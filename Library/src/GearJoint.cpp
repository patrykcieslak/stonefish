//
//  GearJoint.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 28/03/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#include "GearJoint.h"

GearJoint::GearJoint(std::string uniqueName, SolidEntity* solidA, SolidEntity* solidB, const btVector3& axisA, const btVector3& axisB, btScalar ratio) : Joint(uniqueName, false)
{
    gearRatio = ratio;
    
    btRigidBody* bodyA = solidA->getRigidBody();
    btRigidBody* bodyB = solidB->getRigidBody();
    btVector3 axisInA = bodyA->getCenterOfMassTransform().getBasis().inverse() * axisA;
    btVector3 axisInB = bodyB->getCenterOfMassTransform().getBasis().inverse() * axisB;
    
    btGearConstraint* gear = new btGearConstraint(*bodyA, *bodyB, axisInA, axisInB, gearRatio);
    setConstraint(gear);
}

GearJoint::~GearJoint()
{
}

JointType GearJoint::getType()
{
    return GEAR;
}

void GearJoint::Render()
{
    
}

btScalar GearJoint::getRatio()
{
    return gearRatio;
}