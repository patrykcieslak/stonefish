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
//  StaticEntity.h
//  Stonefish
//
//  Created by Patryk Cieslak on 24/05/2014.
//  Copyright (c) 2014-2019 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_StaticEntity__
#define __Stonefish_StaticEntity__

#include "core/MaterialManager.h"
#include "entities/Entity.h"

namespace sf
{
    //! An enum specifiying the type of the static entity.
    typedef enum {STATIC_PLANE, STATIC_TERRAIN, STATIC_OBSTACLE} StaticEntityType;
    
    struct Mesh;
    
    //! An abstract class defining a static simulation entity.
    class StaticEntity : public Entity
    {
    public:
        //! A constructor.
        /*!
         \param uniqueName a name for the entity
         \param m the material of the entity
         \param lookId index of the material used when rendering the entity
         */
        StaticEntity(std::string uniqueName, Material m, int lookId = -1);
        
        //! A destructor.
        virtual ~StaticEntity();
        
        //! A method implementing the rendering of the entity.
        std::vector<Renderable> Render();
        
        //! A method used to add the static entity to the simulation.
        /*!
         \param sm a pointer to the simulation manager
         */
        void AddToSimulation(SimulationManager* sm);
        
        //! A method used to add the static entity to the simulation.
        /*!
         \param sm a pointer to the simulation manager
         \param origin a transformation of the entity origin in the world frame
         */
        virtual void AddToSimulation(SimulationManager* sm, const Transform& origin);
        
        //! A method returning the extents of the entity axis alligned bounding box.
        /*!
         \param min a point located at the minimum coordinate corner
         \param max a point located at the maximum coordinate corner
         */
        virtual void getAABB(Vector3& min, Vector3& max);
        
        //! A method used to set new origin of the entity in the world frame.
        /*!
         \param trans a transformation of the entity origin in the world frame
         */
        void setTransform(const Transform& trans);
        
        //! A method returning the transformation of the entity origin in the world frame.
        Transform getTransform();
        
        //! A method returning the material of the entity.
        Material getMaterial();
        
        //! A method returning the rigid body associated with the entity.
        btRigidBody* getRigidBody();
        
        //! A method returning the type of the entity.
        EntityType getType();
        
        //! A method returninf the type of the static entity.
        virtual StaticEntityType getStaticType() = 0;
        
        //! A static method used to transform a group of static entities together (useful to change the position of multiple linked objects).
        /*!
         \param objects a vector holiding a list of pointers to the objects that are to be transformed
         \param centre the center of the transformation in the world frame
         \param transform the transformation to be applied to the group in the world frame
         */
        static void GroupTransform(std::vector<StaticEntity*>& objects, const Transform& centre, const Transform& transform);
        
    protected:
        void BuildRigidBody(btCollisionShape* shape);
        virtual void BuildGraphicalObject();
        
        btRigidBody* rigidBody;
        Material mat;
        Mesh* phyMesh;
        
        int objectId;
        int lookId;
    };
}

#endif
