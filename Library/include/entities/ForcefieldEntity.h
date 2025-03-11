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
//  ForcefieldEntity.h
//  Stonefish
//
//  Created by Patryk Cieslak on 1/3/13.
//  Copyright (c) 2013-2020 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_ForcefieldEntity__
#define __Stonefish_ForcefieldEntity__

#include "BulletCollision/CollisionDispatch/btGhostObject.h"
#include "entities/Entity.h"

namespace sf
{
    //! An enum specifying the type of forcefield.
    enum class ForcefieldType {POOL, OCEAN, TRIGGER, ATMOSPHERE};
    
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
        EntityType getType() const;
        
    protected:
        btPairCachingGhostObject* ghost;
    };
}

#endif
