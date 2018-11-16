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
#include "entities/Entity.h"

typedef enum {FORCEFIELD_POOL, FORCEFIELD_OCEAN, FORCEFIELD_TRIGGER} ForcefieldType;

//Pure virtual class
class ForcefieldEntity : public Entity
{
public:
    ForcefieldEntity(std::string uniqueName);
    virtual ~ForcefieldEntity();
    
    EntityType getType();
    virtual std::vector<Renderable> Render();
	
	btPairCachingGhostObject* getGhost();
    void AddToDynamicsWorld(btMultiBodyDynamicsWorld* world);
    virtual void GetAABB(btVector3& min, btVector3& max);
	
    virtual ForcefieldType getForcefieldType() = 0;
    
protected:
    btPairCachingGhostObject* ghost;
};


#endif
