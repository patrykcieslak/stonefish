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
//  SuctionCup.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 13/02/2023.
//  Copyright (c) 2023-2026 Patryk Cieslak. All rights reserved.
//

#include "actuators/SuctionCup.h"

#include "core/SimulationManager.h"
#include "core/DeviceFactory.h"
#include "entities/FeatherstoneEntity.h"
#include "joints/SpringJoint.h"
#include "joints/SphericalJoint.h"

namespace sf
{

SuctionCup::SuctionCup(const std::string& uniqueName) : LinkActuator(uniqueName)
{
    pump_ = false;
    joint_ = nullptr;
    attachFE_ = nullptr;
    attachLinkId_ = 0;
}

LinkActuatorType SuctionCup::getLinkActuatorType() const
{
    return LinkActuatorType::SUCTION_CUP;
}

void SuctionCup::setPump(bool enabled)
{
    pump_ = enabled;
}

bool SuctionCup::getPump() const
{
    return pump_;
}

void SuctionCup::AttachToSolid(SolidEntity* body, const Transform& origin)
{
    LinkActuator::AttachToSolid(body, origin);
}

void SuctionCup::AttachToLink(FeatherstoneEntity* multibody, unsigned int linkId)
{
    LinkActuator::AttachToSolid(multibody->getLink(linkId+1).solid.get(), I4());
    attachFE_ = multibody;
    attachLinkId_ = linkId;
}

void SuctionCup::Update(Scalar dt)
{
}

void SuctionCup::Engage(SimulationManager* sm)
{
    if(attach_ != nullptr && joint_ == nullptr && pump_)
    {
        btDispatcher* dispatcher = sm->getDynamicsWorld()->getDispatcher();
        int numManifolds = dispatcher->getNumManifolds();
        for(int i=0; i<numManifolds; ++i)
        {
            btPersistentManifold* contactManifold = dispatcher->getManifoldByIndexInternal(i);
            if(contactManifold->getNumContacts() == 0)
                continue;

            btCollisionObject* coA = (btCollisionObject*)contactManifold->getBody0();
            btCollisionObject* coB = (btCollisionObject*)contactManifold->getBody1();
            Entity* entA = (Entity*)coA->getUserPointer();
            Entity* entB = (Entity*)coB->getUserPointer();
            std::unique_ptr<Joint> joint;
            
            if(entA == attach_ && entB->getType() == EntityType::SOLID)
            {
                Transform jointFrame = ((SolidEntity*)entA)->getCG2CTransform();
                jointFrame.setOrigin(contactManifold->getContactPoint(0).getPositionWorldOnA());
                if(attachFE_ != nullptr)
                {
                    joint = std::make_unique<SphericalJoint>(getName() + "/P2P", (SolidEntity*)entB, attachFE_, attachLinkId_, jointFrame.getOrigin(), false);
                    joint->getSolidB()->getRigidBody()->setDamping(0.0, 0.5);
                }
                else
                {
                    joint = std::make_unique<SpringJoint>(getName() + "/Spring", (SolidEntity*)entA, (SolidEntity*)entB, jointFrame, 
                                            Vector3(10,10,10), Vector3(10,10,10), Vector3(0.5,0.5,0.5), Vector3(0.5,0.5,0.5));
                }
            }
            else if(entB == attach_ && entA->getType() == EntityType::SOLID)
            {
                Transform jointFrame = ((SolidEntity*)entB)->getCG2CTransform();
                jointFrame.setOrigin(contactManifold->getContactPoint(0).getPositionWorldOnB());
                if(attachFE_ != nullptr)
                {
                    joint = std::make_unique<SphericalJoint>(getName() + "/P2P", (SolidEntity*)entA, attachFE_, attachLinkId_, jointFrame.getOrigin(), false);
                    joint->getSolidB()->getRigidBody()->setDamping(0.0, 0.5);
                }
                else
                {
                    joint = std::make_unique<SpringJoint>(getName() + "/Spring", (SolidEntity*)entB, (SolidEntity*)entA, jointFrame,
                                            Vector3(10,10,10), Vector3(10,10,10), Vector3(0.5,0.5,0.5), Vector3(0.5,0.5,0.5));
                }
            }

            if(joint != nullptr)
            {
                contactManifold->clearManifold();
                joint_ = joint.get(); // Save pointer to the joint without owning it
                sm->AddJoint(std::move(joint));
                break;
            }
        }
    }
    else if(joint_ != nullptr && !pump_)
    {
        if(joint_->isMultibodyJoint())
            joint_->getSolidB()->getRigidBody()->setDamping(0.0, 0.0);
        sm->RemoveJoint(joint_);
        joint_ = nullptr;
    }
}

// Statics

ConstructInfo SuctionCup::getConstructInfo()
{
    return ConstructInfo();
}

std::unique_ptr<SuctionCup> SuctionCup::Construct(const std::string& uniqueName, ConstructInfo& info)
{
    return std::make_unique<SuctionCup>(uniqueName);
}

REGISTER_ACTUATOR("suction_cup", SuctionCup)

}
