//
//  StaticEntity.h
//  Stonefish
//
//  Created by Patryk Cieslak on 24/05/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_StaticEntity__
#define __Stonefish_StaticEntity__

#include "Entity.h"
#include "MaterialManager.h"
#include "OpenGLMaterial.h"

typedef enum {STATIC_PLANE, STATIC_TERRAIN, STATIC_OBSTACLE} StaticEntityType;

//abstract class
class StaticEntity : public Entity
{
public:
    StaticEntity(std::string uniqueName, Material* mat, Look l);
    ~StaticEntity();
    
    void Render();
    void SetLook(Look newLook);
    void SetWireframe(bool enabled);
    
    btTransform getTransform();
    void setTransform(const btTransform& trans);
    Material* getMaterial();
    void AddToDynamicsWorld(btDynamicsWorld* world);
    btRigidBody* getRigidBody();
    
    EntityType getType();
    void GetAABB(btVector3& min, btVector3& max);
    
    virtual StaticEntityType getStaticType() = 0;
    
protected:
    btRigidBody* rigidBody;
    GLint displayList;
    
private:
    Material* material;
    Look look;
    bool wireframe;
};

#endif
