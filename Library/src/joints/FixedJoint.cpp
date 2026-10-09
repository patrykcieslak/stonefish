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

FixedJoint::FixedJoint(std::string_view uniqueName, SolidEntity* solid) 
    : Joint(uniqueName, false)
{
    btRigidBody* body = solid->GetRigidBody();
    
    std::unique_ptr<btGeneric6DofConstraint> fixed = std::make_unique<btGeneric6DofConstraint>(*body, Transform::getIdentity(), true);
    fixed->setAngularLowerLimit(Vector3(0,0,0));
    fixed->setAngularUpperLimit(Vector3(0,0,0));
    fixed->setLinearLowerLimit(Vector3(0,0,0));
    fixed->setLinearUpperLimit(Vector3(0,0,0));
    constraint_ = std::move(fixed);

    jSolidA_ = nullptr;
    jSolidB_ = solid;

    cInfo("Fixed joint created between the world and '%s'.", jSolidB_->GetName().c_str());
}

FixedJoint::FixedJoint(std::string_view uniqueName, SolidEntity* solidA, SolidEntity* solidB) 
    : Joint(uniqueName, false)
{
    btRigidBody* bodyA = solidA->GetRigidBody();
    btRigidBody* bodyB = solidB->GetRigidBody();
    Transform frameInA = solidA->GetCgTransform().inverse() * solidB->GetCgTransform();
    Transform frameInB = Transform::getIdentity(); 
    
    std::unique_ptr<btFixedConstraint> fixed = std::make_unique<btFixedConstraint>(*bodyA, *bodyB, frameInA, frameInB);
    constraint_ = std::move(fixed);
    
    jSolidA_ = solidA;
    jSolidB_ = solidB;

    cInfo("Fixed joint created between '%s' and '%s'.", jSolidA_->GetName().c_str(), jSolidB_->GetName().c_str());
}

FixedJoint::FixedJoint(std::string_view uniqueName, SolidEntity* solid, FeatherstoneEntity* fe, int linkId) 
    : Joint(uniqueName, false)
{
    Transform linkTransform = fe->GetLinkTransform(linkId+1);
    Transform solidTransform = solid->GetCgTransform();

    // Pivot point and frame have to be aligned with body B, otherwise the constraint explodes !!!
    Vector3 pivotInA = linkTransform.inverse() * solidTransform.getOrigin();
    Matrix3 frameInA = linkTransform.getBasis().inverse() * solidTransform.getBasis();
    Vector3 pivotInB = V0();
    Matrix3 frameInB = Matrix3::getIdentity();

    std::unique_ptr<btMultiBodyFixedConstraint> fixed = std::make_unique<btMultiBodyFixedConstraint>(
        fe->GetMultiBody(), linkId, solid->GetRigidBody(), pivotInA, pivotInB, frameInA, frameInB
    );
    fixed->setMaxAppliedImpulse(BT_LARGE_FLOAT);
    mbConstraint_ = std::move(fixed);
    
    jSolidA_ = fe->GetLink(linkId+1).solid.get();
    jSolidB_ = solid;

    cInfo("Fixed joint created between '%s' and '%s'.", jSolidA_->GetName().c_str(), jSolidB_->GetName().c_str());
}

FixedJoint::FixedJoint(std::string_view uniqueName, FeatherstoneEntity* feA, FeatherstoneEntity* feB, int linkIdA, int linkIdB) : Joint(uniqueName, false)
{
    Transform linkATransform = feA->GetLinkTransform(linkIdA+1);
    Transform linkBTransform = feB->GetLinkTransform(linkIdB+1);
    
    Vector3 pivotInA = linkATransform.inverse() * linkBTransform.getOrigin();
    Matrix3 frameInA = linkATransform.getBasis().inverse() * linkBTransform.getBasis();	
    Vector3 pivotInB = V0();
    Matrix3 frameInB = Matrix3::getIdentity();
    
    std::unique_ptr<btMultiBodyFixedConstraint> fixed = std::make_unique<btMultiBodyFixedConstraint>(
        feA->GetMultiBody(), linkIdA, feB->GetMultiBody(), linkIdB, pivotInA, pivotInB, frameInA, frameInB
    );
    fixed->setMaxAppliedImpulse(BT_LARGE_FLOAT);
    mbConstraint_ = std::move(fixed);
    
    jSolidA_ = feA->GetLink(linkIdA+1).solid.get();
    jSolidB_ = feB->GetLink(linkIdB+1).solid.get();

    cInfo("Fixed joint created between '%s' and '%s'.", jSolidA_->GetName().c_str(), jSolidB_->GetName().c_str());
}

JointType FixedJoint::GetType() const
{
    return JointType::FIXED;
}

void FixedJoint::UpdateDefinition()
{
    if(constraint_ != nullptr)
    {
        if(jSolidA_ == nullptr)
        {
            std::unique_ptr<btGeneric6DofConstraint> fixed = std::make_unique<btGeneric6DofConstraint>(*jSolidB_->GetRigidBody(), Transform::getIdentity(), true);
            fixed->setAngularLowerLimit(Vector3(0,0,0));
            fixed->setAngularUpperLimit(Vector3(0,0,0));
            fixed->setLinearLowerLimit(Vector3(0,0,0));
            fixed->setLinearUpperLimit(Vector3(0,0,0));
            constraint_ = std::move(fixed);
        }
        else
        {
            Transform frameInA = jSolidA_->GetCgTransform().inverse() * jSolidB_->GetCgTransform();
            // Frame in B is always identity
            constraint_ = std::make_unique<btFixedConstraint>(*jSolidA_->GetRigidBody(), *jSolidB_->GetRigidBody(), frameInA, Transform::getIdentity());
        }        
    }
    else if(mbConstraint_ != nullptr)
    {
        Transform linkATransform = jSolidA_->GetCgTransform();
        Transform linkBTransform = jSolidB_->GetCgTransform();
        
        Vector3 pivotInA = linkATransform.inverse() * linkBTransform.getOrigin();
        Matrix3 frameInA = linkATransform.getBasis().inverse() * linkBTransform.getBasis();	
        
        btMultiBodyFixedConstraint* fix = static_cast<btMultiBodyFixedConstraint*>(mbConstraint_.get());
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
