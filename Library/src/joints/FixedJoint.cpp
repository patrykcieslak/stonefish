//
//  FixedJoint.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 2/4/13.
//  Copyright (c) 2013-2018 Patryk Cieslak. All rights reserved.
//

#include "joints/FixedJoint.h"

#include <BulletDynamics/Featherstone/btMultiBodyFixedConstraint.h>
#include "core/SimulationApp.h"
#include "core/SimulationManager.h"
#include "entities/SolidEntity.h"
#include "entities/FeatherstoneEntity.h"

namespace sf
{

FixedJoint::FixedJoint(std::string uniqueName, SolidEntity* solidA, SolidEntity* solidB) : Joint(uniqueName, false)
{
    btRigidBody* bodyA = solidA->rigidBody;
    btRigidBody* bodyB = solidB->rigidBody;
    Transform frameInA = Transform::getIdentity();
    Transform frameInB = bodyB->getCenterOfMassTransform().inverse() * bodyA->getCenterOfMassTransform(); //CHECK IT!!!!!!!!!!
    
    btFixedConstraint* fixed = new btFixedConstraint(*bodyA, *bodyB, frameInA, frameInB);
    setConstraint(fixed);
}

FixedJoint::FixedJoint(std::string uniqueName, FeatherstoneEntity* feA, FeatherstoneEntity* feB, int linkIdA, int linkIdB, const Vector3& pivot) : Joint(uniqueName, false)
{
    Transform linkATransform = feA->getLinkTransform(linkIdA+1);
    Transform linkBTransform = feB->getLinkTransform(linkIdB+1);
    Vector3 pivotInA = linkATransform.inverse() * pivot;
    Vector3 pivotInB = linkBTransform.inverse() * pivot;
    Matrix3 frameInB = linkBTransform.getBasis().inverse() * linkATransform.getBasis();	
	
	btMultiBodyFixedConstraint* fixed = new btMultiBodyFixedConstraint(feA->getMultiBody(), linkIdA, feB->getMultiBody(), linkIdB, pivotInA, pivotInB, Matrix3::getIdentity(), frameInB);
	setConstraint(fixed);
    fixed->setMaxAppliedImpulse(BT_LARGE_FLOAT);

    //Disable collision
    SimulationApp::getApp()->getSimulationManager()->DisableCollision(feA->getLink(linkIdA+1).solid, feB->getLink(linkIdB+1).solid);
}

JointType FixedJoint::getType()
{
    return JOINT_FIXED;
}

}
