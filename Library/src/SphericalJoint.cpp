//
//  SphericalJoint.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 2/3/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#include "SphericalJoint.h"

SphericalJoint::SphericalJoint(std::string uniqueName, SolidEntity* solidA, SolidEntity* solidB, const btVector3& pivot, bool collideLinkedEntities) : Joint(uniqueName, collideLinkedEntities)
{
    btRigidBody* bodyA = solidA->getRigidBody();
    btRigidBody* bodyB = solidB->getRigidBody();
    btVector3 pivotInA = bodyA->getCenterOfMassTransform().inverse()*UnitSystem::SetPosition(pivot);
    btVector3 pivotInB = bodyB->getCenterOfMassTransform().inverse()*UnitSystem::SetPosition(pivot);
    
    btPoint2PointConstraint* p2p = new btPoint2PointConstraint(*bodyA, *bodyB, pivotInA, pivotInB);
    setConstraint(p2p);
}

SphericalJoint::~SphericalJoint()
{
}

JointType SphericalJoint::getType()
{
    return SPHERICAL;
}

void SphericalJoint::ApplyDamping()
{
}

btVector3 SphericalJoint::Render()
{
    btPoint2PointConstraint* p2p = (btPoint2PointConstraint*)getConstraint();
    btVector3 pivot = p2p->getRigidBodyA().getCenterOfMassTransform()(p2p->getPivotInA());
    btVector3 A = p2p->getRigidBodyA().getCenterOfMassPosition();
    btVector3 B = p2p->getRigidBodyB().getCenterOfMassPosition();
    
    glDummyColor();
    glBegin(GL_LINES);
    glBulletVertex(A);
    glBulletVertex(pivot);
    glBulletVertex(B);
    glBulletVertex(pivot);
    glEnd();
    
    return pivot;
}