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
//  Copyright (c) 2023-2024 Patryk Cieslak. All rights reserved.
//

#include "actuators/SuctionCup.h"

#include "core/SimulationManager.h"
#include "entities/FeatherstoneEntity.h"
#include "joints/SpringJoint.h"
#include "joints/SphericalJoint.h"

namespace sf
{

SuctionCup::SuctionCup(std::string uniqueName) : LinkActuator(uniqueName)
{
    pump = false;
    joint = nullptr;
    attachFe = nullptr;
    attachLinkId = 0;
}

SuctionCup::~SuctionCup()
{
}

ActuatorType SuctionCup::getType() const
{
    return ActuatorType::SUCTION_CUP;
}

void SuctionCup::setPump(bool enabled)
{
    pump = enabled;
}

bool SuctionCup::getPump() const
{
    return pump;
}

void SuctionCup::AttachToSolid(SolidEntity* body, const Transform& origin)
{
    LinkActuator::AttachToSolid(body, origin);
}

void SuctionCup::AttachToLink(FeatherstoneEntity* multibody, unsigned int linkId)
{
    LinkActuator::AttachToSolid(multibody->getLink(linkId+1).solid, I4());
    attachFe = multibody;
    attachLinkId = linkId;
}

void SuctionCup::Update(Scalar dt)
{
}

void SuctionCup::Engage(SimulationManager* sm)
{
    if(attach != nullptr && joint == nullptr && pump)
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
            
            if(entA == attach && entB->getType() == EntityType::SOLID)
            {
                Transform jointFrame = ((SolidEntity*)entA)->getCG2CTransform();
                jointFrame.setOrigin(contactManifold->getContactPoint(0).getPositionWorldOnA());
                if(attachFe != nullptr)
                {
                    joint = new SphericalJoint(getName() + "/P2P", (SolidEntity*)entB, attachFe, attachLinkId, jointFrame.getOrigin(), false);
                    joint->getSolidB()->getRigidBody()->setDamping(0.0, 0.5);
                }
                else
                {
                    joint = new SpringJoint(getName() + "/Spring", (SolidEntity*)entA, (SolidEntity*)entB, jointFrame, 
                                            Vector3(10,10,10), Vector3(10,10,10), Vector3(0.5,0.5,0.5), Vector3(0.5,0.5,0.5));
                }
            }
            else if(entB == attach && entA->getType() == EntityType::SOLID)
            {
                Transform jointFrame = ((SolidEntity*)entB)->getCG2CTransform();
                jointFrame.setOrigin(contactManifold->getContactPoint(0).getPositionWorldOnB());
                if(attachFe != nullptr)
                {
                    joint = new SphericalJoint(getName() + "/P2P", (SolidEntity*)entA, attachFe, attachLinkId, jointFrame.getOrigin(), false);
                    joint->getSolidB()->getRigidBody()->setDamping(0.0, 0.5);
                }
                else
                {
                    joint = new SpringJoint(getName() + "/Spring", (SolidEntity*)entB, (SolidEntity*)entA, jointFrame,
                                            Vector3(10,10,10), Vector3(10,10,10), Vector3(0.5,0.5,0.5), Vector3(0.5,0.5,0.5));
                }
            }

            if(joint != nullptr)
            {
                contactManifold->clearManifold();
                sm->AddJoint(joint);
                break;
            }
        }
    }
    else if(joint != nullptr && !pump)
    {
        if(joint->isMultibodyJoint())
            joint->getSolidB()->getRigidBody()->setDamping(0.0, 0.0);
        sm->RemoveJoint(joint);
        joint = nullptr;
    }
}
    
}
