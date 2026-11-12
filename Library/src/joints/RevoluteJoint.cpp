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
//  Copyright (c) 2013-2026 Patryk Cieslak. All rights reserved.
//

#include "joints/RevoluteJoint.h"

#include "entities/SolidEntity.h"
#include "utils/GeometryFileUtil.h"
#include "core/SimulationApp.h"
#include "core/SimulationManager.h"

namespace sf
{

RevoluteJoint::RevoluteJoint(std::string_view uniqueName, SolidEntity* solidA, SolidEntity* solidB, const Vector3& pivot, const Vector3& axis, bool collideLinked) : Joint(uniqueName, collideLinked)
{
    Vector3 hingeAxis = axis.normalized();
    btRigidBody* bodyA = solidA->GetRigidBody();
    btRigidBody* bodyB = solidB->GetRigidBody();
    axisInA_ = bodyA->getCenterOfMassTransform().getBasis().inverse() * hingeAxis;
    Vector3 axisInB = bodyB->getCenterOfMassTransform().getBasis().inverse() * hingeAxis;
    pivotInA_ = bodyA->getCenterOfMassTransform().inverse()(pivot);
    Vector3 pivotInB = bodyB->getCenterOfMassTransform().inverse()(pivot);
    
    std::unique_ptr<btHingeConstraint> hinge = std::make_unique<btHingeConstraint>(*bodyA, *bodyB, pivotInA_, pivotInB, axisInA_, axisInB, true);
    hinge->setLimit(Scalar(1), Scalar(-1)); //no limit (min > max)
    angleOffset_ = hinge->getHingeAngle();
    sigDamping_ = Scalar(0);
    velDamping_ = Scalar(0);
    constraint_ = std::move(hinge);
    
    cInfo("Created revolute joint '%s'. Offset: %lf rad.", std::string(uniqueName).c_str(), angleOffset_);
    SetIc(Scalar(0));
}

RevoluteJoint::RevoluteJoint(std::string_view uniqueName, SolidEntity* solid, const Vector3& pivot, const Vector3& axis) : Joint(uniqueName, false)
{
    btRigidBody* body = solid->GetRigidBody();
    Vector3 hingeAxis = axis.normalized();
    axisInA_ = body->getCenterOfMassTransform().getBasis().inverse() * hingeAxis;
    pivotInA_ = body->getCenterOfMassTransform().inverse()(pivot);
    
    std::unique_ptr<btHingeConstraint> hinge = std::make_unique<btHingeConstraint>(*body, pivotInA_, axisInA_, true);
    hinge->setLimit(Scalar(1), Scalar(-1)); //no limit (min > max)
    angleOffset_ = hinge->getHingeAngle();
    sigDamping_ = Scalar(0);
    velDamping_ = Scalar(0);
    constraint_ = std::move(hinge);
    
    cInfo("Created revolute joint '%s'. Offset: %lf rad.", std::string(uniqueName).c_str(), angleOffset_);
    SetIc(Scalar(0));
}

void RevoluteJoint::SetDamping(Scalar constantFactor, Scalar viscousFactor)
{
    sigDamping_ = constantFactor > Scalar(0) ? constantFactor : Scalar(0);
    velDamping_ = viscousFactor > Scalar(0) ? viscousFactor : Scalar(0);
}

void RevoluteJoint::SetLimits(Scalar min, Scalar max)
{
    if(min > max) // No limit
    {
        btHingeConstraint* hinge = static_cast<btHingeConstraint*>(constraint_.get());
        hinge->setLimit(Scalar(1), Scalar(-1));    
        return;
    }

    btHingeConstraint* hinge = static_cast<btHingeConstraint*>(constraint_.get());
    min = btFmod(min + angleOffset_, SIMD_2_PI); 
    max = btFmod(max + angleOffset_, SIMD_2_PI);
    hinge->setLimit(min, max);
    cInfo("Setting limits of revolute joint '%s': %lf, %lf.", GetName().c_str(), min, max);
}

void RevoluteJoint::SetIc(Scalar angle)
{
    angleIC_ = angle;
    angleICError_ = angleIC_ - GetAngle();
}

JointType RevoluteJoint::GetType() const
{
    return JointType::REVOLUTE;
}

Scalar RevoluteJoint::GetAngle()
{
    return btNormalizeAngle(static_cast<btHingeConstraint*>(constraint_.get())->getHingeAngle() - angleOffset_);
}

Scalar RevoluteJoint::GetAngularVelocity()
{
    btRigidBody& bodyA = constraint_->getRigidBodyA();
    btRigidBody& bodyB = constraint_->getRigidBodyB();
    Vector3 relativeAV = bodyA.getAngularVelocity() - bodyB.getAngularVelocity();
    Vector3 axis = (bodyA.getCenterOfMassTransform().getBasis() * axisInA_).normalized();
    return relativeAV.dot(axis);
}

void RevoluteJoint::EnableMotor(bool enable, Scalar maxTorque)
{
    btHingeConstraint* hinge = static_cast<btHingeConstraint*>(constraint_.get());
    hinge->enableAngularMotor(enable, Scalar(0), maxTorque / SimulationApp::GetApp()->GetSimulationManager()->GetStepsPerSecond());
}

void RevoluteJoint::SetMotorVelocity(Scalar av)
{
    btHingeConstraint* hinge = static_cast<btHingeConstraint*>(constraint_.get());
    hinge->setMotorTargetVelocity(av/Scalar(4));
}

void RevoluteJoint::ApplyTorque(Scalar T)
{
    btRigidBody& bodyA = constraint_->getRigidBodyA();
    btRigidBody& bodyB = constraint_->getRigidBodyB();
    Vector3 axis = (bodyA.getCenterOfMassTransform().getBasis() * axisInA_).normalized();
    
    Vector3 torque = axis * T;
    bodyA.applyTorque(torque);
    bodyB.applyTorque(-torque);
}

void RevoluteJoint::ApplyDamping()
{
    if(sigDamping_ > Scalar(0.) || velDamping_ > Scalar(0.))
    {
        btRigidBody& bodyA = constraint_->getRigidBodyA();
        btRigidBody& bodyB = constraint_->getRigidBodyB();
        Vector3 axis = (bodyA.getCenterOfMassTransform().getBasis() * axisInA_).normalized();
        Vector3 relativeAV = bodyA.getAngularVelocity() - bodyB.getAngularVelocity();
        Scalar av = relativeAV.dot(axis);
       
        if(av != Scalar(0.))
        {
            Scalar T = sigDamping_ * av/fabs(av) + velDamping_ * av;
            Vector3 torque = axis * -T;
        
            bodyA.applyTorque(torque);
            bodyB.applyTorque(-torque);
        }
    }
}

bool RevoluteJoint::SolvePositionIC(Scalar linearTolerance, Scalar angularTolerance)
{
    Scalar lastAngleICError = angleICError_;
    angleICError_ = angleIC_ - GetAngle();
    
    //Check if IC reached
    if(btFabs(angleICError_) < angularTolerance)
        return true;
    
    //Move joint
    Scalar torque = angleICError_ * Scalar(50) + (angleICError_ - lastAngleICError) * Scalar(300);
    ApplyTorque(torque);
    
    return false;
}

std::vector<Renderable> RevoluteJoint::Render()
{
    std::vector<Renderable> items(0);
    Renderable item;
    item.model = glm::mat4(1.f);
    item.type = RenderableType::JOINT_LINES;
    item.data = std::make_shared<std::vector<glm::vec3>>();
    auto points = item.GetDataAsPoints();
    
    Vector3 A = constraint_->getRigidBodyA().getCenterOfMassPosition();
    Vector3 B = constraint_->getRigidBodyB().getCenterOfMassPosition();
    Vector3 pivot =  constraint_->getRigidBodyA().getCenterOfMassTransform()(pivotInA_);
    Vector3 axis = (constraint_->getRigidBodyA().getCenterOfMassTransform().getBasis() * axisInA_).normalized();
    
    //Calculate axis ends
    Vector3 C1 = pivot;
    Vector3 C2 = pivot + axis * btMax(0.05, btFabs((A-B).safeNorm())/Scalar(2));
    points->push_back(glm::vec3(C1.getX(), C1.getY(), C1.getZ()));
    points->push_back(glm::vec3(C2.getX(), C2.getY(), C2.getZ()));
    
    items.push_back(item);
    
    return items;
}
    
}
