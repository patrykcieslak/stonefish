/*    
    This file is a part of Stonefish.

    Stonefish is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Stonefish is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

//
//  RevoluteJoint.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 1/13/13.
//  Copyright (c) 2013-2023 Patryk Cieslak. All rights reserved.
//

#include "joints/RevoluteJoint.h"

#include "entities/SolidEntity.h"
#include "utils/GeometryFileUtil.h"
#include "core/SimulationApp.h"
#include "core/SimulationManager.h"

namespace sf
{

RevoluteJoint::RevoluteJoint(std::string uniqueName, SolidEntity* solidA, SolidEntity* solidB, const Vector3& pivot, const Vector3& axis, bool collideLinked) : Joint(uniqueName, collideLinked)
{
    Vector3 hingeAxis = axis.normalized();
    btRigidBody* bodyA = solidA->rigidBody;
    btRigidBody* bodyB = solidB->rigidBody;
    axisInA = bodyA->getCenterOfMassTransform().getBasis().inverse() * hingeAxis;
    Vector3 axisInB = bodyB->getCenterOfMassTransform().getBasis().inverse() * hingeAxis;
    pivotInA = bodyA->getCenterOfMassTransform().inverse()(pivot);
    Vector3 pivotInB = bodyB->getCenterOfMassTransform().inverse()(pivot);
    
    btHingeConstraint* hinge = new btHingeConstraint(*bodyA, *bodyB, pivotInA, pivotInB, axisInA, axisInB);
    hinge->setLimit(Scalar(1), Scalar(-1)); //no limit (min > max)
    setConstraint(hinge);
    
    sigDamping = Scalar(0);
    velDamping = Scalar(0);
    angleOffset = hinge->getHingeAngle();
    setIC(Scalar(0));
}

RevoluteJoint::RevoluteJoint(std::string uniqueName, SolidEntity* solid, const Vector3& pivot, const Vector3& axis) : Joint(uniqueName, false)
{
    btRigidBody* body = solid->rigidBody;
    Vector3 hingeAxis = axis.normalized();
    axisInA = body->getCenterOfMassTransform().getBasis().inverse() * hingeAxis;
    pivotInA = body->getCenterOfMassTransform().inverse()(pivot);
    
    btHingeConstraint* hinge = new btHingeConstraint(*body, pivotInA, axisInA);
    hinge->setLimit(Scalar(1), Scalar(-1)); //no limit (min > max)
    setConstraint(hinge);
    
    sigDamping = Scalar(0);
    velDamping = Scalar(0);
    angleOffset = hinge->getHingeAngle();
    setIC(Scalar(0));
}

void RevoluteJoint::setDamping(Scalar constantFactor, Scalar viscousFactor)
{
    sigDamping = constantFactor > Scalar(0) ? constantFactor : Scalar(0);
    velDamping = viscousFactor > Scalar(0) ? viscousFactor : Scalar(0);
}

void RevoluteJoint::setLimits(Scalar min, Scalar max)
{
    if(min > max) // No limit
    {
        btHingeConstraint* hinge = (btHingeConstraint*)getConstraint();
        hinge->setLimit(Scalar(1), Scalar(-1));    
        return;
    }

    btHingeConstraint* hinge = (btHingeConstraint*)getConstraint();
    Scalar l1 = WrapAngle(min + angleOffset);
    Scalar l2 = WrapAngle(max + angleOffset);
    min = btMin(l1, l2);
    max = btMax(l1, l2);    
    hinge->setLimit(min, max);
}

void RevoluteJoint::setIC(Scalar angle)
{
    angleIC = angle;
    angleICError = angleIC - getAngle();
}

JointType RevoluteJoint::getType() const
{
    return JointType::REVOLUTE;
}

Scalar RevoluteJoint::getAngle()
{
    return WrapAngle(((btHingeConstraint*)getConstraint())->getHingeAngle() - angleOffset);
}

Scalar RevoluteJoint::getAngularVelocity()
{
    btRigidBody& bodyA = getConstraint()->getRigidBodyA();
    btRigidBody& bodyB = getConstraint()->getRigidBodyB();
    Vector3 relativeAV = bodyA.getAngularVelocity() - bodyB.getAngularVelocity();
    Vector3 axis = (bodyA.getCenterOfMassTransform().getBasis() * axisInA).normalized();
    return relativeAV.dot(axis);
}

void RevoluteJoint::EnableMotor(bool enable, Scalar maxTorque)
{
    btHingeConstraint* hinge = (btHingeConstraint*)getConstraint();
    hinge->enableAngularMotor(enable, Scalar(0), maxTorque / SimulationApp::getApp()->getSimulationManager()->getStepsPerSecond());
}

void RevoluteJoint::setMotorVelocity(Scalar av)
{
    btHingeConstraint* hinge = (btHingeConstraint*)getConstraint();
    hinge->setMotorTargetVelocity(av/Scalar(4));
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
    Scalar lastAngleICError = angleICError;
    angleICError = angleIC - getAngle();
    
    //Check if IC reached
    if(btFabs(angleICError) < angularTolerance)
        return true;
    
    //Move joint
    Scalar torque = angleICError * Scalar(50) + (angleICError - lastAngleICError) * Scalar(300);
    ApplyTorque(torque);
    
    return false;
}

std::vector<Renderable> RevoluteJoint::Render()
{
    std::vector<Renderable> items(0);
    Renderable item;
    item.model = glm::mat4(1.f);
    item.type = RenderableType::JOINT_LINES;
    
    btTypedConstraint* revo = getConstraint();
    Vector3 A = revo->getRigidBodyA().getCenterOfMassPosition();
    Vector3 B = revo->getRigidBodyB().getCenterOfMassPosition();
    Vector3 pivot =  revo->getRigidBodyA().getCenterOfMassTransform()(pivotInA);
    Vector3 axis = (revo->getRigidBodyA().getCenterOfMassTransform().getBasis() * axisInA).normalized();
    
    //Calculate axis ends
    Vector3 C1 = pivot;
    Vector3 C2 = pivot + axis * btMax(0.05, btFabs((A-B).safeNorm())/Scalar(2));
    item.points.push_back(glm::vec3(C1.getX(), C1.getY(), C1.getZ()));
    item.points.push_back(glm::vec3(C2.getX(), C2.getY(), C2.getZ()));
    
    items.push_back(item);
    
    return items;
}
    
}
