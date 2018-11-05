//
//  Terrain.h
//  Stonefish
//
//  Created by Patryk Cieslak on 1/9/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Terrain__
#define __Stonefish_Terrain__

#include <entities/StaticEntity.h>
#include <BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h>

class Terrain : public StaticEntity
{
public:
    Terrain(std::string uniqueName, unsigned int width, unsigned int length, btScalar size, btScalar minHeight, btScalar maxHeight, btScalar roughness, Material m, int lookId = -1);
   
    StaticEntityType getStaticType();
    
private:
    btScalar* terrainHeight;
};

#endif
