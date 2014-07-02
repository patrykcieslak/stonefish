//
//  FilteredCollisionDispatcher.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 11/05/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#include "FilteredCollisionDispatcher.h"
#include <BulletDynamics/Dynamics/btRigidBody.h>
#include "Entity.h"
#include "SolidEntity.h"
#include "SimulationApp.h"

FilteredCollisionDispatcher::FilteredCollisionDispatcher(btCollisionConfiguration* collisionConfiguration, bool inclusiveMode) : btCollisionDispatcher(collisionConfiguration)
{
    inclusive = inclusiveMode;
    if(inclusive)
        setNearCallback(myNearCallback);
}

bool FilteredCollisionDispatcher::needsCollision(const btCollisionObject* body0, const btCollisionObject* body1)
{
    if(inclusive)
    {
        Entity* ent0 = (Entity*)body0->getUserPointer();
        if(ent0 == NULL)
            return false;
        
        Entity* ent1 = (Entity*)body1->getUserPointer();
        if(ent1 == NULL)
            return false;
        
        //check if there exists a contact joint
        return SimulationApp::getApp()->getSimulationManager()->CheckContact(ent0, ent1);
    }
    else //exclusive
    {
        bool needs = btCollisionDispatcher::needsCollision(body0, body1);
        
        if(needs)
        {
            Entity* ent0 = (Entity*)body0->getUserPointer();
            if(ent0 == NULL)
                return true;
            
            Entity* ent1 = (Entity*)body1->getUserPointer();
            if(ent1 == NULL)
                return true;
            
            return !SimulationApp::getApp()->getSimulationManager()->CheckContact(ent0, ent1);
        }
        else
            return false;
    }
}

void FilteredCollisionDispatcher::myNearCallback(btBroadphasePair& collisionPair, btCollisionDispatcher& dispatcher, const btDispatcherInfo& dispatchInfo)
{
    btCollisionObject* colObj0 = (btCollisionObject*)collisionPair.m_pProxy0->m_clientObject;
    btCollisionObject* colObj1 = (btCollisionObject*)collisionPair.m_pProxy1->m_clientObject;
    
    if (dispatcher.needsCollision(colObj0,colObj1))
    {
        btCollisionObjectWrapper obj0Wrap(0,colObj0->getCollisionShape(),colObj0,colObj0->getWorldTransform(),-1,-1);
        btCollisionObjectWrapper obj1Wrap(0,colObj1->getCollisionShape(),colObj1,colObj1->getWorldTransform(),-1,-1);
        
        //dispatcher will keep algorithms persistent in the collision pair
        if (!collisionPair.m_algorithm)
        {
            collisionPair.m_algorithm = dispatcher.findAlgorithm(&obj0Wrap,&obj1Wrap);
        }
        
        if (collisionPair.m_algorithm)
        {
            btManifoldResult contactPointResult(&obj0Wrap,&obj1Wrap);
            
            if (dispatchInfo.m_dispatchFunc == 	btDispatcherInfo::DISPATCH_DISCRETE)
            {
                //discrete collision detection query
                collisionPair.m_algorithm->processCollision(&obj0Wrap,&obj1Wrap,dispatchInfo,&contactPointResult);
            }
            else
            {
                //continuous collision detection query, time of impact (toi)
                btScalar toi = collisionPair.m_algorithm->calculateTimeOfImpact(colObj0,colObj1,dispatchInfo,&contactPointResult);
                if (dispatchInfo.m_timeOfImpact > toi)
                    dispatchInfo.m_timeOfImpact = toi;
            }
            
            //Add contact point information
            if(contactPointResult.getPersistentManifold()->getNumContacts() > 0)
            {
                //Add contact point to Contact object
                Entity* entA = (Entity*)obj0Wrap.m_collisionObject->getUserPointer();
                Entity* entB = (Entity*)obj1Wrap.m_collisionObject->getUserPointer();
                Contact* contact = SimulationApp::getApp()->getSimulationManager()->getContact(entA, entB);
                contact->AddContactPoint(contactPointResult.getPersistentManifold(), contact->getEntityA() != entA);
                
                //Clear persistent user data (it will leak if not!)
                for(int i = 0; i < contactPointResult.getPersistentManifold()->getNumContacts(); i++)
                {
                    delete (btVector3*)contactPointResult.getPersistentManifold()->getContactPoint(i).m_userPersistentData;
                    contactPointResult.getPersistentManifold()->getContactPoint(i).m_userPersistentData = 0;
                }
            }
        }
    }
}
