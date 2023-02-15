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
//  Copyright (c) 2023 Patryk Cieslak. All rights reserved.
//

#include "actuators/SuctionCup.h"

#include "core/SimulationManager.h"
#include "joints/SpringJoint.h"

namespace sf
{

SuctionCup::SuctionCup(std::string uniqueName) : LinkActuator(uniqueName)
{
    pump = false;
    spring = nullptr;
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
    if(attach != nullptr)
    {
    }
}

void SuctionCup::Update(Scalar dt)
{
}

void SuctionCup::Engage(SimulationManager* sm)
{
    if(attach != nullptr && spring == nullptr && pump)
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
                Transform springFrame = ((SolidEntity*)entA)->getCG2CTransform();
                springFrame.setOrigin(contactManifold->getContactPoint(0).getPositionWorldOnA());
                spring = new SpringJoint(getName() + "/Spring", (SolidEntity*)entA, (SolidEntity*)entB, springFrame, 
                                            Vector3(10,10,10), Vector3(10,10,10), Vector3(0.5,0.5,0.5), Vector3(0.5,0.5,0.5));
            }
            else if(entB == attach && entA->getType() == EntityType::SOLID)
            {
                Transform springFrame = ((SolidEntity*)entB)->getCG2CTransform();
                springFrame.setOrigin(contactManifold->getContactPoint(0).getPositionWorldOnB());
                spring = new SpringJoint(getName() + "/Spring", (SolidEntity*)entB, (SolidEntity*)entA, springFrame,
                                            Vector3(10,10,10), Vector3(10,10,10), Vector3(0.5,0.5,0.5), Vector3(0.5,0.5,0.5));
            }

            if(spring != nullptr)
            {
                contactManifold->clearManifold();
                sm->AddJoint(spring);
                break;
            }
        }
    }
    else if(spring != nullptr && !pump)
    {
        sm->RemoveJoint(spring);
        spring = nullptr;
    }
}
    
}
