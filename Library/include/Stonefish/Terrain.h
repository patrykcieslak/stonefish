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
#include "StaticEntity.h"
#include "MaterialManager.h"
#include "OpenGLContent.h"

class Terrain : public StaticEntity
{
public:
    Terrain(std::string uniqueName, int width, int length, btScalar size, btScalar minHeight, btScalar maxHeight, btScalar roughness, Material m, const btTransform& worldTransform, int lookId = -1);
    ~Terrain();
    
    StaticEntityType getStaticType();
    
private:
    btScalar* terrainHeight;
};

#endif
