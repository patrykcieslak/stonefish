//
//  PlaneEntity.h
//  Stonefish
//
//  Created by Patryk Cieslak on 1/30/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_PlaneEntity__
#define __Stonefish_PlaneEntity__

#include "Entity.h"
#include "MaterialManager.h"
#include "OpenGLMaterial.h"

class PlaneEntity : public Entity
{
public:
    PlaneEntity(std::string uniqueName, btScalar size, Material* mat, Look l, const btTransform& worldTransform);
    ~PlaneEntity();
    
    EntityType getType();
    void Render();
    btTransform getTransform();
    void setTransform(const btTransform& trans);
    void AddToDynamicsWorld(btDynamicsWorld* world);
    
    void SetLook(Look newLook);
    btRigidBody* getRigidBody();
    
private:
    Material* material;
    btRigidBody* rigidBody;
    
    Look look;
    GLint displayList;
};

#endif
