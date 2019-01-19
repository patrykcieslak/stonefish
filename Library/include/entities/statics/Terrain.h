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
        Terrain(std::string uniqueName, std::string pathToHeightmap, Scalar scaleX, Scalar scaleY, Scalar height, Material m, int lookId = -1);
        ~Terrain();
        
        virtual void AddToSimulation(SimulationManager* sm, const Transform& origin);
        
        void getAABB(Vector3 &min, Vector3 &max);
        StaticEntityType getStaticType();
        
    private:
        Scalar* terrainHeight;
        Scalar maxHeight;
    };
}

#endif
