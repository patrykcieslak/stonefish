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
//  FixedJoint.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 2/4/13.
//  Copyright (c) 2013-2019 Patryk Cieslak. All rights reserved.
//

#include "joints/FixedJoint.h"

#include <BulletDynamics/Featherstone/btMultiBodyFixedConstraint.h>
#include "core/SimulationApp.h"
#include "core/SimulationManager.h"
#include "entities/SolidEntity.h"
#include "entities/FeatherstoneEntity.h"

namespace sf
{

FixedJoint::FixedJoint(std::string uniqueName, SolidEntity* solidA, SolidEntity* solidB) : Joint(uniqueName, false)
{
    btRigidBody* bodyA = solidA->rigidBody;
    btRigidBody* bodyB = solidB->rigidBody;
    Transform frameInA = Transform::getIdentity();
    Transform frameInB = bodyB->getCenterOfMassTransform().inverse() * bodyA->getCenterOfMassTransform(); //CHECK IT!!!!!!!!!!
    
    btFixedConstraint* fixed = new btFixedConstraint(*bodyA, *bodyB, frameInA, frameInB);
    setConstraint(fixed);
}

FixedJoint::FixedJoint(std::string uniqueName, SolidEntity* solid, FeatherstoneEntity* fe, int linkId, const Vector3& pivot) : Joint(uniqueName, false)
{
    Transform linkTransform = fe->getLinkTransform(linkId+1);
    Transform solidTransform = solid->getCGTransform();
    Vector3 pivotInA = linkTransform.inverse() * pivot;
    Vector3 pivotInB = solidTransform.inverse() * pivot;
    Matrix3 frameInB = solidTransform.getBasis().inverse() * linkTransform.getBasis();
    
    btMultiBodyFixedConstraint* fixed = new btMultiBodyFixedConstraint(fe->getMultiBody(), linkId, solid->rigidBody, pivotInA, pivotInB, Matrix3::getIdentity(), frameInB);
    setConstraint(fixed);
    fixed->setMaxAppliedImpulse(BT_LARGE_FLOAT);
    
    //Disable collision
    SimulationApp::getApp()->getSimulationManager()->DisableCollision(fe->getLink(linkId+1).solid, solid);
}

FixedJoint::FixedJoint(std::string uniqueName, FeatherstoneEntity* feA, FeatherstoneEntity* feB, int linkIdA, int linkIdB, const Vector3& pivot) : Joint(uniqueName, false)
{
    Transform linkATransform = feA->getLinkTransform(linkIdA+1);
    Transform linkBTransform = feB->getLinkTransform(linkIdB+1);
    Vector3 pivotInA = linkATransform.inverse() * pivot;
    Vector3 pivotInB = linkBTransform.inverse() * pivot;
    Matrix3 frameInB = linkBTransform.getBasis().inverse() * linkATransform.getBasis();	
	
	btMultiBodyFixedConstraint* fixed = new btMultiBodyFixedConstraint(feA->getMultiBody(), linkIdA, feB->getMultiBody(), linkIdB, pivotInA, pivotInB, Matrix3::getIdentity(), frameInB);
	setConstraint(fixed);
    fixed->setMaxAppliedImpulse(BT_LARGE_FLOAT);

    //Disable collision
    SimulationApp::getApp()->getSimulationManager()->DisableCollision(feA->getLink(linkIdA+1).solid, feB->getLink(linkIdB+1).solid);
}

JointType FixedJoint::getType()
{
    return JOINT_FIXED;
}
    
std::vector<Renderable> FixedJoint::Render()
{
    std::vector<Renderable> items(0);
    Renderable item;
    item.model = glm::mat4(1.f);
    item.type = RenderableType::JOINT_LINES;
    
    btTypedConstraint* revo = getConstraint();
    Vector3 A = revo->getRigidBodyA().getCenterOfMassPosition();
    Vector3 B = revo->getRigidBodyB().getCenterOfMassPosition();
    
    item.points.push_back(glm::vec3(A.getX(), A.getY(), A.getZ()));
    item.points.push_back(glm::vec3(B.getX(), B.getY(), B.getZ()));
    items.push_back(item);
    
    return items;
}

}
