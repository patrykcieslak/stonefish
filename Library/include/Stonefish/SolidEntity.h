//
//  SolidEntity.h
//  Stonefish
//
//  Created by Patryk Cieslak on 12/29/12.
//  Copyright(c) 2012-2017 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_SolidEntity__
#define __Stonefish_SolidEntity__

#include <BulletDynamics/Featherstone/btMultiBodyLinkCollider.h>
#include "Entity.h"
#include "MaterialManager.h"
#include "OpenGLMaterial.h"
#include "GeometryUtil.h"
#include "FluidEntity.h"

typedef enum {SOLID_MESH = 0, SOLID_SPHERE, SOLID_CYLINDER, SOLID_BOX, SOLID_TORUS, SOLID_COMPOUND} SolidEntityType;

//pure virtual class
class SolidEntity : public Entity
{
    friend class FeatherstoneEntity;
    
public:
    SolidEntity(std::string uniqueName, Material* mat);
    virtual ~SolidEntity();
    
    EntityType getType();
    void GetAABB(btVector3& min, btVector3& max);
    void AddToDynamicsWorld(btMultiBodyDynamicsWorld* world);
    void AddToDynamicsWorld(btMultiBodyDynamicsWorld* world, const btTransform& worldTransform);
    void RemoveFromDynamicsWorld(btMultiBodyDynamicsWorld* world);

    void UpdateAcceleration();
    void ApplyGravity();
    void ApplyCentralForce(const btVector3& force);
    void ApplyTorque(const btVector3& torque);
    void SetHydrodynamicProperties(btVector3 dragCoefficients, btVector3 addedMass, btVector3 addedInertia);
    void Render();

    virtual btCollisionShape* BuildCollisionShape() = 0;
    virtual void SetLook(Look newLook);
    virtual void SetArbitraryPhysicalProperties(btScalar mass, const btVector3& inertia, const btTransform& cogTransform);
    
    virtual SolidEntityType getSolidType() = 0;
    virtual void ApplyFluidForces(FluidEntity* fluid);
    virtual void CalculateFluidDynamics(const btVector3& surfaceN, const btVector3&surfaceD, const btVector3&fluidV, const Fluid* fluid,
                                        btScalar& submergedVolume, btVector3& cob,  btVector3& drag, btVector3& angularDrag,
                                        btTransform* worldTransform = NULL, const btVector3& velocity = btVector3(0,0,0),
                                        const btVector3& angularVelocity = btVector3(0,0,0)) = 0;
    
    void setDisplayCoordSys(bool enabled);
    btRigidBody* getRigidBody();
    btMultiBodyLinkCollider* getMultibodyLinkCollider();
    btVector3 getMomentsOfInertia();
    btScalar getMass();
    Material* getMaterial();
    btScalar getVolume();
    btVector3 getDragCoefficients();
    Look getLook();
    GLint getDisplayList();

    btTransform getTransform();
    btTransform getLocalTransform();
    btVector3 getLinearVelocity();
    btVector3 getLinearVelocityInLocalPoint(const btVector3& relPos);
    btVector3 getAngularVelocity();
    btVector3 getLinearAcceleration();
    btVector3 getAngularAcceleration();
    
    bool isCoordSysVisible();
    bool fullyImmersed;
    
protected:
    virtual void BuildRigidBody();
    virtual void BuildDisplayList();
    virtual void BuildCollisionList() = 0;
    void BuildMultibodyLinkCollider(btMultiBody* mb, unsigned int child, btMultiBodyDynamicsWorld* world);
    
    btRigidBody* rigidBody;
    btMultiBodyLinkCollider* multibodyCollider;
    
    //Properties
    Material* material;
    btScalar mass;
    btVector3 Ipri;  //Principal moments of inertia
    btScalar thickness;
    btScalar volume;
    btVector3 centerOfBuoyancy;
    btVector3 dragCoeff;
    btVector3 addedMass;
    btVector3 addedInertia;
    btTransform localTransform;
    
    //Motion
    btVector3 linearAcc;
    btVector3 angularAcc;
    
    //Display
    Look look;
    GLint displayList;
    GLint collisionList;
    TriangleMesh *mesh;
    bool dispCoordSys;
};

#endif
