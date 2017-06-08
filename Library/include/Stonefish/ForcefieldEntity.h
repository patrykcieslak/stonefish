//
//  ForcefieldEntity.h
//  Stonefish
//
//  Created by Patryk Cieslak on 1/3/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_ForcefieldEntity__
#define __Stonefish_ForcefieldEntity__

#include <BulletCollision/CollisionDispatch/btGhostObject.h>
#include "Entity.h"

typedef enum {FORCEFIELD_FLUID} ForcefieldType;

//Pure virtual class
class ForcefieldEntity : public Entity
{
public:
    ForcefieldEntity(std::string uniqueName);
    virtual ~ForcefieldEntity();
    
    EntityType getType();
    virtual void Render();
	
	btPairCachingGhostObject* getGhost();
    void AddToDynamicsWorld(btMultiBodyDynamicsWorld* world);
    
    virtual ForcefieldType getForcefieldType() = 0;
    
protected:
    btPairCachingGhostObject* ghost;
};


#endif
