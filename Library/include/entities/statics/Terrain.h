//
//  Terrain.h
//  Stonefish
//
//  Created by Patryk Cieslak on 1/9/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Terrain__
#define __Stonefish_Terrain__

#include <BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h>
#include "entities/StaticEntity.h"

namespace sf
{

class Terrain : public StaticEntity
{
public:
    Terrain(std::string uniqueName, unsigned int width, unsigned int length, Scalar size, Scalar minHeight, Scalar maxHeight, Scalar roughness, Material m, int lookId = -1);
   
    StaticEntityType getStaticType();
    
private:
    Scalar* terrainHeight;
};

}

#endif
