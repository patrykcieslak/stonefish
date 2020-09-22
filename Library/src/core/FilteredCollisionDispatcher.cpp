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
//  FilteredCollisionDispatcher.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 11/05/2014.
//  Copyright (c) 2014-2020 Patryk Cieslak. All rights reserved.
//

#include "core/FilteredCollisionDispatcher.h"

#include "BulletDynamics/Dynamics/btRigidBody.h"
#include "core/SimulationApp.h"
#include "core/SimulationManager.h"
#include "entities/SolidEntity.h"
#include "sensors/Contact.h"

namespace sf
{

FilteredCollisionDispatcher::FilteredCollisionDispatcher(btCollisionConfiguration* collisionConfiguration, bool inclusiveMode) : btCollisionDispatcher(collisionConfiguration)
{
    inclusive = inclusiveMode;
    //setNearCallback(myNearCallback);
}

bool FilteredCollisionDispatcher::needsCollision(const btCollisionObject* body0, const btCollisionObject* body1)
{
    bool needs = btCollisionDispatcher::needsCollision(body0, body1);
    if(!needs)
        return false;
    
    Entity* ent0 = (Entity*)body0->getUserPointer();
    if(ent0 == NULL)
        return false;
        
    Entity* ent1 = (Entity*)body1->getUserPointer();
    if(ent1 == NULL)
        return false;
    
    if(inclusive)
        return SimulationApp::getApp()->getSimulationManager()->CheckCollision(ent0, ent1) > -1;
    else //exclusive
        return SimulationApp::getApp()->getSimulationManager()->CheckCollision(ent0, ent1) == -1;
}

void FilteredCollisionDispatcher::myNearCallback(btBroadphasePair& collisionPair, btCollisionDispatcher& dispatcher, const btDispatcherInfo& dispatchInfo)
{
    btCollisionObject* colObj0 = (btCollisionObject*)collisionPair.m_pProxy0->m_clientObject;
    btCollisionObject* colObj1 = (btCollisionObject*)collisionPair.m_pProxy1->m_clientObject;
    
    if(dispatcher.needsCollision(colObj0,colObj1))
    {
        btCollisionObjectWrapper obj0Wrap(0, colObj0->getCollisionShape(), colObj0, colObj0->getWorldTransform(), -1, -1);
        btCollisionObjectWrapper obj1Wrap(0, colObj1->getCollisionShape(), colObj1, colObj1->getWorldTransform(), -1, -1);
     
        //Dispatcher will keep algorithms persistent in the collision pair
        if(!collisionPair.m_algorithm)
            collisionPair.m_algorithm = dispatcher.findAlgorithm(&obj0Wrap, &obj1Wrap, 0, BT_CONTACT_POINT_ALGORITHMS);
        
        if(collisionPair.m_algorithm)
        {
            btManifoldResult contactPointResult(&obj0Wrap,&obj1Wrap);
            
            if (dispatchInfo.m_dispatchFunc == 	btDispatcherInfo::DISPATCH_DISCRETE)
            {
                //discrete collision detection query
                collisionPair.m_algorithm->processCollision(&obj0Wrap, &obj1Wrap, dispatchInfo, &contactPointResult);
            }
            else
            {
                //continuous collision detection query, time of impact (toi)
                Scalar toi = collisionPair.m_algorithm->calculateTimeOfImpact(colObj0, colObj1, dispatchInfo, &contactPointResult);
                if (dispatchInfo.m_timeOfImpact > toi)
                    dispatchInfo.m_timeOfImpact = toi;
            }
        }
    }
}

}
