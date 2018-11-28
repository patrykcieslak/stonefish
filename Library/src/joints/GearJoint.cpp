//
//  GearJoint.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 28/03/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#include "joints/GearJoint.h"

using namespace sf;

GearJoint::GearJoint(std::string uniqueName, SolidEntity* solidA, SolidEntity* solidB, const Vector3& axisA, const Vector3& axisB, Scalar ratio) : Joint(uniqueName, false)
{
    gearRatio = ratio;
    
    btRigidBody* bodyA = solidA->getRigidBody();
    btRigidBody* bodyB = solidB->getRigidBody();
    Vector3 axisInA = bodyA->getCenterOfMassTransform().getBasis().inverse() * axisA.normalized();
    Vector3 axisInB = bodyB->getCenterOfMassTransform().getBasis().inverse() * axisB.normalized();
    
    btGearConstraint* gear = new btGearConstraint(*bodyA, *bodyB, axisInA, axisInB, gearRatio);
    setConstraint(gear);
}

JointType GearJoint::getType()
{
    return JOINT_GEAR;
}

Scalar GearJoint::getRatio()
{
    return gearRatio;
}

Vector3 GearJoint::Render()
{
    btGearConstraint* gear = (btGearConstraint*)getConstraint();
    Vector3 axisA = gear->getRigidBodyA().getCenterOfMassTransform().getBasis() * gear->getAxisA();
    Vector3 axisB = gear->getRigidBodyB().getCenterOfMassTransform().getBasis() * gear->getAxisB();
    Vector3 A = gear->getRigidBodyA().getCenterOfMassPosition();
    Vector3 B = gear->getRigidBodyB().getCenterOfMassPosition();
    Vector3 rA = ((B-A).cross(axisA)).normalized();
    Vector3 rB = ((A-B).cross(axisB)).normalized();
    Vector3 rBp = axisB.cross(rB).normalized();
    Scalar f1 = rBp.dot(axisA);
    Scalar f2 = (B-A).dot(axisA);
    Scalar rAn;
    Scalar rBn;
    
    if(f1 == 0)
    {
        if(axisA.dot(axisB) < 0.001f) // perpendicular
            rBn = (A-B).dot(rBp);
        else
            rBn = (A-B).length()/(gearRatio + Scalar(1.f)) * gearRatio;
    }
    else
        rBn = f2/f1;
    
    rAn = rBn/gearRatio;
    
    //circle A
	std::vector<glm::vec3> vertices;
	for(unsigned short i=0; i<=24; ++i)
	{
		Vector3 pd = A + rA.rotate(axisA, i/Scalar(12.) * M_PI) * rAn;
		glm::vec3 p = glm::vec3((GLfloat)pd.getX(), (GLfloat)pd.getY(), (GLfloat)pd.getZ()); 
		vertices.push_back(p);
	}
	OpenGLContent::getInstance()->DrawPrimitives(PrimitiveType::LINE_STRIP, vertices, DUMMY_COLOR);
	vertices.clear();
	
    //circle B
    for(unsigned short i=0; i<=24; ++i)
	{
		Vector3 pd = B + rB.rotate(axisB, i/Scalar(12.) * M_PI) * rBn;
		glm::vec3 p = glm::vec3((GLfloat)pd.getX(), (GLfloat)pd.getY(), (GLfloat)pd.getZ()); 
		vertices.push_back(p);
	}
	OpenGLContent::getInstance()->DrawPrimitives(PrimitiveType::LINE_STRIP, vertices, DUMMY_COLOR);
	
    return B + rBn*rBp;
}
