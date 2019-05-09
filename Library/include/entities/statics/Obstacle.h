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
//  Obstacle.h
//  Stonefish
//
//  Created by Patryk Cieslak on 24/05/2014.
//  Copyright(c) 2014-2018 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Obstacle__
#define __Stonefish_Obstacle__

#include "entities/StaticEntity.h"

namespace sf
{
    //! A static obstacle loaded from a file or taking one of the simple geometrical shapes.
    class Obstacle : public StaticEntity
    {
    public:
        //! A constructor building an obstacle based on mesh files.
        /*!
         \param uniqueName a name for the body
         \param graphicsFilename a path to the 3d model used for rendering
         \param graphicsScale a scale factor to be used when reading the mesh file
         \param graphicsOrigin a pose of the mesh with respect to the body origin frame
         \param physicsFilename a path to the 3d model used for physics computation (collisions, fluid dynamics)
         \param physicsScale a scale factor to be used when reading the mesh file
         \param physicsOrigin a pose of the mesh with respect to the body origin frame
         \param m a material of which the body is made
         \param lookId a graphical material defining how the mesh is rendered
         \param smoothGraphicsNormals defines if the graphics mesh normals should be smoothed after loading
         */
        Obstacle(std::string uniqueName,
                 std::string graphicsFilename, Scalar graphicsScale, const Transform& graphicsOrigin,
                 std::string physicsFilename, Scalar physicsScale, const Transform& physicsOrigin,
                 Material m, int lookId = -1, bool smoothGraphicsNormals = false);
        
        //! A constructor building an obstacle based on a mesh file.
        /*!
         \param uniqueName a name for the body
         \param modelFilename a path to the 3d model used for both physics and rendering
         \param scale a scale factor to be used when reading the mesh file
         \param origin a pose of the mesh with respect to the body origin frame
         \param m a material of which the body is made
         \param lookId a graphical material defining how the mesh is rendered
         \param smoothNormals defines if the model normals should be smoothed after loading
         */
        Obstacle(std::string uniqueName, std::string modelFilename, Scalar scale, const Transform& origin, Material m, int lookId = -1, bool smoothNormals = false);
        
        //! A constructor building a spherical obstacle.
        /*!
         \param uniqueName a name for the body
         \param sphereRadius a radius of the spherical body
         \param m a material of the body
         \param lookId a graphical material defining how the body is rendered
         */
        Obstacle(std::string uniqueName, Scalar sphereRadius, Material m, int lookId = -1);
    
        //! A constructor building a box obstacle.
        /*!
         \param uniqueName a name for the body
         \param boxDimensions a vector of box dimensions (side lengths)
         \param m a material of the body
         \param lookId a graphical material defining how the body is rendered
         \param uvMode texture coordinates generation mode (0 - texture cross, 1 - same texture on all faces)
         */
        Obstacle(std::string uniqueName, Vector3 boxDimensions, Material m, int lookId = -1, unsigned int uvMode = 0);
        
        //! A constructor building a cylindrical obstacle.
        /*!
         \param uniqueName a name for the body
         \param cylinderRadius a radius of the cylindrical body
         \param cylinderHeight a height of the cylindrical body
         \param m a material of the body
         \param lookId a graphical material defining how the body is rendered
         */
        Obstacle(std::string uniqueName, Scalar cylinderRadius, Scalar cylinderHeight, Material m, int lookId = -1);
        
        //! A destructor.
        ~Obstacle();
        
        //! A method that returns the static body type.
        StaticEntityType getStaticType();
        
    private:
        void BuildGraphicalObject();
        Mesh* graMesh;
    };
}

#endif
