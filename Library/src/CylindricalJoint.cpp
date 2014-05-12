//
//  CylindricalJoint.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 28/03/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#include "CylindricalJoint.h"

CylindricalJoint::CylindricalJoint(std::string uniqueName, SolidEntity* solidA, SolidEntity* solidB, const btVector3& pivot, const btVector3& axis, bool collideLinkedEntities) : Joint(uniqueName, collideLinkedEntities)
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
    btTransform sliderFrame(sliderBasis, UnitSystem::SetPosition(pivot));
    
    btTransform frameInA = bodyA->getCenterOfMassTransform().inverse() * sliderFrame;
    btTransform frameInB = bodyB->getCenterOfMassTransform().inverse() * sliderFrame;
    axisInA = frameInA.getBasis().getColumn(0).normalized();
    pivotInA = frameInA.getOrigin();
    
    btSliderConstraint* slider = new btSliderConstraint(*bodyA, *bodyB, frameInA, frameInB, true);
    slider->setLowerLinLimit(1.);
    slider->setUpperLinLimit(-1.);
    slider->setLowerAngLimit(1.);
    slider->setUpperAngLimit(-1.);
    setConstraint(slider);
}

CylindricalJoint::~CylindricalJoint()
{
}

JointType CylindricalJoint::getType()
{
    return CYLINDRICAL;
}

void CylindricalJoint::ApplyDamping()
{
}

btVector3 CylindricalJoint::Render()
{
    btTypedConstraint* cyli = getConstraint();
    btVector3 A = cyli->getRigidBodyA().getCenterOfMassPosition();
    btVector3 B = cyli->getRigidBodyB().getCenterOfMassPosition();
    btVector3 pivot = cyli->getRigidBodyA().getCenterOfMassTransform()(pivotInA);
    btVector3 axis = (cyli->getRigidBodyA().getCenterOfMassTransform().getBasis() * axisInA).normalized();
    
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