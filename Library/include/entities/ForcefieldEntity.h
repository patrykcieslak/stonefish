//
//  ForcefieldEntity.h
//  Stonefish
//
//  Created by Patryk Cieslak on 1/3/13.
//  Copyright (c) 2013-2019 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_ForcefieldEntity__
#define __Stonefish_ForcefieldEntity__

#include <BulletCollision/CollisionDispatch/btGhostObject.h>
#include "entities/Entity.h"

namespace sf
{
    //! An enum specifying the type of forcefield.
    typedef enum {FORCEFIELD_POOL, FORCEFIELD_OCEAN, FORCEFIELD_TRIGGER, FORCEFIELD_ATMOSPHERE} ForcefieldType;
    
    //! An abstract class representing some kind of a force field.
    class ForcefieldEntity : public Entity
    {
    public:
        //! A constructor.
        /*!
         \param uniqueName a name for the force field
         */
        ForcefieldEntity(std::string uniqueName);
        
        //! A destructor.
        virtual ~ForcefieldEntity();
        
        //! A method used to add the force field to the simulation.
        /*!
         \param sm a pointer to the simulation manager
         */
        void AddToSimulation(SimulationManager* sm);
        
        //! A method implementing the rendering of the force field.
        virtual std::vector<Renderable> Render();
        
        //! A method returning the extents of the force field axis alligned bounding box.
        /*!
         \param min a point located at the minimum coordinate corner
         \param max a point located at the maximum coordinate corner
         */
        virtual void getAABB(Vector3& min, Vector3& max);
        
        //! A method returning the pair caching object for the force field.
        btPairCachingGhostObject* getGhost();
        
        //! A method returning the type of the force field.
        virtual ForcefieldType getForcefieldType() = 0;
        
        //! A method returning the type of the entity.
        EntityType getType();
        
    protected:
        btPairCachingGhostObject* ghost;
    };
}

#endif
