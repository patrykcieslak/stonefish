//
//  SolidEntity.h
//  Stonefish
//
//  Created by Patryk Cieslak on 12/29/12.
//  Copyright (c) 2012 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_SolidEntity__
#define __Stonefish_SolidEntity__

#include "Entity.h"
#include "MaterialManager.h"
#include "OpenGLMaterial.h"
#include "GeometryUtil.h"

typedef enum {MESH = 0, SPHERE, CYLINDER, BOX, TORUS, COMPOUND} SolidEntityType;

//pure virtual class
class SolidEntity : public Entity
{
public:
    SolidEntity(std::string uniqueName, bool isStatic, Material* mat);
    virtual ~SolidEntity();
    
    EntityType getType();
    void AddToDynamicsWorld(btDynamicsWorld* world);
    void AddToDynamicsWorld(btDynamicsWorld* world, const btTransform& worldTransform);
    void RemoveFromDynamicsWorld(btDynamicsWorld* world);
    void ApplyGravity();
    void Render();
    void SetHydrodynamicProperties(btVector3 dragCoefficients, btVector3 addedMass, btVector3 addedInertia);
    
    virtual void SetLook(Look newLook);
    virtual void SetArbitraryPhysicalProperties(btScalar mass, const btVector3& inertia, const btVector3& centerOfGravity);
    
    virtual SolidEntityType getSolidType() = 0;
    virtual btCollisionShape* BuildCollisionShape() = 0;
    virtual void CalculateFluidDynamics(const btVector3& surfaceN, const btVector3&surfaceD, const btVector3&fluidV, const Fluid* fluid,
                                        btScalar& submergedVolume, btVector3& cob,  btVector3& drag, btVector3& angularDrag,
                                        btTransform* worldTransform = NULL, const btVector3& velocity = btVector3(0,0,0),
                                        const btVector3& angularVelocity = btVector3(0,0,0)) = 0;
    bool isStatic();
    void setDisplayCoordSys(bool enabled);
    void setTransform(const btTransform& trans);
    btTransform getTransform();
    btRigidBody* getRigidBody();
    btTransform getLocalTransform();
    btVector3 getMomentsOfInertia();
    btScalar getMass();
    Material* getMaterial();
    btScalar getVolume();
    btVector3 getDragCoefficients();
    Look getLook();
    GLint getDisplayList();
    
    bool fullyImmersed;
    
protected:
    virtual void BuildRigidBody();
    virtual void BuildDisplayList();
    virtual void BuildCollisionList() = 0;

    bool staticBody;
    Material* material;
    btRigidBody* rigidBody;
    btScalar mass;
    btVector3 Ipri;  //principal moments of inertia
    btTransform localTransform;
    
    btScalar volume;
    btVector3 centerOfBuoyancy;
    btVector3 dragCoeff;
    btVector3 addMass;
    btVector3 addInertia;
    
    Look look;
    GLint displayList;
    GLint collisionList;
    TriangleMesh *mesh;
    bool dispCoordSys;
};

#endif
