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
    axisInA = bodyA->getCenterOfMassTransform().getBasis().inverse() * hingeAxis;
    btVector3 axisInB = bodyB->getCenterOfMassTransform().getBasis().inverse() * hingeAxis;
    pivotInA = bodyA->getCenterOfMassTransform().inverse()(UnitSystem::SetPosition(pivot));
    btVector3 pivotInB = bodyB->getCenterOfMassTransform().inverse()(UnitSystem::SetPosition(pivot));
    
    btHingeConstraint* hinge = new btHingeConstraint(*bodyA, *bodyB, pivotInA, pivotInB, axisInA, axisInB);
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

btVector3 RevoluteJoint::Render()
{
    btTypedConstraint* revo = getConstraint();
    btVector3 A = revo->getRigidBodyA().getCenterOfMassPosition();
    btVector3 B = revo->getRigidBodyB().getCenterOfMassPosition();
    btVector3 pivot =  revo->getRigidBodyA().getCenterOfMassTransform()(pivotInA);
    btVector3 axis = (revo->getRigidBodyA().getCenterOfMassTransform().getBasis() * axisInA).normalized();
    
    //calculate axis ends
    btScalar e1 = (A-pivot).dot(axis);
    btScalar e2 = (B-pivot).dot(axis);
    btVector3 C1 = pivot + e1 * axis;
    btVector3 C2 = pivot + e2 * axis;
    
    glDummyColor();
    //links
    glBegin(GL_LINES);
    glBulletVertex(A);
    glBulletVertex(C1);
    glBulletVertex(B);
    glBulletVertex(C2);
    glEnd();
    
    //axis
    glEnable(GL_LINE_STIPPLE);
    glBegin(GL_LINES);
    glBulletVertex(C1);
    glBulletVertex(C2);
    glEnd();
    glDisable(GL_LINE_STIPPLE);
    
    return (C1+C2)/btScalar(2.);
}

btScalar RevoluteJoint::getAngle()
{
    return UnitSystem::GetAngle(((btHingeConstraint*)getConstraint())->getHingeAngle());
}

btScalar RevoluteJoint::getAngularVelocity()
{
    btRigidBody& bodyA = getConstraint()->getRigidBodyA();
    btRigidBody& bodyB = getConstraint()->getRigidBodyB();
    btVector3 relativeAV = bodyA.getAngularVelocity() - bodyB.getAngularVelocity();
    btVector3 axis = (bodyA.getCenterOfMassTransform().getBasis() * axisInA).normalized();
    return UnitSystem::GetAngle(relativeAV.dot(axis));
}

void RevoluteJoint::applyTorque(btScalar T)
{
    ((btHingeConstraint*)getConstraint())->enableAngularMotor(true, BT_LARGE_FLOAT, UnitSystem::SetTorque(btVector3(T,0,0)).x());
}

void RevoluteJoint::setTargetVelocity(btScalar v, btScalar maxT)
{
    //Have to divide by 10 to get correct velocity???
    ((btHingeConstraint*)getConstraint())->enableAngularMotor(true, UnitSystem::SetAngle(v)/btScalar(10.), maxT);
}

btScalar RevoluteJoint::getTargetVelocity()
{
    return UnitSystem::GetAngle(((btHingeConstraint*)getConstraint())->getMotorTargetVelosity());
}