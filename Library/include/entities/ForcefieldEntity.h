//
//  ForcefieldEntity.h
//  Stonefish
//
//  Created by Patryk Cieslak on 1/3/13.
//  Copyright (c) 2013-2018 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_ForcefieldEntity__
#define __Stonefish_ForcefieldEntity__

#include <BulletCollision/CollisionDispatch/btGhostObject.h>
#include "entities/Entity.h"

namespace sf
{
    //!
    typedef enum {FORCEFIELD_POOL, FORCEFIELD_OCEAN, FORCEFIELD_TRIGGER, FORCEFIELD_ATMOSPHERE} ForcefieldType;
    
    //!
    class ForcefieldEntity : public Entity
    {
    public:
        ForcefieldEntity(std::string uniqueName);
        virtual ~ForcefieldEntity();
        
        void AddToSimulation(SimulationManager* sm);
        virtual std::vector<Renderable> Render();
        
        virtual void getAABB(Vector3& min, Vector3& max);
        btPairCachingGhostObject* getGhost();
        virtual ForcefieldType getForcefieldType() = 0;
        EntityType getType();
        
    protected:
        btPairCachingGhostObject* ghost;
    };
}

#endif
