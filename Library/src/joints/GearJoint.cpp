//
//  GearJoint.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 28/03/2014.
//  Copyright (c) 2014-2018 Patryk Cieslak. All rights reserved.
//

#include "joints/GearJoint.h"

#include "entities/SolidEntity.h"

namespace sf
{

GearJoint::GearJoint(std::string uniqueName, SolidEntity* solidA, SolidEntity* solidB, const Vector3& axisA, const Vector3& axisB, Scalar ratio) : Joint(uniqueName, false)
{
    gearRatio = ratio;
    
    btRigidBody* bodyA = solidA->rigidBody;
    btRigidBody* bodyB = solidB->rigidBody;
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

std::vector<Renderable> GearJoint::Render()
{
    std::vector<Renderable> items(0);
    Renderable item;
    item.model = glm::mat4(1.f);
    item.type = RenderableType::JOINT_LINES;
    
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
    
    //Circle A
	for(unsigned short i=0; i<24; ++i)
	{
		Vector3 pd1 = A + rA.rotate(axisA, i/Scalar(12.) * M_PI) * rAn;
        Vector3 pd2 = A + rA.rotate(axisA, (i+1)/Scalar(12.) * M_PI) * rAn;
		glm::vec3 p1 = glm::vec3((GLfloat)pd1.getX(), (GLfloat)pd1.getY(), (GLfloat)pd1.getZ());
        glm::vec3 p2 = glm::vec3((GLfloat)pd2.getX(), (GLfloat)pd2.getY(), (GLfloat)pd2.getZ());
        item.points.push_back(p1);
        item.points.push_back(p2);
	}
	
    //Circle B
    for(unsigned short i=0; i<24; ++i)
	{
        Vector3 pd1 = B + rB.rotate(axisB, i/Scalar(12.) * M_PI) * rBn;
        Vector3 pd2 = B + rB.rotate(axisB, (i+1)/Scalar(12.) * M_PI) * rBn;
        glm::vec3 p1 = glm::vec3((GLfloat)pd1.getX(), (GLfloat)pd1.getY(), (GLfloat)pd1.getZ());
        glm::vec3 p2 = glm::vec3((GLfloat)pd2.getX(), (GLfloat)pd2.getY(), (GLfloat)pd2.getZ());
        item.points.push_back(p1);
        item.points.push_back(p2);
	}
    
    items.push_back(item);
    return items;
}

}
