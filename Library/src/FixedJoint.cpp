//
//  FixedJoint.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 2/4/13.
//  Copyright (c) 2013-2017 Patryk Cieslak. All rights reserved.
//

#include "FixedJoint.h"
#include <BulletDynamics/Featherstone/btMultiBodyFixedConstraint.h>
#include "SimulationApp.h"

FixedJoint::FixedJoint(std::string uniqueName, SolidEntity* solidA, SolidEntity* solidB) : Joint(uniqueName, false)
{
    btRigidBody* bodyA = solidA->getRigidBody();
    btRigidBody* bodyB = solidB->getRigidBody();
    btTransform frameInA = btTransform::getIdentity();
    btTransform frameInB = bodyB->getCenterOfMassTransform().inverse() * bodyA->getCenterOfMassTransform(); //CHECK IT!!!!!!!!!!
    
    btFixedConstraint* fixed = new btFixedConstraint(*bodyA, *bodyB, frameInA, frameInB);
    setConstraint(fixed);
}

FixedJoint::FixedJoint(std::string uniqueName, FeatherstoneEntity* feA, FeatherstoneEntity* feB, int linkIdA, int linkIdB) : Joint(uniqueName, false)
{
	btMultiBody* mbA = feA->getMultiBody();
	btMultiBody* mbB = feB->getMultiBody();
	btVector3 pivotInB = mbB->getBaseWorldTransform().inverse() * (mbA->getBaseWorldTransform() * btVector3(0,0,0));
	btMatrix3x3 frameInB = mbB->getBaseWorldTransform().getBasis().inverse() * mbA->getBaseWorldTransform().getBasis();	
	
	btMultiBodyFixedConstraint* fixed = new btMultiBodyFixedConstraint(mbA, linkIdA, mbB, linkIdB, btVector3(0,0,0), pivotInB, btMatrix3x3::getIdentity(), frameInB);
	setConstraint(fixed);
    
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