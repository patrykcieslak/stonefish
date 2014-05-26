//
//  TerrainEntity.h
//  Stonefish
//
//  Created by Patryk Cieslak on 1/9/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_TerrainEntity__
#define __Stonefish_TerrainEntity__

#include <BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h>
#include "StaticEntity.h"
#include "MaterialManager.h"
#include "OpenGLMaterial.h"

class TerrainEntity : public StaticEntity
{
public:
    TerrainEntity(std::string uniqueName, int width, int length, btScalar size, btScalar minHeight, btScalar maxHeight, btScalar roughness, Material* mat, Look l, const btTransform& worldTransform);
    ~TerrainEntity();
    
    StaticEntityType getStaticType();
    
private:
    btScalar* terrainHeight;
};

#endif
