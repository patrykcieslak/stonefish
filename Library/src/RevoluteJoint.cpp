//
//  RevoluteJoint.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 1/13/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#include "RevoluteJoint.h"

RevoluteJoint::RevoluteJoint(std::string uniqueName, SolidEntity* solidA, SolidEntity* solidB, const btVector3& pivot, const btVector3& axis, bool collideLinkedEntities) : Joint(uniqueName, collideLinkedEntities)
{
    btRigidBody* bodyA = solidA->getRigidBody();
    btRigidBody* bodyB = solidB->getRigidBody();
    btVector3 hingeAxis = axis.normalized();
    axisInA = bodyA->getCenterOfMassTransform().getBasis().inverse()*hingeAxis;
    btVector3 axisInB = bodyB->getCenterOfMassTransform().getBasis().inverse()*hingeAxis;
    
    btHingeConstraint* hinge = new btHingeConstraint(*bodyA, *bodyB, bodyA->getCenterOfMassTransform().inverse()(UnitSystem::SetPosition(pivot)), bodyB->getCenterOfMassTransform().inverse()(UnitSystem::SetPosition(pivot)), axisInA, axisInB);
    //hinge->setParam(BT_CONSTRAINT_CFM, CONSTRAINT_CFM);
    //hinge->setParam(BT_CONSTRAINT_STOP_ERP, CONSTRAINT_STOP_ERP);
    //hinge->setParam(BT_CONSTRAINT_STOP_CFM, CONSTRAINT_STOP_CFM);
    hinge->setLimit(1, -1); //no limit (min > max)
    setConstraint(hinge);
}

RevoluteJoint::~RevoluteJoint()
{
}

JointType RevoluteJoint::getType()
{
    return REVOLUTE;
}

void RevoluteJoint::Render()
{
    
}

btScalar RevoluteJoint::getAngle()
{
    return ((btHingeConstraint*)getConstraint())->getHingeAngle();
}

btScalar RevoluteJoint::getAngularVelocity()
{
    btRigidBody& bodyA = getConstraint()->getRigidBodyA();
    btRigidBody& bodyB = getConstraint()->getRigidBodyB();
    btVector3 relativeAV = bodyA.getAngularVelocity() - bodyB.getAngularVelocity();
    btVector3 hingeAxis = bodyA.getCenterOfMassTransform().getBasis() * axisInA;
    return hingeAxis.dot(relativeAV);
}

void RevoluteJoint::applyTorque(btScalar T)
{
    ((btHingeConstraint*)getConstraint())->enableAngularMotor(true, BT_LARGE_FLOAT, T);
}

void RevoluteJoint::setTargetVelocity(btScalar v, btScalar maxT)
{
    ((btHingeConstraint*)getConstraint())->enableAngularMotor(true, v, maxT);
}

btScalar RevoluteJoint::getTargetVelocity()
{
    return ((btHingeConstraint*)getConstraint())->getMotorTargetVelosity();
}

btVector3 RevoluteJoint::getAxis()
{
    return (getConstraint()->getRigidBodyA().getCenterOfMassTransform().getBasis() * axisInA).normalized();
}