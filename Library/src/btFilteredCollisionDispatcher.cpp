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

btFilteredCollisionDispatcher::btFilteredCollisionDispatcher(btCollisionConfiguration* collisionConfiguration) : btCollisionDispatcher(collisionConfiguration)
{
}

bool btFilteredCollisionDispatcher::needsCollision(const btCollisionObject* body0, const btCollisionObject* body1)
{
    
    if(SimulationApp::getApp()->getSimulationManager()->getCollisionFilter() == INCLUSIVE)
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
    else //EXCLUSIVE
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