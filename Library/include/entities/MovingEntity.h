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
//  MovingEntity.h
//  Stonefish
//
//  Created by Patryk Cieslak on 15/07/20.
//  Copyright(c) 2025 Patryk Cieslak. All rights reserved.
//

#pragma once

#include "core/MaterialManager.h"
#include "entities/Entity.h"
#include "graphics/OpenGLDataStructs.h"
#include <memory>

namespace sf
{
    class OpenGLOceanParticles;

    //! An abstract class representing a moving rigid body.
    class MovingEntity : public Entity
    {
    public:
        //! A constructor.
        /*!
         \param uniqueName a name for the body
         \param material the name of the material the body is made of
         \param look the name of the graphical material used for rendering
         */
        MovingEntity(std::string uniqueName, std::string material, std::string look);
        
        //! A destructor.
        virtual ~MovingEntity();
        
        //! A method adding the body to the simulation manager.
        /*!
         \param sm a pointer to the simulation manager
         */
        virtual void AddToSimulation(SimulationManager* sm) = 0;
        
        //! A method adding the body to the simulation manager.
        /*!
         \param sm a pointer to the simulation manager
         \param origin a pose of the body CG in the world frame
         */
        virtual void AddToSimulation(SimulationManager* sm, const Transform& origin) = 0;
        
        //! A method returning the elements that should be rendered.
        virtual std::vector<Renderable> Render() = 0;

        //! A method returning the type of the entity.
        virtual EntityType getType() const = 0;
        
        //! A method returning the pose of the body origin in the world frame.
        virtual Transform getOTransform() const = 0;

        //! A method returning the pose of the body in the world frame.
        virtual Transform getCGTransform() const = 0;
        
        //! A method returning the linear velocity of the body.
        virtual Vector3 getLinearVelocity() const = 0;
        
        //! A method returning the linear velocity of the body at the given point.
        virtual Vector3 getLinearVelocityInLocalPoint(const Vector3& relPos) const = 0;
        
        //! A method returning the angular velocity of the body.
        virtual Vector3 getAngularVelocity() const = 0;

        //! A method returning the linear acceleration of the body.
        virtual Vector3 getLinearAcceleration() const = 0;
        
        //! A method returning the angular acceleration of the body.
        virtual Vector3 getAngularAcceleration() const = 0;

        //! A method setting the linear acceleration of the body.
        /*!
         \param a linear acceleration [m s^-2]
         */
        void setLinearAcceleration(Vector3 a);
        
        //! A method setting the angular acceleration of the body.
        /*!
         \param epsilon angular acceleration [rad s^-2]
         */
        void setAngularAcceleration(Vector3 epsilon);

        //! A method returning the extents of the body axis alligned bounding box.
        /*!
         \param min a point located at the minimum coordinate corner
         \param max a point located at the maximum coordinate corner
         */
        virtual void getAABB(Vector3& min, Vector3& max) = 0;
        
        //! A method returning the material of the body.
        Material getMaterial() const;
        
        //! A method used to change the rendering style of the object.
        /*!
         \param newLookId an index of the graphical material that should be used to render the body
         */
        void setLook(int newLookId);
        
        //! A method used to set display mode used for the body.
        /*!
         \param m flag defining the display mode
         */
        void setDisplayMode(DisplayMode m);
        
        //! A method returning the index of the graphical material used in rendering.
        int getLook() const;
        
        //! A method returning the index of the graphical object used in rendering.
        int getGraphicalObject() const;

        //! A method returning the associated particles system.
        std::shared_ptr<OpenGLOceanParticles> getOceanParticles();

        //! A method returning the rigid body associated with the entity.
        btRigidBody* getRigidBody();
        
    protected:
        //Body
        btRigidBody* rigidBody;
        Material mat;

        //Motion
        Vector3 filteredLinearVel;
        Vector3 filteredAngularVel;
        Vector3 linearAcc;
        Vector3 angularAcc;

        //Display
        int lookId;
        int graObjectId;
        DisplayMode dm;
        std::shared_ptr<OpenGLOceanParticles> particles;

    private:
    };
}
