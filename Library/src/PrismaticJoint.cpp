//
//  PrismaticJoint.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 27/03/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#include "PrismaticJoint.h"

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
    slider->setLowerLinLimit(1);
    slider->setUpperLinLimit(-1);
    slider->setLowerAngLimit(0.0);
    slider->setUpperAngLimit(0.0);
    setConstraint(slider);
}

PrismaticJoint::~PrismaticJoint()
{
}

JointType PrismaticJoint::getType()
{
    return PRISMATIC;
}

void PrismaticJoint::Render()
{
    
}

btVector3 PrismaticJoint::getAxis()
{
    return (getConstraint()->getRigidBodyA().getCenterOfMassTransform().getBasis() * axisInA).normalized();
}