//
//  btFilteredCollisionDispatcher.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 11/05/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#include "btFilteredCollisionDispatcher.h"
#include <BulletDynamics/Dynamics/btRigidBody.h>
#include "Entity.h"
#include "SolidEntity.h"
#include "SimulationApp.h"

btFilteredCollisionDispatcher::btFilteredCollisionDispatcher(btCollisionConfiguration* collisionConfiguration, bool inclusiveMode) : btCollisionDispatcher(collisionConfiguration)
{
    inclusive = inclusiveMode;
    if(inclusive)
        setNearCallback(myNearCallback);
}

bool btFilteredCollisionDispatcher::needsCollision(const btCollisionObject* body0, const btCollisionObject* body1)
{
    
    if(inclusive)
    {
        //get entities from collision objects
        const btRigidBody* rb0 = btRigidBody::upcast(body0);
        Entity* ent0 = (Entity*)rb0->getUserPointer();
        if(ent0 == NULL)
            return false;
        
        const btRigidBody* rb1 = btRigidBody::upcast(body1);
        Entity* ent1 = (Entity*)rb1->getUserPointer();
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
            const btRigidBody* rb0 = btRigidBody::upcast(body0);
            Entity* ent0 = (Entity*)rb0->getUserPointer();
            if(ent0 == NULL)
                return true; //really?
            
            const btRigidBody* rb1 = btRigidBody::upcast(body1);
            Entity* ent1 = (Entity*)rb1->getUserPointer();
            if(ent1 == NULL)
                return true; //really?
            
            return !SimulationApp::getApp()->getSimulationManager()->CheckContact(ent0, ent1);
        }
        
        return false;
    }
}

void btFilteredCollisionDispatcher::myNearCallback(btBroadphasePair& collisionPair, btCollisionDispatcher& dispatcher, const btDispatcherInfo& dispatchInfo)
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
                
                if(contactPointResult.getPersistentManifold()->getNumContacts() > 0)
                {
                    const btRigidBody* rb0 = btRigidBody::upcast(colObj0);
                    const btRigidBody* rb1 = btRigidBody::upcast(colObj1);
                    Entity* ent0 = (Entity*)rb0->getUserPointer();
                    Entity* ent1 = (Entity*)rb1->getUserPointer();
                    Contact* contact = SimulationApp::getApp()->getSimulationManager()->getContact(ent0, ent1);
                    contact->AddContactPoint(contactPointResult.getPersistentManifold());
                }
            }
            else
            {
                //continuous collision detection query, time of impact (toi)
                btScalar toi = collisionPair.m_algorithm->calculateTimeOfImpact(colObj0,colObj1,dispatchInfo,&contactPointResult);
                if (dispatchInfo.m_timeOfImpact > toi)
                    dispatchInfo.m_timeOfImpact = toi;
            }
        }
    }
}
