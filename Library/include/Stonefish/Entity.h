//
//  Entity.h
//  Stonefish
//
//  Created by Patryk Cieslak on 11/28/12.
//  Copyright (c) 2012 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Entity__
#define __Stonefish_Entity__

#include "UnitSystem.h"
#include "NameManager.h"
#include "OpenGLContent.h"

#define BIT(x) (1<<(x))

typedef enum
{
    ENTITY_STATIC, ENTITY_SOLID, ENTITY_CABLE, ENTITY_FEATHERSTONE, ENTITY_FORCEFIELD, ENTITY_SYSTEM
}
EntityType;

typedef enum
{
    MASK_NONCOLLIDING = 0,
    MASK_STATIC = BIT(0),
    MASK_DEFAULT = BIT(1),
    MASK_CABLE_EVEN = BIT(2),
    MASK_CABLE_ODD = BIT(3)
}
CollisionMask;

//abstract class
class Entity
{
public:
	Entity(std::string uniqueName);
	virtual ~Entity();
    
    void setRenderable(bool render);
    bool isRenderable();
    std::string getName();
    
   	virtual EntityType getType() = 0;
    virtual std::vector<Renderable> Render() = 0;
    virtual void AddToDynamicsWorld(btMultiBodyDynamicsWorld* world) = 0;
    virtual void GetAABB(btVector3& min, btVector3& max) = 0;
    
protected:
    static btVector3 findInertiaAxis(btMatrix3x3 I, btScalar value);
    
private:
    bool renderable;
    std::string name;
    
    static NameManager nameManager;
};

#endif
