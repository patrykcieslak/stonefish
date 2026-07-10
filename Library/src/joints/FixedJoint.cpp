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
//  Copyright (c) 2013-2026 Patryk Cieslak. All rights reserved.
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

FixedJoint::FixedJoint(const std::string& uniqueName, SolidEntity* solid) 
    : Joint(uniqueName, false)
{
    btRigidBody* body = solid->getRigidBody();
    
    btGeneric6DofConstraint* fixed = new btGeneric6DofConstraint(*body, Transform::getIdentity(), true);
    fixed->setAngularLowerLimit(Vector3(0,0,0));
    fixed->setAngularUpperLimit(Vector3(0,0,0));
    fixed->setLinearLowerLimit(Vector3(0,0,0));
    fixed->setLinearUpperLimit(Vector3(0,0,0));
    setConstraint(fixed);

    jSolidA_ = nullptr;
    jSolidB_ = solid;

    cInfo("Fixed joint created between the world and '%s'.", jSolidB_->getName().c_str());
}

FixedJoint::FixedJoint(const std::string& uniqueName, SolidEntity* solidA, SolidEntity* solidB) 
    : Joint(uniqueName, false)
{
    btRigidBody* bodyA = solidA->getRigidBody();
    btRigidBody* bodyB = solidB->getRigidBody();
    Transform frameInA = solidA->getCGTransform().inverse() * solidB->getCGTransform();
    Transform frameInB = Transform::getIdentity(); 
    
    btFixedConstraint* fixed = new btFixedConstraint(*bodyA, *bodyB, frameInA, frameInB);
    setConstraint(fixed);
    
    jSolidA_ = solidA;
    jSolidB_ = solidB;

    cInfo("Fixed joint created between '%s' and '%s'.", jSolidA_->getName().c_str(), jSolidB_->getName().c_str());
}

FixedJoint::FixedJoint(const std::string& uniqueName, SolidEntity* solid, FeatherstoneEntity* fe, int linkId) 
    : Joint(uniqueName, false)
{
    Transform linkTransform = fe->getLinkTransform(linkId+1);
    Transform solidTransform = solid->getCGTransform();

    // Pivot point and frame have to be aligned with body B, otherwise the constraint explodes !!!
    Vector3 pivotInA = linkTransform.inverse() * solidTransform.getOrigin();
    Matrix3 frameInA = linkTransform.getBasis().inverse() * solidTransform.getBasis();
    Vector3 pivotInB = V0();
    Matrix3 frameInB = Matrix3::getIdentity();

    btMultiBodyFixedConstraint* fixed = new btMultiBodyFixedConstraint(fe->getMultiBody(), linkId, solid->getRigidBody(), pivotInA, pivotInB, frameInA, frameInB);
    fixed->setMaxAppliedImpulse(BT_LARGE_FLOAT);
    setConstraint(fixed);
    
    jSolidA_ = fe->getLink(linkId+1).solid.get();
    jSolidB_ = solid;

    cInfo("Fixed joint created between '%s' and '%s'.", jSolidA_->getName().c_str(), jSolidB_->getName().c_str());
}

FixedJoint::FixedJoint(const std::string& uniqueName, FeatherstoneEntity* feA, FeatherstoneEntity* feB, int linkIdA, int linkIdB) : Joint(uniqueName, false)
{
    Transform linkATransform = feA->getLinkTransform(linkIdA+1);
    Transform linkBTransform = feB->getLinkTransform(linkIdB+1);
    
    Vector3 pivotInA = linkATransform.inverse() * linkBTransform.getOrigin();
    Matrix3 frameInA = linkATransform.getBasis().inverse() * linkBTransform.getBasis();	
    Vector3 pivotInB = V0();
    Matrix3 frameInB = Matrix3::getIdentity();
    
    btMultiBodyFixedConstraint* fixed = new btMultiBodyFixedConstraint(feA->getMultiBody(), linkIdA, feB->getMultiBody(), linkIdB, pivotInA, pivotInB, frameInA, frameInB);
    fixed->setMaxAppliedImpulse(BT_LARGE_FLOAT);
    setConstraint(fixed);
    
    jSolidA_ = feA->getLink(linkIdA+1).solid.get();
    jSolidB_ = feB->getLink(linkIdB+1).solid.get();

    cInfo("Fixed joint created between '%s' and '%s'.", jSolidA_->getName().c_str(), jSolidB_->getName().c_str());
}

JointType FixedJoint::getType() const
{
    return JointType::FIXED;
}

void FixedJoint::UpdateDefinition()
{
    if(constraint_ != nullptr)
    {
        delete constraint_;
        if(jSolidA_ == nullptr)
        {
            btGeneric6DofConstraint* fixed = new btGeneric6DofConstraint(*jSolidB_->getRigidBody(), Transform::getIdentity(), true);
            fixed->setAngularLowerLimit(Vector3(0,0,0));
            fixed->setAngularUpperLimit(Vector3(0,0,0));
            fixed->setLinearLowerLimit(Vector3(0,0,0));
            fixed->setLinearUpperLimit(Vector3(0,0,0));
            setConstraint(fixed);
        }
        else
        {
            Transform frameInA = jSolidA_->getCGTransform().inverse() * jSolidB_->getCGTransform();
            // Frame in B is always identity
            setConstraint(new btFixedConstraint(*jSolidA_->getRigidBody(), *jSolidB_->getRigidBody(), frameInA, Transform::getIdentity()));   
        }        
    }
    else if(mbConstraint_ != nullptr)
    {
        Transform linkATransform = jSolidA_->getCGTransform();
        Transform linkBTransform = jSolidB_->getCGTransform();
        
        Vector3 pivotInA = linkATransform.inverse() * linkBTransform.getOrigin();
        Matrix3 frameInA = linkATransform.getBasis().inverse() * linkBTransform.getBasis();	
        
        btMultiBodyFixedConstraint* fix = static_cast<btMultiBodyFixedConstraint*>(mbConstraint_);
        fix->setPivotInA(pivotInA);
        fix->setFrameInA(frameInA);
        // Pivot and frame in B are always identity
    }
}

std::vector<Renderable> FixedJoint::Render()
{
    std::vector<Renderable> items(0);
    return items;
}

}
