//
//  FixedJoint.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 2/4/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#include "FixedJoint.h"

FixedJoint::FixedJoint(btRigidBody* bodyA, btRigidBody* bodyB) : Joint()
{
    btVector3 pivotInB = bodyB->getWorldTransform().inverse() * bodyA->getWorldTransform().getOrigin();
    btMatrix3x3 rotationInB = bodyB->getWorldTransform().getBasis().inverse() * bodyA->getWorldTransform().getBasis();
    btTransform frameInA = btTransform(btMatrix3x3().getIdentity(), btVector3(0,0,0));
    btTransform frameInB = btTransform(rotationInB, pivotInB);
    
    btGeneric6DofConstraint* generic = new btGeneric6DofConstraint(*bodyA, *bodyB, frameInA, frameInB, true);
    //all locked
    generic->setLinearLowerLimit(btVector3(0,0,0));
    generic->setLinearUpperLimit(btVector3(0,0,0));
    generic->setAngularLowerLimit(btVector3(0,0,0));
    generic->setAngularUpperLimit(btVector3(0,0,0));
    
    generic->setParam(BT_CONSTRAINT_CFM, CONSTRAINT_CFM, 0);
    generic->setParam(BT_CONSTRAINT_STOP_ERP, CONSTRAINT_STOP_ERP, 0);
    generic->setParam(BT_CONSTRAINT_STOP_CFM, CONSTRAINT_STOP_CFM, 0);
    
    
    generic->setParam(BT_CONSTRAINT_CFM, CONSTRAINT_CFM, 1);
    generic->setParam(BT_CONSTRAINT_STOP_ERP, CONSTRAINT_STOP_ERP, 1);
    generic->setParam(BT_CONSTRAINT_STOP_CFM, CONSTRAINT_STOP_CFM, 1);
    
    generic->setParam(BT_CONSTRAINT_CFM, CONSTRAINT_CFM, 2);
    generic->setParam(BT_CONSTRAINT_STOP_ERP, CONSTRAINT_STOP_ERP, 2);
    generic->setParam(BT_CONSTRAINT_STOP_CFM, CONSTRAINT_STOP_CFM, 2);
    
    generic->setParam(BT_CONSTRAINT_CFM, CONSTRAINT_CFM, 3);
    generic->setParam(BT_CONSTRAINT_STOP_ERP, CONSTRAINT_STOP_ERP, 3);
    generic->setParam(BT_CONSTRAINT_STOP_CFM, CONSTRAINT_STOP_CFM, 3);
    
    generic->setParam(BT_CONSTRAINT_CFM, CONSTRAINT_CFM, 4);
    generic->setParam(BT_CONSTRAINT_STOP_ERP, CONSTRAINT_STOP_ERP, 4);
    generic->setParam(BT_CONSTRAINT_STOP_CFM, CONSTRAINT_STOP_CFM, 4);
    
    generic->setParam(BT_CONSTRAINT_CFM, CONSTRAINT_CFM, 5);
    generic->setParam(BT_CONSTRAINT_STOP_ERP, CONSTRAINT_STOP_ERP, 5);
    generic->setParam(BT_CONSTRAINT_STOP_CFM, CONSTRAINT_STOP_CFM, 5);
    
    setConstraint(generic);
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
}