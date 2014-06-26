//
//  FeatherstoneEntity.h
//  Stonefish
//
//  Created by Patryk Cieslak on 8/20/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_FeatherstoneEntity__
#define __Stonefish_FeatherstoneEntity__

#include <BulletDynamics/Featherstone/btMultiBody.h>
#include <BulletDynamics/Featherstone/btMultiBodyLinkCollider.h>
#include <BulletDynamics/Featherstone/btMultiBodyLink.h>
#include <BulletDynamics/Featherstone/btMultiBodyJointLimitConstraint.h>
#include <BulletDynamics/Featherstone/btMultiBodyJointMotor.h>
#include "Entity.h"
#include "SolidEntity.h"
#include "StaticEntity.h"

struct FeatherstoneLink
{
    SolidEntity* solid;
    btTransform transform;
    
    FeatherstoneLink(SolidEntity* s, const btTransform& t)
    {
        solid = s;
        transform = t;
    }
};

/*! Featherstone multi-body */
class FeatherstoneEntity : public Entity
{
public:
    FeatherstoneEntity(std::string uniqueName, unsigned int totalNumOfLinks, SolidEntity* baseSolid, const btTransform& transform, btMultiBodyDynamicsWorld* world, bool fixedBase = false);
    virtual ~FeatherstoneEntity();
    
    void AddLink(SolidEntity* solid, const btTransform& transform, btMultiBodyDynamicsWorld* world);
    void AddRevoluteJoint(unsigned int parent, unsigned int child, const btVector3& pivot, const btVector3& axis, bool collide = false);
    void AddPrismaticJoint(unsigned int parent, unsigned int child, const btVector3& pivot, const btVector3& axis, bool collide = false);
    void AddCollider(SolidEntity* solid);
    void AddCollider(StaticEntity* stat);
    
    void DriveJoint(unsigned int child, btScalar forceTorque);
    
    void setJointIC(unsigned int child, btScalar position, btScalar velocity);
    void getJointPosition(unsigned int child, btScalar& position, btMultibodyLink::eFeatherstoneJointType& jointType);
    void getJointVelocity(unsigned int child, btScalar& velocity, btMultibodyLink::eFeatherstoneJointType& jointType);
    void setBaseRenderable(bool render);
    
    void AddToDynamicsWorld(btMultiBodyDynamicsWorld* world);
    void Render();
    void RenderStructure();
    EntityType getType();
    void GetAABB(btVector3& min, btVector3& max);
    
private:
    btMultiBody* multiBody;
    std::vector<FeatherstoneLink> links;
    bool baseRenderable;
};

#endif