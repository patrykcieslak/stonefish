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
//  SphericalJoint.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 2/3/13.
//  Copyright (c) 2013-2025 Patryk Cieslak. All rights reserved.
//

#include "joints/SphericalJoint.h"

#include "BulletDynamics/Featherstone/btMultiBodyPoint2Point.h"
#include "entities/SolidEntity.h"
#include "entities/FeatherstoneEntity.h"

namespace sf
{

SphericalJoint::SphericalJoint(std::string uniqueName, SolidEntity* solidA, SolidEntity* solidB, const Vector3& pivot, bool collideLinked) : Joint(uniqueName, collideLinked)
{
    btRigidBody* bodyA = solidA->rigidBody;
    btRigidBody* bodyB = solidB->rigidBody;
    Vector3 pivotInA = bodyA->getCenterOfMassTransform().inverse() * pivot;
    Vector3 pivotInB = bodyB->getCenterOfMassTransform().inverse() * pivot;
    
    btPoint2PointConstraint* p2p = new btPoint2PointConstraint(*bodyA, *bodyB, pivotInA, pivotInB);
    setConstraint(p2p);
    
    jSolidA = solidA;
    jSolidB = solidB;

    sigDamping = Vector3(0.,0.,0.);
    velDamping = Vector3(0.,0.,0.);
    angleIC = Vector3(0.,0.,0.);
}

SphericalJoint::SphericalJoint(std::string uniqueName, SolidEntity* solid, FeatherstoneEntity* fe, int linkId, const Vector3& pivot, bool collideLinked) : Joint(uniqueName, collideLinked)
{
    Transform linkTransform = fe->getLinkTransform(linkId+1);
    Transform solidTransform = solid->getCGTransform();
    Vector3 pivotInA = linkTransform.inverse() * pivot;
    Vector3 pivotInB = solidTransform.inverse() * pivot;
    
    btMultiBodyPoint2Point* p2p = new btMultiBodyPoint2Point(fe->getMultiBody(), linkId, solid->rigidBody, pivotInA, pivotInB);
    p2p->setMaxAppliedImpulse(BT_LARGE_FLOAT);
    setConstraint(p2p);
    
    jSolidA = fe->getLink(linkId+1).solid;
    jSolidB = solid;

    sigDamping = Vector3(0.,0.,0.);
    velDamping = Vector3(0.,0.,0.);
    angleIC = Vector3(0.,0.,0.);
}

void SphericalJoint::setDamping(Vector3 constantFactor, Vector3 viscousFactor)
{
    if(isMultibodyJoint())
        return; 

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

JointType SphericalJoint::getType() const
{
    return JointType::SPHERICAL;
}

void SphericalJoint::ApplyTorque(Vector3 T)
{
    if(isMultibodyJoint())
        return;

    btRigidBody& bodyA = getConstraint()->getRigidBodyA();
    btRigidBody& bodyB = getConstraint()->getRigidBodyB();
    Vector3 torque = T;
    bodyA.applyTorque(torque);
    bodyB.applyTorque(-torque);
}

void SphericalJoint::ApplyDamping()
{
    if(isMultibodyJoint())
        return;

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

std::vector<Renderable> SphericalJoint::Render()
{
    std::vector<Renderable> items(0);
    btTypedConstraint* c = getConstraint();
    if(c != nullptr)
    {
        Renderable item;
        item.model = glm::mat4(1.f);
        item.type = RenderableType::JOINT_LINES;
        item.data = std::make_shared<std::vector<glm::vec3>>();
        auto points = item.getDataAsPoints();
        
        btPoint2PointConstraint* p2p = (btPoint2PointConstraint*)getConstraint();
        Vector3 pivot = p2p->getRigidBodyA().getCenterOfMassTransform()(p2p->getPivotInA());
        Vector3 A = p2p->getRigidBodyA().getCenterOfMassPosition();
        Vector3 B = p2p->getRigidBodyB().getCenterOfMassPosition();
        
        points->push_back(glm::vec3(A.getX(), A.getY(), A.getZ()));
        points->push_back(glm::vec3(pivot.getX(), pivot.getY(), pivot.getZ()));
        points->push_back(glm::vec3(B.getX(), B.getY(), B.getZ()));
        points->push_back(glm::vec3(pivot.getX(), pivot.getY(), pivot.getZ()));
        
        items.push_back(item);
    }
    return items;
}

}
