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
#include <core/MaterialManager.h>
#include <graphics/OpenGLContent.h>

typedef enum {STATIC_PLANE, STATIC_TERRAIN, STATIC_OBSTACLE} StaticEntityType;

//abstract class
class StaticEntity : public Entity
{
public:
    StaticEntity(std::string uniqueName, Material m, int lookId = -1);
    ~StaticEntity();
    
    std::vector<Renderable> Render();
    void SetLook(int newLookId);
    void SetWireframe(bool enabled);
    
    void AddToDynamicsWorld(btMultiBodyDynamicsWorld* world);
    void AddToDynamicsWorld(btMultiBodyDynamicsWorld* world, const btTransform& worldTransform);
    virtual void GetAABB(btVector3& min, btVector3& max);
    
    void setTransform(const btTransform& trans);
    btTransform getTransform();
    Material getMaterial();
    btRigidBody* getRigidBody();
    EntityType getType();
    
    virtual StaticEntityType getStaticType() = 0;
    
    static void GroupTransform(std::vector<StaticEntity*>& objects, const btTransform& centre, const btTransform& transform);
    
protected:
    void BuildRigidBody(btCollisionShape* shape);
	void BuildGraphicalObject();
	
    btRigidBody* rigidBody;
	Material mat;
    Mesh* mesh;
    
    int objectId;
    int lookId;
    bool wireframe;
};

#endif
