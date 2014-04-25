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
#include "Entity.h"
#include "MaterialManager.h"
#include "OpenGLMaterial.h"

class TerrainEntity : public Entity
{
public:
    TerrainEntity(std::string uniqueName, int width, int length, btScalar size, btScalar minHeight, btScalar maxHeight, btScalar roughness, Material* mat, Look l, const btTransform& worldTransform);
    ~TerrainEntity();
    
    EntityType getType();
    void Render();
    btTransform getTransform();
    Material* getMaterial();
    void setTransform(const btTransform& trans);
    void AddToDynamicsWorld(btDynamicsWorld* world);
    
    void SetLook(Look newLook);
    void setDrawWireframe(bool wire);
    
private:
    btScalar* terrainHeight;
    btRigidBody* rigidBody;
    Material* material;
    
    Look look;
    GLint displayList;
    bool wireframe;
};

#endif
