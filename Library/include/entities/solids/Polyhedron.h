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
//  Polyhedron.h
//  Stonefish
//
//  Created by Patryk Cieslak on 29/12/12.
//  Copyright (c) 2012-2021 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Polyhedron__
#define __Stonefish_Polyhedron__

#include "entities/SolidEntity.h"

namespace sf
{
    //! A class representing a rigid body of any shape described by a polyhedron (mesh with triangle faces).
    class Polyhedron : public SolidEntity
    {
    public:
        //! A constructor.
        /*!
         \param uniqueName a name for the body
         \param phy the specific settings of the physics computation for the body
         \param graphicsFilename a path to the 3d model used for rendering
         \param graphicsScale a scale factor to be used when reading the mesh file
         \param graphicsOrigin a pose of the mesh with respect to the body origin frame
         \param physicsFilename a path to the 3d model used for physics computation (collisions, fluid dynamics)
         \param physicsScale a scale factor to be used when reading the mesh file
         \param physicsOrigin a pose of the mesh with respect to the body origin frame
         \param material the name of the material the body is made of
         \param look the name of the graphical material used for rendering
         \param thickness defines the thickness of the physics geometry walls, if higher than zero the mesh is considered a shell
         \param approx defines what type of approximation of the body shape should be used in the fluid dynamics computation
         */
        Polyhedron(std::string uniqueName, BodyPhysicsSettings phy, 
                   std::string graphicsFilename, Scalar graphicsScale, const Transform& graphicsOrigin,
                   std::string physicsFilename, Scalar physicsScale, const Transform& physicsOrigin,
                   std::string material, std::string look, Scalar thickness = Scalar(-1), GeometryApproxType approx = GeometryApproxType::AUTO);
        
        //! A constructor.
        /*!
         \param uniqueName a name for the body
         \param phy the specific settings of the physics computation for the body
         \param modelFilename a path to the 3d model used for both physics and rendering
         \param scale a scale factor to be used when reading the mesh file
         \param origin a pose of the mesh with respect to the body origin frame
         \param material the name of the material the body is made of
         \param look the name of the graphical material used for rendering
         \param thickness defines the thickness of the model walls, if higher than zero the mesh is considered a shell
         \param approx defines what type of approximation of the body shape should be used in the fluid dynamics computation
         */
        Polyhedron(std::string uniqueName, BodyPhysicsSettings phy, std::string modelFilename, Scalar scale, const Transform& origin,
                   std::string material, std::string look, Scalar thickness = Scalar(-1), GeometryApproxType approx =  GeometryApproxType::AUTO);
        
        //! A destructor.
        ~Polyhedron();
        
        //! A method that returns the type of solid.
        SolidType getSolidType();
        
        //! A method that returns the collision shape.
        btCollisionShape* BuildCollisionShape();
        
        //! A method used to build the graphical representation of the body.
        void BuildGraphicalObject();
        
    private:
        Mesh *graMesh; //Mesh used for rendering
    };
}

#endif
