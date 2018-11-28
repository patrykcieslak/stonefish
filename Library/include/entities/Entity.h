//
//  Entity.h
//  Stonefish
//
//  Created by Patryk Cieslak on 11/28/12.
//  Copyright (c) 2012 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Entity__
#define __Stonefish_Entity__

#define BIT(x) (1<<(x))

#include "StonefishCommon.h"

namespace sf
{
    
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

struct Renderable;
    
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
    virtual void getAABB(Vector3& min, Vector3& max) = 0;
    
protected:
    static Vector3 findInertiaAxis(Matrix3 I, Scalar value);
    
private:
    bool renderable;
    std::string name;
};
    
}

#endif
