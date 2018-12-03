//
//  CylindricalJoint.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 28/03/2014.
//  Copyright (c) 2014-2018 Patryk Cieslak. All rights reserved.
//

#include "joints/CylindricalJoint.h"

#include "entities/SolidEntity.h"

namespace sf
{

CylindricalJoint::CylindricalJoint(std::string uniqueName, SolidEntity* solidA, SolidEntity* solidB, const Vector3& pivot, const Vector3& axis, bool collideLinkedEntities) : Joint(uniqueName, collideLinkedEntities)
{
    btRigidBody* bodyA = solidA->rigidBody;
    btRigidBody* bodyB = solidB->rigidBody;
    
    Vector3 sliderAxis = axis.normalized();
    Vector3 v2;
    if(fabs(sliderAxis.z()) > 0.8) v2 = Vector3(1,0,0); else v2 = Vector3(0,0,1);
    Vector3 v3 = (sliderAxis.cross(v2)).normalized();
    v2 = (v3.cross(sliderAxis)).normalized();
    Matrix3 sliderBasis(sliderAxis.x(), v2.x(), v3.x(),
                            sliderAxis.y(), v2.y(), v3.y(),
                            sliderAxis.z(), v2.z(), v3.z());
    Transform sliderFrame(sliderBasis, pivot);
    
    Transform frameInA = bodyA->getCenterOfMassTransform().inverse() * sliderFrame;
    Transform frameInB = bodyB->getCenterOfMassTransform().inverse() * sliderFrame;
    axisInA = frameInA.getBasis().getColumn(0).normalized();
    pivotInA = frameInA.getOrigin();
    
    btSliderConstraint* slider = new btSliderConstraint(*bodyA, *bodyB, frameInA, frameInB, true);
    slider->setLowerLinLimit(1.);
    slider->setUpperLinLimit(-1.);
    slider->setLowerAngLimit(1.);
    slider->setUpperAngLimit(-1.);
    setConstraint(slider);
    
    linSigDamping = Scalar(0.);
    linVelDamping = Scalar(0.);
    angSigDamping = Scalar(0.);
    angVelDamping = Scalar(0.);
    
    displacementIC = Scalar(0.);
    angleIC = Scalar(0.);
}

void CylindricalJoint::setDamping(Scalar linearConstantFactor, Scalar linearViscousFactor, Scalar angularConstantFactor, Scalar angularViscousFactor)
{
    linSigDamping = linearConstantFactor > Scalar(0) ? linearConstantFactor : Scalar(0);
    linVelDamping = linearViscousFactor > Scalar(0) ? linearViscousFactor : Scalar(0);
    angSigDamping = angularConstantFactor > Scalar(0) ? angularConstantFactor : Scalar(0);
    angVelDamping = angularViscousFactor > Scalar(0) ? angularViscousFactor : Scalar(0);
}

void CylindricalJoint::setLimits(Scalar linearMin, Scalar linearMax, Scalar angularMin, Scalar angularMax)
{
    btSliderConstraint* slider = (btSliderConstraint*)getConstraint();
    slider->setLowerLinLimit(linearMin);
    slider->setUpperLinLimit(linearMax);
    slider->setLowerAngLimit(angularMin);
    slider->setUpperAngLimit(angularMax);
}

void CylindricalJoint::setIC(Scalar displacement, Scalar angle)
{
    displacementIC = displacement;
    angleIC = angle;
}

JointType CylindricalJoint::getType()
{
    return JOINT_CYLINDRICAL;
}

void CylindricalJoint::ApplyForce(Scalar F)
{
    btRigidBody& bodyA = getConstraint()->getRigidBodyA();
    btRigidBody& bodyB = getConstraint()->getRigidBodyB();
    Vector3 axis = (bodyA.getCenterOfMassTransform().getBasis() * axisInA).normalized();
    Vector3 force = axis * F;
    bodyA.applyCentralForce(force);
    bodyB.applyCentralForce(-force);
}

void CylindricalJoint::ApplyTorque(Scalar T)
{
    btRigidBody& bodyA = getConstraint()->getRigidBodyA();
    btRigidBody& bodyB = getConstraint()->getRigidBodyB();
    Vector3 axis = (bodyA.getCenterOfMassTransform().getBasis() * axisInA).normalized();
    Vector3 torque = axis * T;
    bodyA.applyTorque(torque);
    bodyB.applyTorque(-torque);
}

void CylindricalJoint::ApplyDamping()
{
    if(linSigDamping > Scalar(0.) || linVelDamping > Scalar(0.) || angSigDamping > Scalar(0.) || angVelDamping > Scalar(0.))
    {
        btRigidBody& bodyA = getConstraint()->getRigidBodyA();
        btRigidBody& bodyB = getConstraint()->getRigidBodyB();
        Vector3 axis = (bodyA.getCenterOfMassTransform().getBasis() * axisInA).normalized();
        Vector3 relativeV = bodyA.getLinearVelocity() - bodyB.getLinearVelocity();
        Scalar v = relativeV.dot(axis);
        Vector3 relativeAV = bodyA.getAngularVelocity() - bodyB.getAngularVelocity();
        Scalar av = relativeAV.dot(axis);
        
        if(v != Scalar(0.))
        {
            Scalar F = linSigDamping * v/fabs(v) + linVelDamping * v;
            Vector3 force = axis * -F;
            
            bodyA.applyCentralForce(force);
            bodyB.applyCentralForce(-force);
        }
        
        if(av != Scalar(0.))
        {
            Scalar T = angSigDamping * av/fabs(av) + angVelDamping * av;
            Vector3 torque = axis * -T;
            
            bodyA.applyTorque(torque);
            bodyB.applyTorque(-torque);
        }
    }
}

bool CylindricalJoint::SolvePositionIC(Scalar linearTolerance, Scalar angularTolerance)
{
    return true;
}

Vector3 CylindricalJoint::Render()
{
    btTypedConstraint* cyli = getConstraint();
    Vector3 A = cyli->getRigidBodyA().getCenterOfMassPosition();
    Vector3 B = cyli->getRigidBodyB().getCenterOfMassPosition();
    Vector3 pivot = cyli->getRigidBodyA().getCenterOfMassTransform()(pivotInA);
    Vector3 axis = (cyli->getRigidBodyA().getCenterOfMassTransform().getBasis() * axisInA).normalized();
    
    //calculate axis ends
    Scalar e1 = (A-pivot).dot(axis);
    Scalar e2 = (B-pivot).dot(axis);
    Vector3 C1 = pivot + e1 * axis;
    Vector3 C2 = pivot + e2 * axis;
    
	/*std::vector<glm::vec3> vertices;
	vertices.push_back(glm::vec3(A.getX(), A.getY(), A.getZ()));
	vertices.push_back(glm::vec3(C1.getX(), C1.getY(), C1.getZ()));
	vertices.push_back(glm::vec3(B.getX(), B.getY(), B.getZ()));
	vertices.push_back(glm::vec3(C2.getX(), C2.getY(), C2.getZ()));
	//OpenGLContent::getInstance()->DrawPrimitives(PrimitiveType::LINES, vertices, DUMMY_COLOR);
	vertices.clear();
	
	vertices.push_back(glm::vec3(C1.getX(), C1.getY(), C1.getZ()));
	vertices.push_back(glm::vec3(C2.getX(), C2.getY(), C2.getZ()));
	glEnable(GL_LINE_STIPPLE);
    //OpenGLContent::getInstance()->DrawPrimitives(PrimitiveType::LINES, vertices, DUMMY_COLOR);
	glDisable(GL_LINE_STIPPLE);
    */
    return (C1+C2)/Scalar(2.);
}

}
