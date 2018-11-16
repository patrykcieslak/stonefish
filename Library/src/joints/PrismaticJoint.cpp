//
//  PrismaticJoint.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 27/03/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#include "joints/PrismaticJoint.h"

PrismaticJoint::PrismaticJoint(std::string uniqueName, SolidEntity* solidA, SolidEntity* solidB, const btVector3& axis, bool collideLinkedEntities) : Joint(uniqueName, collideLinkedEntities)
{
    btRigidBody* bodyA = solidA->getRigidBody();
    btRigidBody* bodyB = solidB->getRigidBody();
    
    btVector3 sliderAxis = axis.normalized();
    btVector3 v2;
    if(fabs(sliderAxis.z()) > 0.8) v2 = btVector3(1,0,0); else v2 = btVector3(0,0,1);
    btVector3 v3 = (sliderAxis.cross(v2)).normalized();
    v2 = (v3.cross(sliderAxis)).normalized();
    btMatrix3x3 sliderBasis(sliderAxis.x(), v2.x(), v3.x(),
                            sliderAxis.y(), v2.y(), v3.y(),
                            sliderAxis.z(), v2.z(), v3.z());
    btTransform sliderFrame(sliderBasis, (bodyA->getCenterOfMassPosition() + bodyB->getCenterOfMassPosition())/btScalar(2.));
    
    btTransform frameInA = bodyA->getCenterOfMassTransform().inverse() * sliderFrame;
    btTransform frameInB = bodyB->getCenterOfMassTransform().inverse() * sliderFrame;
    axisInA = frameInA.getBasis().getColumn(0).normalized();
    
    btSliderConstraint* slider = new btSliderConstraint(*bodyA, *bodyB, frameInA, frameInB, true);
    slider->setLowerLinLimit(1.);
    slider->setUpperLinLimit(-1.);
    slider->setLowerAngLimit(0.);
    slider->setUpperAngLimit(0.);
    setConstraint(slider);
    
    sigDamping = btScalar(0.);
    velDamping = btScalar(0.);
    
    displacementIC = btScalar(0.);
}

void PrismaticJoint::setDamping(btScalar constantFactor, btScalar viscousFactor)
{
    sigDamping = constantFactor > btScalar(0.) ? UnitSystem::SetForce(constantFactor) : btScalar(0.);
    velDamping = viscousFactor > btScalar(0.) ? viscousFactor : btScalar(0.);
}

void PrismaticJoint::setLimits(btScalar min, btScalar max)
{
    btSliderConstraint* slider = (btSliderConstraint*)getConstraint();
    slider->setLowerLinLimit(UnitSystem::SetLength(min));
    slider->setUpperLinLimit(UnitSystem::SetLength(max));
}

void PrismaticJoint::setIC(btScalar displacement)
{
    displacementIC = UnitSystem::SetLength(displacement);
}

JointType PrismaticJoint::getType()
{
    return JOINT_PRISMATIC;
}

void PrismaticJoint::ApplyForce(btScalar F)
{
    btRigidBody& bodyA = getConstraint()->getRigidBodyA();
    btRigidBody& bodyB = getConstraint()->getRigidBodyB();
    btVector3 axis = (bodyA.getCenterOfMassTransform().getBasis() * axisInA).normalized();
    btVector3 force = UnitSystem::SetForce(axis * F);
    bodyA.applyCentralForce(force);
    bodyB.applyCentralForce(-force);
}

void PrismaticJoint::ApplyDamping()
{
    if(sigDamping > btScalar(0.) || velDamping > btScalar(0.))
    {
        btRigidBody& bodyA = getConstraint()->getRigidBodyA();
        btRigidBody& bodyB = getConstraint()->getRigidBodyB();
        btVector3 axis = (bodyA.getCenterOfMassTransform().getBasis() * axisInA).normalized();
        btVector3 relativeV = bodyA.getLinearVelocity() - bodyB.getLinearVelocity();
        btScalar v = relativeV.dot(axis);
        
        if(v != btScalar(0.))
        {
            btScalar F = sigDamping * v/fabs(v) + velDamping * v;
            btVector3 force = axis * -F;
            
            bodyA.applyCentralForce(force);
            bodyB.applyCentralForce(-force);
        }
    }
}

bool PrismaticJoint::SolvePositionIC(btScalar linearTolerance, btScalar angularTolerance)
{
    return true;
}

btVector3 PrismaticJoint::Render()
{
    btTypedConstraint* slider = getConstraint();
    btVector3 A = slider->getRigidBodyA().getCenterOfMassPosition();
    btVector3 B = slider->getRigidBodyB().getCenterOfMassPosition();
    btVector3 pivot = (A+B)/btScalar(2.);
    btVector3 axis = (slider->getRigidBodyA().getCenterOfMassTransform().getBasis() * axisInA).normalized();
    
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
