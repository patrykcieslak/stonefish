//
//  Entity.h
//  Stonefish
//
//  Created by Patryk Cieslak on 11/28/12.
//  Copyright (c) 2012-2019 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Entity__
#define __Stonefish_Entity__

#define BIT(x) (1<<(x))

#include "StonefishCommon.h"

namespace sf
{
    //! An enum specifying the type of entity.
    typedef enum { ENTITY_STATIC, ENTITY_SOLID, ENTITY_FEATHERSTONE, ENTITY_FORCEFIELD } EntityType;
    
    //! An enum used for collision filtering.
    typedef enum
    {
        MASK_NONCOLLIDING = 0,
        MASK_STATIC = BIT(0),
        MASK_DEFAULT = BIT(1)
    }
    CollisionMask;
    
    struct Renderable;
    class SimulationManager;
    
    //! An abstract class representing a simulation entity.
    class Entity
    {
    public:
        //! A constructor.
        /*!
         \param uniqueName a name for the entity
         */
        Entity(std::string uniqueName);
        
        //! A destructor.
        virtual ~Entity();
        
        //! A method used to set if the entity should be renderable.
        /*!
         \param render a flag informing if the entity should be rendered
         */
        void setRenderable(bool render);
        
        //! A method informing if the entity is renderable.
        bool isRenderable();
        
        //! A method returning the name of the entity.
        std::string getName();
        
        //! A method returning the type of the entity.
        virtual EntityType getType() = 0;
        
        //! A method implementing rendering of the entity.
        virtual std::vector<Renderable> Render() = 0;
        
        //! A method used to add the entity to the simulation.
        /*!
         \param sm a pointer to a simulation manager
         */
        virtual void AddToSimulation(SimulationManager* sm) = 0;
        
        //! A method returning the extents of the entity axis alligned bounding box.
        /*!
         \param min a point located at the minimum coordinate corner
         \param max a point located at the maximum coordinate corner
         */
        virtual void getAABB(Vector3& min, Vector3& max) = 0;
        
    protected:
        static Vector3 findInertiaAxis(Matrix3 I, Scalar value);
        
    private:
        bool renderable;
        std::string name;
    };
}

#endif
