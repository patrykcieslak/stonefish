//
//  SphericalJoint.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 2/3/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#include "SphericalJoint.h"

SphericalJoint::SphericalJoint(btRigidBody* bodyA, btRigidBody* bodyB, const btVector3& pivot) : Joint()
{
    btVector3 pivotInA = bodyA->getWorldTransform().inverse()*UnitSystem::SetPosition(pivot);
    btVector3 pivotInB = bodyB->getWorldTransform().inverse()*UnitSystem::SetPosition(pivot);
    btTransform frameInA = btTransform(btMatrix3x3().getIdentity(), pivotInA);
    btTransform frameInB = btTransform(btMatrix3x3().getIdentity(), pivotInB);
    
    btGeneric6DofConstraint* generic = new btGeneric6DofConstraint(*bodyA, *bodyB, frameInA, frameInB, true);
    //translations locked, rotations free
    generic->setLinearLowerLimit(btVector3(0,0,0));
    generic->setLinearUpperLimit(btVector3(0,0,0));
    generic->setAngularLowerLimit(btVector3(1,1,1));    //no limit
    generic->setAngularUpperLimit(btVector3(-1,-1,-1)); //
    
    generic->setParam(BT_CONSTRAINT_CFM, CONSTRAINT_CFM);
    generic->setParam(BT_CONSTRAINT_STOP_ERP, CONSTRAINT_STOP_ERP);
    generic->setParam(BT_CONSTRAINT_STOP_CFM, CONSTRAINT_STOP_CFM);
    
    setConstraint(generic);
}

SphericalJoint::~SphericalJoint()
{
}

JointType SphericalJoint::getType()
{
    return SPHERICAL;
}

void SphericalJoint::Render()
{
}