//
//  Terrain.h
//  Stonefish
//
//  Created by Patryk Cieslak on 1/9/13.
//  Copyright (c) 2013-2019 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Terrain__
#define __Stonefish_Terrain__

#include <BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h>
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
         \param m the material that the terrain is made of
         \param lookId the id of the material used to render the terrain
         */
        Terrain(std::string uniqueName, std::string pathToHeightmap, Scalar scaleX, Scalar scaleY, Scalar height, Material m, int lookId = -1);
        
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
        Scalar* terrainHeight;
        Scalar maxHeight;
    };
}

#endif
