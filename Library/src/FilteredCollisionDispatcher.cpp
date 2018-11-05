//
//  FilteredCollisionDispatcher.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 11/05/2014.
//  Copyright (c) 2014-2017 Patryk Cieslak. All rights reserved.
//

#include <core/FilteredCollisionDispatcher.h>

#include <core/SimulationApp.h>
#include <entities/SolidEntity.h>
#include <BulletDynamics/Dynamics/btRigidBody.h>

FilteredCollisionDispatcher::FilteredCollisionDispatcher(btCollisionConfiguration* collisionConfiguration, bool inclusiveMode) : btCollisionDispatcher(collisionConfiguration)
{
    inclusive = inclusiveMode;
    setNearCallback(myNearCallback);
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
        btCollisionObjectWrapper obj0Wrap(0,colObj0->getCollisionShape(), colObj0, colObj0->getWorldTransform(), -1, -1);
        btCollisionObjectWrapper obj1Wrap(0,colObj1->getCollisionShape(), colObj1, colObj1->getWorldTransform(), -1, -1);
     
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
                btScalar toi = collisionPair.m_algorithm->calculateTimeOfImpact(colObj0, colObj1, dispatchInfo, &contactPointResult);
                if (dispatchInfo.m_timeOfImpact > toi)
                    dispatchInfo.m_timeOfImpact = toi;
            }
            
            btManifoldArray marray;
            collisionPair.m_algorithm->getAllContactManifolds(marray);
            
            Entity* entA = (Entity*)obj0Wrap.m_collisionObject->getUserPointer();
            Entity* entB = (Entity*)obj1Wrap.m_collisionObject->getUserPointer();
            Contact* contact = SimulationApp::getApp()->getSimulationManager()->getContact(entA, entB);
            
            for(int i=0; i<marray.size(); ++i)
            {
                if(marray[i]->getNumContacts() > 0)
                {
                    if(contact != NULL)
                        contact->AddContactPoint(marray[i], contact->getEntityA() != entA);
                        
                    for(int h=0; h<marray[i]->getNumContacts(); ++h)
                    {
                        delete (btVector3*)marray[i]->getContactPoint(h).m_userPersistentData;
                        marray[i]->getContactPoint(h).m_userPersistentData = NULL;
                    }
                }
            }
        }
    }
}
