//
//  CableEntity.h
//  Stonefish
//
//  Created by Patryk Cieslak on 12/29/12.
//  Copyright (c) 2012 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_CableEntity__
#define __Stonefish_CableEntity__

#include "Entity.h"
#include "MaterialManager.h"
#include "OpenGLMaterial.h"

class CableEntity : public Entity
{
public:
    CableEntity(std::string uniqueName, const btVector3& end1, const btVector3& end2, unsigned int parts, btScalar diam, btScalar stiffness, bool selfCollidable, Material* mat);
    CableEntity(std::string uniqueName, const btVector3& end1, const btVector3& end2, btVector3* midPoints, unsigned int midPointsCount, unsigned int parts, btScalar diam, btScalar stiffness, bool selfCollidable, Material* mat);
    virtual ~CableEntity();
    
    EntityType getType();
    void SetLook(Look newLook);
    void Render();
    void AddToDynamicsWorld(btMultiBodyDynamicsWorld* world);
    void ApplyGravity();
    void CalculateFluidDynamics(const btVector3& surfaceN, const btVector3&surfaceD, const btVector3&fluidV, const Fluid* fluid,
                                btScalar& submergedVolume, btVector3& cob,  btVector3& drag, btVector3& angularDrag,
                                const btTransform& worldTransform, const btVector3& velocity, const btVector3& angularVelocity);
    
    Material* getMaterial();
    btRigidBody* getFirstEnd();
    btRigidBody* getSecondEnd();
    btScalar getPartVolume();
    
private:
    std::vector<btRigidBody*> cableParts;
    std::vector<btTypedConstraint*> links;
    bool selfCollision;
    
    btScalar diameter;
    btScalar partLength;
    btScalar partVolume;
    
    Material* material;
    Look look;
};

#endif
