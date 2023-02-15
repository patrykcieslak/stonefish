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
//  SpringJoint.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 15/02/13.
//  Copyright (c) 2023 Patryk Cieslak. All rights reserved.
//

#include "joints/SpringJoint.h"

#include "core/SimulationApp.h"
#include "core/SimulationManager.h"
#include "entities/SolidEntity.h"

namespace sf
{

SpringJoint::SpringJoint(std::string uniqueName, SolidEntity* solid, const Transform& attachment,
                        const Vector3& linearStiffness, const Vector3& angularStiffness,
                        const Vector3& linearDamping, const Vector3& angularDamping) : Joint(uniqueName, false)
{
    btRigidBody* bodyA = solid->rigidBody;
    Transform frameInA = solid->getCGTransform().inverse() * attachment;

    btGeneric6DofSpring2Constraint* spring = new btGeneric6DofSpring2Constraint(*bodyA, frameInA, RO_ZYX);
    spring->enableSpring(0, true);
    spring->enableSpring(1, true);
    spring->enableSpring(2, true);
    spring->enableSpring(3, true);
    spring->enableSpring(4, true);
    spring->enableSpring(5, true);
    spring->setStiffness(0, linearStiffness.getX());
    spring->setStiffness(1, linearStiffness.getY());
    spring->setStiffness(2, linearStiffness.getZ());
    spring->setStiffness(3, angularStiffness.getX());
    spring->setStiffness(4, angularStiffness.getY());
    spring->setStiffness(5, angularStiffness.getZ());
    spring->setDamping(0, linearDamping.getX());
    spring->setDamping(1, linearDamping.getY());
    spring->setDamping(2, linearDamping.getZ());
    spring->setDamping(3, angularDamping.getX());
    spring->setDamping(4, angularDamping.getY());
    spring->setDamping(5, angularDamping.getZ());
    setConstraint(spring);
}

SpringJoint::SpringJoint(std::string uniqueName, SolidEntity* solidA, SolidEntity* solidB, const Transform& attachment,
                        const Vector3& linearStiffness, const Vector3& angularStiffness,
                        const Vector3& linearDamping, const Vector3& angularDamping) : Joint(uniqueName, false)
{
    btRigidBody* bodyA = solidA->rigidBody;
    btRigidBody* bodyB = solidB->rigidBody;
    Transform frameInA = bodyA->getCenterOfMassTransform().inverse() * attachment;
    Transform frameInB = bodyB->getCenterOfMassTransform().inverse() * attachment;
    
    btGeneric6DofSpring2Constraint* spring = new btGeneric6DofSpring2Constraint(*bodyA, *bodyB, frameInA, frameInB, RO_ZYX);
    spring->enableSpring(0, true);
    spring->enableSpring(1, true);
    spring->enableSpring(2, true);
    spring->enableSpring(3, true);
    spring->enableSpring(4, true);
    spring->enableSpring(5, true);
    spring->setStiffness(0, linearStiffness.getX());
    spring->setStiffness(1, linearStiffness.getY());
    spring->setStiffness(2, linearStiffness.getZ());
    spring->setStiffness(3, angularStiffness.getX());
    spring->setStiffness(4, angularStiffness.getY());
    spring->setStiffness(5, angularStiffness.getZ());
    spring->setDamping(0, linearDamping.getX());
    spring->setDamping(1, linearDamping.getY());
    spring->setDamping(2, linearDamping.getZ());
    spring->setDamping(3, angularDamping.getX());
    spring->setDamping(4, angularDamping.getY());
    spring->setDamping(5, angularDamping.getZ());
    setConstraint(spring);
}

JointType SpringJoint::getType() const
{
    return JointType::SPRING;
}

std::vector<Renderable> SpringJoint::Render()
{
    std::vector<Renderable> items(0);
    btGeneric6DofSpring2Constraint* c = (btGeneric6DofSpring2Constraint*)getConstraint();
    if(c != nullptr)
    {
        Renderable item;
        item.model = glm::mat4(1.f);
        item.type = RenderableType::JOINT_LINES;
        Vector3 A = (c->getRigidBodyA().getCenterOfMassTransform() * c->getFrameOffsetA()).getOrigin();
        Vector3 B = (c->getRigidBodyB().getCenterOfMassTransform() * c->getFrameOffsetB()).getOrigin();   
        item.points.push_back(glm::vec3(A.getX(), A.getY(), A.getZ()));
        item.points.push_back(glm::vec3(B.getX(), B.getY(), B.getZ()));
        items.push_back(item);    
    }
    return items;
}

}
