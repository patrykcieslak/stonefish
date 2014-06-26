//
//  GhostEntity.h
//  Stonefish
//
//  Created by Patryk Cieslak on 1/3/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_GhostEntity__
#define __Stonefish_GhostEntity__

#include <BulletCollision/CollisionDispatch/btGhostObject.h>
#include "Entity.h"

typedef enum {FLUID} GhostType;

//pure virtual class
class GhostEntity : public Entity
{
public:
    GhostEntity(std::string uniqueName);
    virtual ~GhostEntity();
    
    EntityType getType();
    btPairCachingGhostObject* getGhost();
    void AddToDynamicsWorld(btMultiBodyDynamicsWorld* world);
    
    virtual GhostType getGhostType() = 0;
    virtual void Render() = 0;
    
protected:
    btPairCachingGhostObject* ghost;
};


#endif
