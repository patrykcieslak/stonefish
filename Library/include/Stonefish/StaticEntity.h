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
#include "OpenGLContent.h"

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
    EntityType getType();
    virtual void GetAABB(btVector3& min, btVector3& max);
    
    Material getMaterial();
    btRigidBody* getRigidBody();
    
    virtual StaticEntityType getStaticType() = 0;
    
protected:
    btRigidBody* rigidBody;
	int objectId;
	Mesh* mesh;
    
private:
    Material mat;
    int lookId;
    bool wireframe;
};

#endif
