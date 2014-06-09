//
//  FixedJoint.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 2/4/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#include "FixedJoint.h"

FixedJoint::FixedJoint(std::string uniqueName, SolidEntity* solidA, SolidEntity* solidB) : Joint(uniqueName, false)
{
    btRigidBody* bodyA = solidA->getRigidBody();
    btRigidBody* bodyB = solidB->getRigidBody();
    btTransform frameInA = btTransform::getIdentity();
    btTransform frameInB = bodyB->getCenterOfMassTransform().inverse() * bodyA->getCenterOfMassTransform();
    
    btFixedConstraint* fixed = new btFixedConstraint(*bodyA, *bodyB, frameInA, frameInB);
    setConstraint(fixed);
}

FixedJoint::~FixedJoint()
{
}

JointType FixedJoint::getType()
{
    return FIXED;
}

btVector3 FixedJoint::Render()
{
    btTypedConstraint* fixed = getConstraint();
    btVector3 A = fixed->getRigidBodyA().getCenterOfMassPosition();
    btVector3 B = fixed->getRigidBodyB().getCenterOfMassPosition();
        
    glDummyColor();
    //link
    glBegin(GL_LINES);
    glBulletVertex(A);
    glBulletVertex(B);
    glEnd();
    
    return (A+B)/btScalar(2.);
}