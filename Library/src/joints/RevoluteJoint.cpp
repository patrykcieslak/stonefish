//
//  RevoluteJoint.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 1/13/13.
//  Copyright (c) 2013-2017 Patryk Cieslak. All rights reserved.
//

#include "joints/RevoluteJoint.h"

using namespace sf;

RevoluteJoint::RevoluteJoint(std::string uniqueName, SolidEntity* solidA, SolidEntity* solidB, const Vector3& pivot, const Vector3& axis, bool collideLinkedEntities) : 
		RevoluteJoint(uniqueName, solidA->getRigidBody(), solidB->getRigidBody(), pivot, axis, collideLinkedEntities)
{
}

RevoluteJoint::RevoluteJoint(std::string uniqueName, SolidEntity* solid, const Vector3& pivot, const Vector3& axis) : Joint(uniqueName, false)
{
    btRigidBody* body = solid->getRigidBody();
    Vector3 hingeAxis = axis.normalized();
    axisInA = body->getCenterOfMassTransform().getBasis().inverse() * hingeAxis;
    pivotInA = body->getCenterOfMassTransform().inverse()(pivot);
    
    btHingeConstraint* hinge = new btHingeConstraint(*body, pivotInA, axisInA);
    hinge->setLimit(1.0, -1.0); //no limit (min > max)
    setConstraint(hinge);
    
    sigDamping = Scalar(0.);
    velDamping = Scalar(0.);
    
    angleIC = Scalar(0.);
}

RevoluteJoint::RevoluteJoint(std::string uniqueName, btRigidBody* bodyA, btRigidBody* bodyB, const Vector3& pivot, const Vector3& axis, bool collideLinkedEntities) : Joint(uniqueName, collideLinkedEntities)
{
	Vector3 hingeAxis = axis.normalized();
    axisInA = bodyA->getCenterOfMassTransform().getBasis().inverse() * hingeAxis;
    Vector3 axisInB = bodyB->getCenterOfMassTransform().getBasis().inverse() * hingeAxis;
    pivotInA = bodyA->getCenterOfMassTransform().inverse()(pivot);
    Vector3 pivotInB = bodyB->getCenterOfMassTransform().inverse()(pivot);
    
    btHingeConstraint* hinge = new btHingeConstraint(*bodyA, *bodyB, pivotInA, pivotInB, axisInA, axisInB);
    hinge->setLimit(1.0, -1.0); //no limit (min > max)
    hinge->enableMotor(false);
    setConstraint(hinge);
    
    sigDamping = Scalar(0.);
    velDamping = Scalar(0.);
    
    angleIC = Scalar(0.);
}	

void RevoluteJoint::setDamping(Scalar constantFactor, Scalar viscousFactor)
{
    sigDamping = constantFactor > Scalar(0) ? constantFactor : Scalar(0);
    velDamping = viscousFactor > Scalar(0) ? viscousFactor : Scalar(0);
}

void RevoluteJoint::setLimits(Scalar min, Scalar max)
{
    btHingeConstraint* hinge = (btHingeConstraint*)getConstraint();
    hinge->setLimit(min, max);
}

void RevoluteJoint::setIC(Scalar angle)
{
    angleIC = angle;
}

JointType RevoluteJoint::getType()
{
    return JOINT_REVOLUTE;
}

Scalar RevoluteJoint::getAngle()
{
    return ((btHingeConstraint*)getConstraint())->getHingeAngle();
}

Scalar RevoluteJoint::getAngularVelocity()
{
    btRigidBody& bodyA = getConstraint()->getRigidBodyA();
    btRigidBody& bodyB = getConstraint()->getRigidBodyB();
    Vector3 relativeAV = bodyA.getAngularVelocity() - bodyB.getAngularVelocity();
    Vector3 axis = (bodyA.getCenterOfMassTransform().getBasis() * axisInA).normalized();
    return relativeAV.dot(axis);
}

void RevoluteJoint::ApplyTorque(Scalar T)
{
    btRigidBody& bodyA = getConstraint()->getRigidBodyA();
    btRigidBody& bodyB = getConstraint()->getRigidBodyB();
    Vector3 axis = (bodyA.getCenterOfMassTransform().getBasis() * axisInA).normalized();
    
    Vector3 torque = axis * T;
    bodyA.applyTorque(torque);
    bodyB.applyTorque(-torque);
}

void RevoluteJoint::ApplyDamping()
{
    if(sigDamping > Scalar(0.) || velDamping > Scalar(0.))
    {
        btRigidBody& bodyA = getConstraint()->getRigidBodyA();
        btRigidBody& bodyB = getConstraint()->getRigidBodyB();
        Vector3 axis = (bodyA.getCenterOfMassTransform().getBasis() * axisInA).normalized();
        Vector3 relativeAV = bodyA.getAngularVelocity() - bodyB.getAngularVelocity();
        Scalar av = relativeAV.dot(axis);
       
        if(av != Scalar(0.))
        {
            Scalar T = sigDamping * av/fabs(av) + velDamping * av;
            Vector3 torque = axis * -T;
        
            bodyA.applyTorque(torque);
            bodyB.applyTorque(-torque);
        }
    }
}

bool RevoluteJoint::SolvePositionIC(Scalar linearTolerance, Scalar angularTolerance)
{
    Scalar angleError = angleIC - getAngle();
    
    //Check if IC reached
    if(fabs(angleError) < angularTolerance)
        return true;
    
    //Move joint
    Scalar torque = angleError * Scalar(1000.) - getAngularVelocity() * Scalar(2000.0);
    ApplyTorque(torque);
    
    return false;
}

Vector3 RevoluteJoint::Render()
{
    btTypedConstraint* revo = getConstraint();
    Vector3 A = revo->getRigidBodyA().getCenterOfMassPosition();
    Vector3 B = revo->getRigidBodyB().getCenterOfMassPosition();
    Vector3 pivot =  revo->getRigidBodyA().getCenterOfMassTransform()(pivotInA);
    Vector3 axis = (revo->getRigidBodyA().getCenterOfMassTransform().getBasis() * axisInA).normalized();
    
    //calculate axis ends
    Scalar e1 = (A-pivot).dot(axis);
    Scalar e2 = (B-pivot).dot(axis);
    Vector3 C1 = pivot + e1 * axis;
    Vector3 C2 = pivot + e2 * axis;
    
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
    
    return (C1+C2)/Scalar(2.);
}
