//
//  RevoluteJoint.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 1/13/13.
//  Copyright (c) 2013-2017 Patryk Cieslak. All rights reserved.
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
    hinge->setLimit(1.0, -1.0); //no limit (min > max)
    hinge->enableMotor(false);
    setConstraint(hinge);
    
    sigDamping = btScalar(0.);
    velDamping = btScalar(0.);
    
    angleIC = btScalar(0.);
}

RevoluteJoint::RevoluteJoint(std::string uniqueName, SolidEntity* solid, const btVector3& pivot, const btVector3& axis) : Joint(uniqueName, false)
{
    btRigidBody* body = solid->getRigidBody();
    btVector3 hingeAxis = axis.normalized();
    axisInA = body->getCenterOfMassTransform().getBasis().inverse() * hingeAxis;
    pivotInA = body->getCenterOfMassTransform().inverse()(UnitSystem::SetPosition(pivot));
    
    btHingeConstraint* hinge = new btHingeConstraint(*body, pivotInA, axisInA);
    hinge->setLimit(1.0, -1.0); //no limit (min > max)
    setConstraint(hinge);
    
    sigDamping = btScalar(0.);
    velDamping = btScalar(0.);
    
    angleIC = btScalar(0.);
}

void RevoluteJoint::setDamping(btScalar constantFactor, btScalar viscousFactor)
{
    sigDamping = constantFactor > btScalar(0.) ? UnitSystem::SetTorque(constantFactor) : btScalar(0.);
    velDamping = viscousFactor > btScalar(0.) ? viscousFactor : btScalar(0.);
}

void RevoluteJoint::setLimits(btScalar min, btScalar max)
{
    btHingeConstraint* hinge = (btHingeConstraint*)getConstraint();
    hinge->setLimit(UnitSystem::SetAngle(min), UnitSystem::SetAngle(max));
}

void RevoluteJoint::setIC(btScalar angle)
{
    angleIC = UnitSystem::SetAngle(angle);
}

JointType RevoluteJoint::getType()
{
    return JOINT_REVOLUTE;
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
    btVector3 axis = (bodyA.getCenterOfMassTransform().getBasis() * axisInA).normalized();
    return relativeAV.dot(axis);
}

void RevoluteJoint::ApplyTorque(btScalar T)
{
    btRigidBody& bodyA = getConstraint()->getRigidBodyA();
    btRigidBody& bodyB = getConstraint()->getRigidBodyB();
    btVector3 axis = (bodyA.getCenterOfMassTransform().getBasis() * axisInA).normalized();
    
    btVector3 torque = UnitSystem::SetTorque(axis * T);
    bodyA.applyTorque(torque);
    bodyB.applyTorque(-torque);
}

void RevoluteJoint::ApplyDamping()
{
    if(sigDamping > btScalar(0.) || velDamping > btScalar(0.))
    {
        btRigidBody& bodyA = getConstraint()->getRigidBodyA();
        btRigidBody& bodyB = getConstraint()->getRigidBodyB();
        btVector3 axis = (bodyA.getCenterOfMassTransform().getBasis() * axisInA).normalized();
        btVector3 relativeAV = bodyA.getAngularVelocity() - bodyB.getAngularVelocity();
        btScalar av = relativeAV.dot(axis);
       
        if(av != btScalar(0.))
        {
            btScalar T = sigDamping * av/fabs(av) + velDamping * av;
            btVector3 torque = axis * -T;
        
            bodyA.applyTorque(torque);
            bodyB.applyTorque(-torque);
        }
    }
}

bool RevoluteJoint::SolvePositionIC(btScalar linearTolerance, btScalar angularTolerance)
{
    btScalar angleError = angleIC - UnitSystem::SetAngle(getAngle());
    
    //Check if IC reached
    if(fabs(angleError) < angularTolerance)
        return true;
    
    //Move joint
    btScalar torque = angleError * btScalar(1000.) - getAngularVelocity() * btScalar(2000.0);
    ApplyTorque(UnitSystem::GetTorque(torque));
    
    return false;
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
    
    std::vector<glm::vec3> vertices;
	vertices.push_back(glm::vec3(A.getX(), A.getY(), A.getZ()));
	vertices.push_back(glm::vec3(C1.getX(), C1.getY(), C1.getZ()));
	vertices.push_back(glm::vec3(B.getX(), B.getY(), B.getZ()));
	vertices.push_back(glm::vec3(C2.getX(), C2.getY(), C2.getZ()));
	OpenGLContent::getInstance()->DrawPrimitives(PrimitiveType::LINES, vertices, DUMMY_COLOR);
	vertices.clear();
	
	vertices.push_back(glm::vec3(C1.getX(), C1.getY(), C1.getZ()));
	vertices.push_back(glm::vec3(C2.getX(), C2.getY(), C2.getZ()));
	glEnable(GL_LINE_STIPPLE);
    OpenGLContent::getInstance()->DrawPrimitives(PrimitiveType::LINES, vertices, DUMMY_COLOR);
	glDisable(GL_LINE_STIPPLE);
    
    return (C1+C2)/btScalar(2.);
}