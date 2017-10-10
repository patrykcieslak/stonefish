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
#include "Ocean.h"
#include "MaterialManager.h"
#include "OpenGLContent.h"

typedef enum {SOLID_POLYHEDRON = 0, SOLID_SPHERE, SOLID_CYLINDER, SOLID_BOX, SOLID_TORUS, SOLID_COMPOUND} SolidEntityType;

//pure virtual class
class SolidEntity : public Entity
{
    friend class FeatherstoneEntity;
    
public:
    SolidEntity(std::string uniqueName, Material m, int lookId = -1);
    virtual ~SolidEntity();
    
    EntityType getType();
    virtual SolidEntityType getSolidType() = 0;
    
	void AddToDynamicsWorld(btMultiBodyDynamicsWorld* world);
    void AddToDynamicsWorld(btMultiBodyDynamicsWorld* world, const btTransform& worldTransform);
    void RemoveFromDynamicsWorld(btMultiBodyDynamicsWorld* world);
	virtual btCollisionShape* BuildCollisionShape() = 0;
    virtual void BuildGraphicalObject();
	
	//Computation
    void UpdateAcceleration();
    virtual void ComputeFluidForces(const HydrodynamicsSettings& settings, const Ocean* fluid, const btTransform& cogTransform, const btTransform& geometryTransform, 
									const btVector3& linearV, const btVector3& angularV, const btVector3& linearA, const btVector3& angularA, btVector3& Fb, btVector3& Tb, btVector3& Fd, btVector3& Td, btVector3& Fa, btVector3& Ta);
    virtual void ComputeFluidForces(const HydrodynamicsSettings& settings, const Ocean* fluid, btVector3& Fb, btVector3& Tb, btVector3& Fd, btVector3& Td, btVector3& Fa, btVector3& Ta);
    
	//Applying forces
	void ApplyCentralForce(const btVector3& force);
    void ApplyTorque(const btVector3& torque);
    void ApplyGravity();
	virtual void ApplyFluidForces(const HydrodynamicsSettings& settings, const Ocean* fluid);
	
	//Rigid body
	void SetArbitraryPhysicalProperties(btScalar mass, const btVector3& inertia, const btTransform& cogTransform);
	void SetHydrodynamicProperties(const eigMatrix6x6& addedMass, const eigMatrix6x6& damping, const btTransform& cobTransform);
	void setComputeHydrodynamics(bool flag);
	
	btRigidBody* getRigidBody();
    btMultiBodyLinkCollider* getMultibodyLinkCollider();
    btVector3 getMomentsOfInertia();
    btScalar getMass();
    Material getMaterial();
    btScalar getVolume();
    virtual std::vector<Vertex>* getMeshVertices(); //Copy of vertices, must be deleted manually!!!
	
    btTransform getTransform() const;
    btTransform getLocalTransform();
    btVector3 getLinearVelocity();
    btVector3 getLinearVelocityInLocalPoint(const btVector3& relPos);
    btVector3 getAngularVelocity();
    btVector3 getLinearAcceleration();
    btVector3 getAngularAcceleration();
    
	//Rendering
	virtual std::vector<Renderable> Render();
	void GetAABB(btVector3& min, btVector3& max);
	void SetLook(int newLookId);
	void setDisplayCoordSys(bool enabled);
	int getLook();
	int getObject();
    bool isCoordSysVisible();
	
protected:
    void ComputeEquivEllipsoid();
    virtual void BuildRigidBody();
    void BuildMultibodyLinkCollider(btMultiBody* mb, unsigned int child, btMultiBodyDynamicsWorld* world);
    
	//Rigid body
    btRigidBody* rigidBody;
    btMultiBodyLinkCollider* multibodyCollider;
    
    Mesh *mesh;
    Material mat;
    btScalar thickness;
    btScalar volume;
	
	btScalar mass;  //Mass of solid
	btVector3 Ipri; //Principal moments of inertia
    btTransform localTransform; //Transform between graphical center and calculated mass center
	
    eigMatrix6x6 aMass; //Hydrodynamic added mass matrix
	eigMatrix6x6 dCS; //Hydrodynamic damping coefficients multiplied by cross sections
    btVector3 CoB; //Center of Buoyancy (in CoG frame)
    btVector3 ellipsoidR; //Radii of hydrodynamic proxy ellipsoid
    btTransform ellipsoidTransform; //Transform of the ellipsoid
	bool computeHydro;
	
    //Motion
    btVector3 linearAcc;
    btVector3 angularAcc;
    
    //Display
    int lookId;
	int objectId;
    bool dispCoordSys;
};

#endif
