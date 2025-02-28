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
//  Terrain.h
//  Stonefish
//
//  Created by Patryk Cieslak on 1/9/13.
//  Copyright (c) 2013-2019 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Terrain__
#define __Stonefish_Terrain__

#include "BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h"
#include "entities/StaticEntity.h"

namespace sf
{
    //! A class representing a heightfield terrain.
    class Terrain : public StaticEntity
    {
    public:
        //! A constructor.
        /*!
         \param uniqueName a name for the terrain
         \param pathToHeightmap a path to the file containing the heightmap
         \param scaleX the scale in the X direction [m/pix]
         \param scaleY the scale in the Y direction [m/pix]
         \param height the height at the maximum possible heightmap value [m]
         \param material the name of the material the terrain is made of
         \param look the name of the graphical material used for rendering
         \param uvScale scaling of texture coordinates
         */
        Terrain(std::string uniqueName, std::string pathToHeightmap, Scalar scaleX, Scalar scaleY, Scalar height, std::string material, std::string look = "", float uvScale = 1.f);
        
        //! A destructor.
        ~Terrain();
        
        //! A method used to add the terrain to the simulation.
        /*!
         \param sm a pointer to the simulation manager
         \param origin the origin of the terrain in the world frame
         */
        virtual void AddToSimulation(SimulationManager* sm, const Transform& origin);
        
        //! A method returning the extents of the terrain axis alligned bounding box.
        /*!
         \param min a point located at the minimum coordinate corner
         \param max a point located at the maximum coordinate corner
         */
        void getAABB(Vector3 &min, Vector3 &max);
        
        //! A method returning the type of static entity.
        StaticEntityType getStaticType();
        
    private:
        Scalar* heightfield;
        Scalar maxHeight;
    };
}

#endif
