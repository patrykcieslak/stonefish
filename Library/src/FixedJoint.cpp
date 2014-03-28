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

void FixedJoint::Render()
{
    btTypedConstraint* fixed = getConstraint();
    
    btVector3 A = fixed->getRigidBodyA().getCenterOfMassPosition();
    btVector3 B = fixed->getRigidBodyB().getCenterOfMassPosition();
        
    glColor3f(1.f, 0, 0);
    glBegin(GL_LINES);
    glVertex3f(A.x(), A.y(), A.z());
    glVertex3f(B.x(), B.y(), B.z());
    glEnd();
}