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
#include "core/TrajectoryGenerator.h"

namespace sf
{
    //! An abstract class defining an animated simulation entity.
    class AnimatedEntity : public MovingEntity
    {
    public:
        //! A constructor.
        /*!
         \param uniqueName a name for the entity
         \param graphicsFilename a path to the 3d model used for rendering
         \param graphicsScale a scale factor to be used when reading the mesh file
         \param graphicsOrigin a pose of the mesh with respect to the body origin frame
         \param material the name of the material the entity is made of
         \param look the name of the graphical material used for rendering
         */
        AnimatedEntity(std::string uniqueName, std::string graphicsFilename, Scalar graphicsScale, const Transform& graphicsOrigin, std::string material, std::string look = "");
        
        //! A destructor.
        virtual ~AnimatedEntity();
        
        //! A method conneting a trajectroy generator with the animated entity to animate automatically.
        /*!
         \param generator a pointer to the trajectory generator object
         */
        void ConnectTrajectoryGenerator(TrajectoryGenerator* generator);
        
        //! A method updating the transformation and velocities of the animated body.
        /*!
         \param dt time step [s]
         */
        void Update(Scalar dt);

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
        
        //! A method returning the elements that should be rendered.
        std::vector<Renderable> Render();
                  
        //! A method used to set new origin of the entity in the world frame.
        /*!
         \param trans a transformation of the entity origin in the world frame
         */
        void setOTransform(const Transform& trans);
  
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

        //! A method returning the extents of the body axis alligned bounding box.
        /*!
         \param min a point located at the minimum coordinate corner
         \param max a point located at the maximum coordinate corner
         */
        void getAABB(Vector3& min, Vector3& max);
      
        //void AddTrajectoryPoint(Vector3 p, Scalar twist);

        //void RemoveTrajectoryPoint(Vector3 );

        //void ClearTrajectory();        
    private:
        Transform T_O;
        Vector3 vel;
        Vector3 aVel;
        TrajectoryGenerator* tg;
    };
}

#endif
