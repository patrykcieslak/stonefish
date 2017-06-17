//
//  FixedJoint.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 2/4/13.
//  Copyright (c) 2013-2017 Patryk Cieslak. All rights reserved.
//

#include "FixedJoint.h"
#include <BulletDynamics/Featherstone/btMultiBodyFixedConstraint.h>

FixedJoint::FixedJoint(std::string uniqueName, SolidEntity* solidA, SolidEntity* solidB) : Joint(uniqueName, false)
{
    btRigidBody* bodyA = solidA->getRigidBody();
    btRigidBody* bodyB = solidB->getRigidBody();
    btTransform frameInA = btTransform::getIdentity();
    btTransform frameInB = bodyB->getCenterOfMassTransform().inverse() * bodyA->getCenterOfMassTransform();
    
    btFixedConstraint* fixed = new btFixedConstraint(*bodyA, *bodyB, frameInA, frameInB);
    setConstraint(fixed);
}

FixedJoint::FixedJoint(std::string uniqueName, btMultiBody* mb, btRigidBody* rb) : Joint(uniqueName, false)
{
	btTransform frameInRB = rb->getCenterOfMassTransform().inverse() * mb->getBaseWorldTransform();
	
	btMultiBodyFixedConstraint* fixed = new btMultiBodyFixedConstraint(mb, -1, rb, btVector3(0,0,0), frameInRB.getOrigin(), btMatrix3x3::getIdentity(), frameInRB.getBasis());
	setConstraint(fixed);
}

JointType FixedJoint::getType()
{
    return JOINT_FIXED;
}

btVector3 FixedJoint::Render()
{
    return btVector3();
}