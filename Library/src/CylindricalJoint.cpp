//
//  CylindricalJoint.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 28/03/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#include <joints/CylindricalJoint.h>

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
    
    linSigDamping = btScalar(0.);
    linVelDamping = btScalar(0.);
    angSigDamping = btScalar(0.);
    angVelDamping = btScalar(0.);
    
    displacementIC = btScalar(0.);
    angleIC = btScalar(0.);
}

void CylindricalJoint::setDamping(btScalar linearConstantFactor, btScalar linearViscousFactor, btScalar angularConstantFactor, btScalar angularViscousFactor)
{
    linSigDamping = linearConstantFactor > btScalar(0.) ? UnitSystem::SetForce(linearConstantFactor) : btScalar(0.);
    linVelDamping = linearViscousFactor > btScalar(0.) ? linearViscousFactor : btScalar(0.);
    angSigDamping = angularConstantFactor > btScalar(0.) ? UnitSystem::SetTorque(angularConstantFactor) : btScalar(0.);
    angVelDamping = angularViscousFactor > btScalar(0.) ? angularViscousFactor : btScalar(0.);
}

void CylindricalJoint::setLimits(btScalar linearMin, btScalar linearMax, btScalar angularMin, btScalar angularMax)
{
    btSliderConstraint* slider = (btSliderConstraint*)getConstraint();
    slider->setLowerLinLimit(UnitSystem::SetLength(linearMin));
    slider->setUpperLinLimit(UnitSystem::SetLength(linearMax));
    slider->setLowerAngLimit(UnitSystem::SetAngle(angularMin));
    slider->setUpperAngLimit(UnitSystem::SetAngle(angularMax));
}

void CylindricalJoint::setIC(btScalar displacement, btScalar angle)
{
    displacementIC = UnitSystem::SetLength(displacement);
    angleIC = UnitSystem::SetAngle(angle);
}

JointType CylindricalJoint::getType()
{
    return JOINT_CYLINDRICAL;
}

void CylindricalJoint::ApplyForce(btScalar F)
{
    btRigidBody& bodyA = getConstraint()->getRigidBodyA();
    btRigidBody& bodyB = getConstraint()->getRigidBodyB();
    btVector3 axis = (bodyA.getCenterOfMassTransform().getBasis() * axisInA).normalized();
    btVector3 force = UnitSystem::SetForce(axis * F);
    bodyA.applyCentralForce(force);
    bodyB.applyCentralForce(-force);
}

void CylindricalJoint::ApplyTorque(btScalar T)
{
    btRigidBody& bodyA = getConstraint()->getRigidBodyA();
    btRigidBody& bodyB = getConstraint()->getRigidBodyB();
    btVector3 axis = (bodyA.getCenterOfMassTransform().getBasis() * axisInA).normalized();
    btVector3 torque = UnitSystem::SetTorque(axis * T);
    bodyA.applyTorque(torque);
    bodyB.applyTorque(-torque);
}

void CylindricalJoint::ApplyDamping()
{
    if(linSigDamping > btScalar(0.) || linVelDamping > btScalar(0.) || angSigDamping > btScalar(0.) || angVelDamping > btScalar(0.))
    {
        btRigidBody& bodyA = getConstraint()->getRigidBodyA();
        btRigidBody& bodyB = getConstraint()->getRigidBodyB();
        btVector3 axis = (bodyA.getCenterOfMassTransform().getBasis() * axisInA).normalized();
        btVector3 relativeV = bodyA.getLinearVelocity() - bodyB.getLinearVelocity();
        btScalar v = relativeV.dot(axis);
        btVector3 relativeAV = bodyA.getAngularVelocity() - bodyB.getAngularVelocity();
        btScalar av = relativeAV.dot(axis);
        
        if(v != btScalar(0.))
        {
            btScalar F = linSigDamping * v/fabs(v) + linVelDamping * v;
            btVector3 force = axis * -F;
            
            bodyA.applyCentralForce(force);
            bodyB.applyCentralForce(-force);
        }
        
        if(av != btScalar(0.))
        {
            btScalar T = angSigDamping * av/fabs(av) + angVelDamping * av;
            btVector3 torque = axis * -T;
            
            bodyA.applyTorque(torque);
            bodyB.applyTorque(-torque);
        }
    }
}

bool CylindricalJoint::SolvePositionIC(btScalar linearTolerance, btScalar angularTolerance)
{
    return true;
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