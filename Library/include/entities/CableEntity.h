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
//  CableEntity.h
//  Stonefish
//
//  Created by Patryk Cieslak on 12/11/25.
//  Copyright(c) 2025 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_CableEntity__
#define __Stonefish_CableEntity__

#include "BulletSoftBody/btSoftBody.h"
#include "core/MaterialManager.h"
#include "entities/Entity.h"
#include "graphics/OpenGLDataStructs.h"

namespace sf    
{
    struct HydrodynamicsSettings;
    class SolidEntity;
    class FeatherstoneEntity;
    class Ocean;

    enum class CableEnds {NONE, FIRST, SECOND, BOTH};

    struct CableNodalForces
    {
        // Hydrodynamic forces
        Vector3 Fb; // Buoyancy force
        Vector3 Fdq; // Form drag force
        Vector3 Fdf; // Skin friction force

        void clearForces()
        {
            Fb.setZero();
            Fdq.setZero();
            Fdf.setZero();
        }
    };

    //! A class representing a cable.
    class CableEntity : public Entity
    {
    public:
        //! Constructor of the CableEntity class.
        /*!
         \param uniqueName the name of the cable entity
         \param phy the physics settings for the cable
         \param firstEnd the starting point of the cable in the world frame
         \param secondEnd the ending point of the cable in the world frame
         \param numSegments the number of segments the cable is divided into
         \param diameter the diameter of the cable
         \param material the name of the material the cable is made of
         \param look the name of the graphical material used for rendering
         \param stretching the allowed stretching factor of the cable (0.0 means inextensible)
         \param uvScale a scaling factor for texture coordinates
         */
        CableEntity(std::string uniqueName, PhysicsSettings phy, Vector3 firstEnd, Vector3 secondEnd, size_t numSegments, 
            Scalar diameter, std::string material, std::string look, Scalar stretching = 0.0, float uvScale = 1.0f);

        //! A mrethod attaching the cable ends to the world.
        /*! 
         \param ends specifies which ends of the cable should be attached
         */
        void AttachToWorld(CableEnds ends);

        //! A method attaching the cable ends to a solid body.
        /*! 
         \param ends specifies which ends of the cable should be attached
         \param solid a pointer to the solid entity
         */
        void AttachToSolid(CableEnds ends, SolidEntity* solid);

        //! A method adding the body to the simulation manager.
        /*!
         \param sm a pointer to the simulation manager
         */
        void AddToSimulation(SimulationManager* sm) override;

        //! A method which applies gravity to the body.
        /*!
         \param g a vector specifing the gravitational acceleration
         */
        void ApplyGravity(const Vector3& g);

        //! A method that computes fluid dynamics based on selected settings.
        /*!
         \param settings a structure holding settings of fluid dynamics computation
         \param ocn a pointer to the ocean entity
         */
        void ComputeHydrodynamicForces(HydrodynamicsSettings settings, Ocean* ocn);

        //! A method which applies precomputed hydrodynamic forces to the cable.
        void ApplyHydrodynamicForces();

        //! A method returning the elements that should be rendered.
        std::vector<Renderable> Render() override;

        //! A method used to set display mode used for the body.
        /*!
         \param m flag defining the display mode
         */
        void setDisplayMode(DisplayMode m);

        //! A method returning the type of the entity.
        EntityType getType() const override;

        //! A method returning the rest length of the cable.
        Scalar getRestLength() const;

        //! A method returning the current lenght of the cable.
        Scalar getLength() const;

        //! A method returning the underlying soft body object.
        btSoftBody* getSoftBody() const;

        //! A method returning the extents of the entity axis alligned bounding box.
        /*!
         \param min a point located at the minimum coordinate corner
         \param max a point located at the maximum coordinate corner
         */
        void getAABB(Vector3& min, Vector3& max) override;
                
    private:
        Scalar circularSegmentArea(Scalar h) const;

        btSoftBody* cableBody_;
        Scalar radius_;
        Material mat_;
        std::vector<CableNodalForces> nodalForces_;
        Scalar restLength_;
        PhysicsSettings phy_;

        size_t objectId_;
        int lookId_;
        float uvScale_;
        DisplayMode displayMode_;
        size_t numGraphicalNodes_;
    };
}

#endif /* defined(__Stonefish_CableEntity__) */