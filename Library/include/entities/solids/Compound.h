//
//  Compound.h
//  Stonefish
//
//  Created by Patryk Cieslak on 19/09/17.
//  Copyright (c) 2017-2018 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Compound__
#define __Stonefish_Compound__

#include "entities/SolidEntity.h"

namespace sf
{
    //! A structure representing one part of the compound body.
    typedef struct
    {
        SolidEntity* solid;
        Transform origin;
        bool isExternal;
    } CompoundPart;
    
    //! A class representing a rigid body built of multiple other rigid bodies.
    class Compound : public SolidEntity
    {
    public:
        //! A constructor.
        /*!
         \param uniqueName a name for the body
         \param firstExternalPart a pointer to the first external rigid body
         \param origin a transformation from the compound body origin to the first part origin
         \param enableHydrodynamicForces a flag to enable computation of hydrodynamic forces
         */
        Compound(std::string uniqueName, SolidEntity* firstExternalPart, const Transform& origin, bool enableHydrodynamicForces = true);
        
        //! A destructor.
        ~Compound();
        
        //! A method adding new internal rigid body to the compound body.
        /*!
         \param solid a pointer to the rigid part
         \param origin a tranformation from the compound body origin to the part origin
         */
        void AddInternalPart(SolidEntity* solid, const Transform& origin);
        
        //! A method adding new external rigid body to the compound body.
        /*!
         \param solid a pointer to the rigid part
         \param origin a tranformation from the compound body origin to the part origin
         */
        void AddExternalPart(SolidEntity* solid, const Transform& origin);
        
        //! A method which computes hydrodynamic forces acting on the compound body.
        /*!
         \param settings a structure holding settings of the hydrodynamic computation
         \param liquid a pointer to a fluid entity (only Ocean supported now)
         */
        void ComputeFluidForces(HydrodynamicsSettings settings, Ocean* liquid);
        
        //! A method returning the material of the body.
        Material getMaterial(size_t partId) const;
        
        //! A method returning the part id for the collision shape id
        size_t getPartId(size_t collisionShapeId) const;
        
        //! A method that returns the type of solid.
        SolidType getSolidType();
        
        //! A method that returns a copy of all physics mesh vertices.
        std::vector<Vertex>* getMeshVertices();
        
        //! A method that constructs a collision shape for the body.
        btCollisionShape* BuildCollisionShape();
        
        //! A method that builds a graphical object for the body.
        void BuildGraphicalObject();
        
        //! A method that returns elements that have to be rendered for the body.
        std::vector<Renderable> Render();
        
    private:
        std::vector<CompoundPart> parts; //Parts of the compound solid
        std::vector<size_t> collisionPartId;
        
        void RecalculatePhysicalProperties();
    };

}

#endif
