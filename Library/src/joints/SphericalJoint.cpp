//
//  SphericalJoint.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 2/3/13.
//  Copyright (c) 2013-2017 Patryk Cieslak. All rights reserved.
//

#include "joints/SphericalJoint.h"

using namespace sf;

SphericalJoint::SphericalJoint(std::string uniqueName, SolidEntity* solidA, SolidEntity* solidB, const Vector3& pivot, bool collideLinkedEntities) : Joint(uniqueName, collideLinkedEntities)
{
    btRigidBody* bodyA = solidA->getRigidBody();
    btRigidBody* bodyB = solidB->getRigidBody();
    Vector3 pivotInA = bodyA->getCenterOfMassTransform().inverse() * pivot;
    Vector3 pivotInB = bodyB->getCenterOfMassTransform().inverse() * pivot;
    
    btPoint2PointConstraint* p2p = new btPoint2PointConstraint(*bodyA, *bodyB, pivotInA, pivotInB);
    setConstraint(p2p);
    
    sigDamping = Vector3(0.,0.,0.);
    velDamping = Vector3(0.,0.,0.);
    
    angleIC = Vector3(0.,0.,0.);
}

void SphericalJoint::setDamping(Vector3 constantFactor, Vector3 viscousFactor)
{
    for(int i = 0; i < 3; i++)
    {
        sigDamping[i] = constantFactor[i] > Scalar(0.) ? constantFactor[i] : Scalar(0.);
        velDamping[i] = viscousFactor[i] > Scalar(0.) ? viscousFactor[i] : Scalar(0.);
    }
}

void SphericalJoint::setIC(Vector3 angles)
{
    angleIC = angles;
}

JointType SphericalJoint::getType()
{
    return JOINT_SPHERICAL;
}

void SphericalJoint::ApplyTorque(Vector3 T)
{
    btRigidBody& bodyA = getConstraint()->getRigidBodyA();
    btRigidBody& bodyB = getConstraint()->getRigidBodyB();
    Vector3 torque = T;
    bodyA.applyTorque(torque);
    bodyB.applyTorque(-torque);
}

void SphericalJoint::ApplyDamping()
{
    if(sigDamping.length2() > Scalar(0.) || velDamping.length2() > Scalar(0.))
    {
        btRigidBody& bodyA = getConstraint()->getRigidBodyA();
        btRigidBody& bodyB = getConstraint()->getRigidBodyB();
        Vector3 relativeAV = bodyA.getAngularVelocity() - bodyB.getAngularVelocity();
        Vector3 torque(0.,0.,0.);
        
        if(relativeAV.x() != Scalar(0.))
            torque[0] = -(sigDamping.x() * relativeAV.x()/fabs(relativeAV.x())) - velDamping.x() * relativeAV.x();
        if(relativeAV.y() != Scalar(0.))
            torque[1] = -(sigDamping.y() * relativeAV.y()/fabs(relativeAV.y())) - velDamping.y() * relativeAV.y();
        if(relativeAV.z() != Scalar(0.))
            torque[2] = -(sigDamping.z() * relativeAV.z()/fabs(relativeAV.z())) - velDamping.z() * relativeAV.z();
        
        bodyA.applyTorque(torque);
        bodyB.applyTorque(-torque);
    }
}

bool SphericalJoint::SolvePositionIC(Scalar linearTolerance, Scalar angularTolerance)
{
    return true;
}

Vector3 SphericalJoint::Render()
{
    btPoint2PointConstraint* p2p = (btPoint2PointConstraint*)getConstraint();
    Vector3 pivot = p2p->getRigidBodyA().getCenterOfMassTransform()(p2p->getPivotInA());
    Vector3 A = p2p->getRigidBodyA().getCenterOfMassPosition();
    Vector3 B = p2p->getRigidBodyB().getCenterOfMassPosition();
    
	std::vector<glm::vec3> vertices;
	vertices.push_back(glm::vec3(A.getX(), A.getY(), A.getZ()));
	vertices.push_back(glm::vec3(pivot.getX(), pivot.getY(), pivot.getZ()));
	vertices.push_back(glm::vec3(B.getX(), B.getY(), B.getZ()));
	vertices.push_back(glm::vec3(pivot.getX(), pivot.getY(), pivot.getZ()));
	OpenGLContent::getInstance()->DrawPrimitives(PrimitiveType::LINES, vertices, DUMMY_COLOR);
	
    return pivot;
}
