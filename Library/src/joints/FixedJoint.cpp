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
//  Copyright (c) 2013-2025 Patryk Cieslak. All rights reserved.
//

#include "joints/FixedJoint.h"

#include "BulletDynamics/Featherstone/btMultiBodyFixedConstraint.h"
#include "core/SimulationApp.h"
#include "core/SimulationManager.h"
#include "entities/SolidEntity.h"
#include "entities/FeatherstoneEntity.h"
#include "utils/DebugUtil.hpp"

namespace sf
{

FixedJoint::FixedJoint(std::string uniqueName, SolidEntity* solid) 
    : Joint(uniqueName, false)
{
    btRigidBody* body = solid->rigidBody;
    
    btFixedConstraint* fixed = new btFixedConstraint(*body, Transform::getIdentity());
    setConstraint(fixed);

    jSolidA = nullptr;
    jSolidB = solid;

    cInfo("Fixed joint created between the world and '%s'.", jSolidB->getName().c_str());
}

FixedJoint::FixedJoint(std::string uniqueName, SolidEntity* solidA, SolidEntity* solidB) 
    : Joint(uniqueName, false)
{
    btRigidBody* bodyA = solidA->rigidBody;
    btRigidBody* bodyB = solidB->rigidBody;
    Transform frameInA = Transform::getIdentity();
    Transform frameInB = solidB->getCGTransform().inverse() * solidA->getCGTransform();
    
    btFixedConstraint* fixed = new btFixedConstraint(*bodyA, *bodyB, frameInA, frameInB);
    setConstraint(fixed);
    
    jSolidA = solidA;
    jSolidB = solidB;

    cInfo("Fixed joint created between '%s' and '%s'.", jSolidA->getName().c_str(), jSolidB->getName().c_str());
}

FixedJoint::FixedJoint(std::string uniqueName, SolidEntity* solid, FeatherstoneEntity* fe, int linkId) 
    : Joint(uniqueName, false)
{
    Transform linkTransform = fe->getLinkTransform(linkId+1);
    Transform solidTransform = solid->getCGTransform();

    Vector3 pivotInA = linkTransform.inverse() * solidTransform.getOrigin();
    Vector3 pivotInB = V0();
    Matrix3 frameInA = linkTransform.getBasis().inverse() * solidTransform.getBasis();
    Matrix3 frameInB = Matrix3::getIdentity();

    btMultiBodyFixedConstraint* fixed = new btMultiBodyFixedConstraint(fe->getMultiBody(), linkId, solid->rigidBody, pivotInA, pivotInB, frameInA, frameInB);
    fixed->setMaxAppliedImpulse(BT_LARGE_FLOAT);
    setConstraint(fixed);
    
    jSolidA = fe->getLink(linkId+1).solid;
    jSolidB = solid;

    cInfo("Fixed joint created between '%s' and '%s'.", jSolidA->getName().c_str(), jSolidB->getName().c_str());
}

FixedJoint::FixedJoint(std::string uniqueName, FeatherstoneEntity* feA, FeatherstoneEntity* feB, int linkIdA, int linkIdB) : Joint(uniqueName, false)
{
    Transform linkATransform = feA->getLinkTransform(linkIdA+1);
    Transform linkBTransform = feB->getLinkTransform(linkIdB+1);
    Vector3 pivotInA = V0();
    Vector3 pivotInB = linkBTransform.inverse() * linkATransform.getOrigin();
    Matrix3 frameInA = Matrix3::getIdentity();
    Matrix3 frameInB = linkBTransform.getBasis().inverse() * linkATransform.getBasis();	
    
    btMultiBodyFixedConstraint* fixed = new btMultiBodyFixedConstraint(feA->getMultiBody(), linkIdA, feB->getMultiBody(), linkIdB, pivotInA, pivotInB, frameInA, frameInB);
    fixed->setMaxAppliedImpulse(BT_LARGE_FLOAT);
    setConstraint(fixed);
    
    jSolidA = feA->getLink(linkIdA+1).solid;
    jSolidB = feB->getLink(linkIdB+1).solid;

    cInfo("Fixed joint created between '%s' and '%s'.", jSolidA->getName().c_str(), jSolidB->getName().c_str());
}

JointType FixedJoint::getType() const
{
    return JointType::FIXED;
}

void FixedJoint::UpdateDefinition()
{
    if(constraint != nullptr)
    {
        delete constraint;
        if(jSolidA == nullptr)
        {
            setConstraint(new btFixedConstraint(*jSolidB->getRigidBody(), Transform::getIdentity()));
        }
        else
        {
            Transform frameInA = Transform::getIdentity();
            Transform frameInB = jSolidB->getCGTransform().inverse() * jSolidA->getCGTransform();
            setConstraint(new btFixedConstraint(*jSolidA->getRigidBody(), *jSolidB->getRigidBody(), frameInA, frameInB));   
        }        
    }
    else if(mbConstraint != nullptr)
    {
        Transform linkATransform = jSolidA->getCGTransform();
        Transform linkBTransform = jSolidB->getCGTransform();
        Vector3 pivotInB = linkBTransform.inverse() * linkATransform.getOrigin();
        Matrix3 frameInB = linkBTransform.getBasis().inverse() * linkATransform.getBasis();	
        
        btMultiBodyFixedConstraint* fix = static_cast<btMultiBodyFixedConstraint*>(mbConstraint);
        fix->setPivotInB(pivotInB);
        fix->setFrameInB(frameInB);
    }
}

std::vector<Renderable> FixedJoint::Render()
{
    std::vector<Renderable> items(0);
    return items;
}

}
