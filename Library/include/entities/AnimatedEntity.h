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
//  AnimatedEntity.h
//  Stonefish
//
//  Created by Patryk Cieslak on 15/07/2020.
//  Copyright (c) 2020 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_AnimatedEntity__
#define __Stonefish_AnimatedEntity__

#include "entities/MovingEntity.h"
#include "entities/animation/Trajectory.h"

namespace sf
{
    //! An abstract class defining an animated simulation entity.
    class AnimatedEntity : public MovingEntity
    {
    public:
        //! A constructor building an empty body.
        /*!
         \param uniqueName a name for the entity
         \param traj a pointer to the body trajectory
         */
        AnimatedEntity(std::string uniqueName, Trajectory* traj);

        //! A constructor building a spherical body.
        /*!
         \param uniqueName a name for the entity
         \param traj a pointer to the body trajectory
         \param sphereRadius the radius of the sphere [m]
         \param origin a pose of the mesh with respect to the body origin frame
         \param material the name of the material the entity is made of
         \param look the name of the graphical material used for rendering
         \param collides a flag determining if the body can collide with other bodies
         */
        AnimatedEntity(std::string uniqueName, Trajectory* traj, Scalar sphereRadius, const Transform& origin, std::string material, std::string look = "", bool collides = false);

        //! A constructor building a cylindrical body.
        /*!
         \param uniqueName a name for the entity
         \param traj a pointer to the body trajectory
         \param cylinderRadius the radius of the cylinder [m]
         \param cylinderHeight the height of the cylinder [m]
         \param origin a pose of the mesh with respect to the body origin frame
         \param material the name of the material the entity is made of
         \param look the name of the graphical material used for rendering
         \param collides a flag determining if the body can collide with other bodies
         */
        AnimatedEntity(std::string uniqueName, Trajectory* traj, Scalar cylinderRadius, Scalar cylinderHeight, const Transform& origin, std::string material, std::string look = "", bool collides = false);

        //! A constructor building a box body.
        /*!
         \param uniqueName a name for the entity
         \param traj a pointer to the body trajectory
         \param boxDimensions the dimensions of the box [m]
         \param origin a pose of the mesh with respect to the body origin frame
         \param material the name of the material the entity is made of
         \param look the name of the graphical material used for rendering
         \param collides a flag determining if the body can collide with other bodies
         */
        AnimatedEntity(std::string uniqueName, Trajectory* traj, Vector3 boxDimensions, const Transform& origin, std::string material, std::string look = "", bool collides = false);
        
        //! A constructor building a mesh body. 
        /*!
         \param uniqueName a name for the entity
         \param traj a pointer to the body trajectory
         \param modelFilename a path to the 3d model used for rendering
         \param scale a scale factor to be used when reading the mesh file
         \param origin a pose of the mesh with respect to the body origin frame
         \param material the name of the material the entity is made of
         \param look the name of the graphical material used for rendering
         \param collides a flag determining if the body can collide with other bodies
         */
        AnimatedEntity(std::string uniqueName, Trajectory* traj, std::string modelFilename, Scalar scale, const Transform& origin,
                       std::string material, std::string look = "", bool collides = false);
        
        //! A constructor building a mesh body. 
        /*!
         \param uniqueName a name for the entity
         \param traj a pointer to the body trajectory
         \param graphicsFilename a path to the 3d model used for rendering
         \param graphicsScale a scale factor to be used when reading the mesh file
         \param graphicsOrigin a pose of the mesh with respect to the body origin frame
         \param physicsFilename a path to the 3d model used for collision
         \param physicsScale a scale factor to be used when reading the mesh file
         \param physicsOrigin a pose of the mesh with respect to the body origin frame
         \param material the name of the material the entity is made of
         \param look the name of the graphical material used for rendering
         \param collides a flag determining if the body can collide with other bodies
         */
        AnimatedEntity(std::string uniqueName, Trajectory* traj, std::string graphicsFilename, Scalar graphicsScale, const Transform& graphicsOrigin,
                       std::string physicsFilename, Scalar physicsScale, const Transform& physicsOrigin, std::string material, std::string look = "", bool collides = false);
        
        //! A destructor.
        virtual ~AnimatedEntity();

        //! A method adding the body to the simulation manager.
        /*!
         \param sm a pointer to the simulation manager
         */
        void AddToSimulation(SimulationManager* sm);
        
        //! A method adding the body to the simulation manager.
        /*!
         \param sm a pointer to the simulation manager
         \param origin a pose of the body CG in the world frame
         */
        void AddToSimulation(SimulationManager* sm, const Transform& origin);

        //! A method updating the position and velocity of the body.
        /*!
         \param dt a time step [s]
         */
        void Update(Scalar dt);
        
        //! A method returning the elements that should be rendered.
        std::vector<Renderable> Render();
        
        //! A method returning the type of the entity.
        EntityType getType() const;
        
        //! A method returning the pose of the body origin in the world frame.
        Transform getOTransform() const;
    
        //! A method returning the pose of the body in the world frame.
        Transform getCGTransform() const;

        //! A method returning the linear velocity of the body.
        Vector3 getLinearVelocity() const;
        
        //! A method returning the angular velocity of the body.
        Vector3 getAngularVelocity() const;
        
        //! A method returning the linear velocity of the body at the given point.
        Vector3 getLinearVelocityInLocalPoint(const Vector3& relPos) const;

        //! A method returning the linear acceleration of the body.
        Vector3 getLinearAcceleration() const;
        
        //! A method returning the angular acceleration of the body.
        Vector3 getAngularAcceleration() const;

        //! A method returning a pointer to the body trajectory.
        Trajectory* getTrajectory();

        //! A method returning the extents of the body axis alligned bounding box.
        /*!
         \param min a point located at the minimum coordinate corner
         \param max a point located at the maximum coordinate corner
         */
        void getAABB(Vector3& min, Vector3& max);
      
    private:
        void BuildRigidBody(btCollisionShape* shape, bool collides);

        Transform T_CG2O;
        Transform T_O2G;
        Transform T_O2C;
        Trajectory* tr;
        int phyObjectId;
    };
}

#endif
