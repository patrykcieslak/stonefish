//
//  Entity.h
//  Stonefish
//
//  Created by Patryk Cieslak on 11/28/12.
//  Copyright (c) 2012 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Entity__
#define __Stonefish_Entity__

#include "common.h"

#define BIT(x) (1<<(x))

typedef enum
{
    SOLID, PLANE, TERRAIN, CABLE, GHOST
}
EntityType;

typedef enum
{
    NONCOLLIDING = 0,
    STATIC = BIT(0),
    DEFAULT = BIT(1),
    CABLE_EVEN = BIT(2),
    CABLE_ODD = BIT(3)
}
CollisionMask;

//pure virtual class
class Entity
{
public:
	Entity(std::string uniqueName);
	virtual ~Entity();
    
    void setRenderable(bool render);
    bool isRenderable();
    std::string getName();
    
   	virtual EntityType getType() = 0;
    virtual void Render() = 0;
    virtual btTransform getTransform() = 0;
    virtual void setTransform(const btTransform& trans) = 0;
    virtual void AddToDynamicsWorld(btDynamicsWorld* world) = 0;
    //virtual void RemoveFromDynamicsWorld(btDynamicsWorld* world) = 0;
    
private:
    bool renderable;
    std::string name;
};

#endif
