//
//  SphericalJoint.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 2/3/13.
//  Copyright (c) 2013-2017 Patryk Cieslak. All rights reserved.
//

#include "SphericalJoint.h"

SphericalJoint::SphericalJoint(std::string uniqueName, SolidEntity* solidA, SolidEntity* solidB, const btVector3& pivot, bool collideLinkedEntities) : Joint(uniqueName, collideLinkedEntities)
{
    btRigidBody* bodyA = solidA->getRigidBody();
    btRigidBody* bodyB = solidB->getRigidBody();
    btVector3 pivotInA = bodyA->getCenterOfMassTransform().inverse() * UnitSystem::SetPosition(pivot);
    btVector3 pivotInB = bodyB->getCenterOfMassTransform().inverse() * UnitSystem::SetPosition(pivot);
    
    btPoint2PointConstraint* p2p = new btPoint2PointConstraint(*bodyA, *bodyB, pivotInA, pivotInB);
    setConstraint(p2p);
    
    sigDamping = btVector3(0.,0.,0.);
    velDamping = btVector3(0.,0.,0.);
    
    angleIC = btVector3(0.,0.,0.);
}

void SphericalJoint::setDamping(btVector3 constantFactor, btVector3 viscousFactor)
{
    for(int i = 0; i < 3; i++)
    {
        sigDamping[i] = constantFactor[i] > btScalar(0.) ? UnitSystem::SetTorque(constantFactor[i]) : btScalar(0.);
        velDamping[i] = viscousFactor[i] > btScalar(0.) ? viscousFactor[i] : btScalar(0.);
    }
}

void SphericalJoint::setIC(btVector3 angles)
{
    angleIC = UnitSystem::SetAngularVelocity(angles);
}

JointType SphericalJoint::getType()
{
    return JOINT_SPHERICAL;
}

void SphericalJoint::ApplyTorque(btVector3 T)
{
    btRigidBody& bodyA = getConstraint()->getRigidBodyA();
    btRigidBody& bodyB = getConstraint()->getRigidBodyB();
    btVector3 torque = UnitSystem::SetTorque(T);
    bodyA.applyTorque(torque);
    bodyB.applyTorque(-torque);
}

void SphericalJoint::ApplyDamping()
{
    if(sigDamping.length2() > btScalar(0.) || velDamping.length2() > btScalar(0.))
    {
        btRigidBody& bodyA = getConstraint()->getRigidBodyA();
        btRigidBody& bodyB = getConstraint()->getRigidBodyB();
        btVector3 relativeAV = bodyA.getAngularVelocity() - bodyB.getAngularVelocity();
        btVector3 torque(0.,0.,0.);
        
        if(relativeAV.x() != btScalar(0.))
            torque[0] = -(sigDamping.x() * relativeAV.x()/fabs(relativeAV.x())) - velDamping.x() * relativeAV.x();
        if(relativeAV.y() != btScalar(0.))
            torque[1] = -(sigDamping.y() * relativeAV.y()/fabs(relativeAV.y())) - velDamping.y() * relativeAV.y();
        if(relativeAV.z() != btScalar(0.))
            torque[2] = -(sigDamping.z() * relativeAV.z()/fabs(relativeAV.z())) - velDamping.z() * relativeAV.z();
        
        bodyA.applyTorque(torque);
        bodyB.applyTorque(-torque);
    }
}

bool SphericalJoint::SolvePositionIC(btScalar linearTolerance, btScalar angularTolerance)
{
    return true;
}

btVector3 SphericalJoint::Render()
{
    btPoint2PointConstraint* p2p = (btPoint2PointConstraint*)getConstraint();
    btVector3 pivot = p2p->getRigidBodyA().getCenterOfMassTransform()(p2p->getPivotInA());
    btVector3 A = p2p->getRigidBodyA().getCenterOfMassPosition();
    btVector3 B = p2p->getRigidBodyB().getCenterOfMassPosition();
    
	std::vector<glm::vec3> vertices;
	vertices.push_back(glm::vec3(A.getX(), A.getY(), A.getZ()));
	vertices.push_back(glm::vec3(pivot.getX(), pivot.getY(), pivot.getZ()));
	vertices.push_back(glm::vec3(B.getX(), B.getY(), B.getZ()));
	vertices.push_back(glm::vec3(pivot.getX(), pivot.getY(), pivot.getZ()));
	OpenGLContent::getInstance()->DrawPrimitives(PrimitiveType::LINES, vertices, DUMMY_COLOR);
	
    return pivot;
}
