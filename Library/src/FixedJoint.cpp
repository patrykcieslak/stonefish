//
//  FixedJoint.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 2/4/13.
//  Copyright (c) 2013-2017 Patryk Cieslak. All rights reserved.
//

#include <joints/FixedJoint.h>

#include <core/SimulationApp.h>
#include <BulletDynamics/Featherstone/btMultiBodyFixedConstraint.h>

FixedJoint::FixedJoint(std::string uniqueName, SolidEntity* solidA, SolidEntity* solidB) : Joint(uniqueName, false)
{
    btRigidBody* bodyA = solidA->getRigidBody();
    btRigidBody* bodyB = solidB->getRigidBody();
    btTransform frameInA = btTransform::getIdentity();
    btTransform frameInB = bodyB->getCenterOfMassTransform().inverse() * bodyA->getCenterOfMassTransform(); //CHECK IT!!!!!!!!!!
    
    btFixedConstraint* fixed = new btFixedConstraint(*bodyA, *bodyB, frameInA, frameInB);
    setConstraint(fixed);
}

FixedJoint::FixedJoint(std::string uniqueName, FeatherstoneEntity* feA, FeatherstoneEntity* feB, int linkIdA, int linkIdB, const btVector3& pivot) : Joint(uniqueName, false)
{
    btTransform linkATransform = feA->getLinkTransform(linkIdA+1);
    btTransform linkBTransform = feB->getLinkTransform(linkIdB+1);
    btVector3 pivotInA = linkATransform.inverse() * pivot;
    btVector3 pivotInB = linkBTransform.inverse() * pivot;
    btMatrix3x3 frameInB = linkBTransform.getBasis().inverse() * linkATransform.getBasis();	
	
	btMultiBodyFixedConstraint* fixed = new btMultiBodyFixedConstraint(feA->getMultiBody(), linkIdA, feB->getMultiBody(), linkIdB, pivotInA, pivotInB, btMatrix3x3::getIdentity(), frameInB);
	setConstraint(fixed);
    fixed->setMaxAppliedImpulse(BT_LARGE_FLOAT);

    //Disable collision
    SimulationApp::getApp()->getSimulationManager()->DisableCollision(feA->getLink(linkIdA+1).solid, feB->getLink(linkIdB+1).solid);
}

JointType FixedJoint::getType()
{
    return JOINT_FIXED;
}

btVector3 FixedJoint::Render()
{
    return btVector3();
}