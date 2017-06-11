//
//  UnderwaterVehicle.h
//  Stonefish
//
//  Created by Patryk Cieslak on 17/05/17.
//  Copyright(c) 2017 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_UnderwaterVehicle__
#define __Stonefish_UnderwaterVehicle__

#include "SystemEntity.h"
#include "SolidEntity.h"
#include "SimpleSensor.h"
#include "Thruster.h"
#include "Manipulator.h"

typedef struct
{
    SolidEntity* solid;
    btTransform position;
    bool isExternal;
} VehiclePart;

class UnderwaterVehicle : public SystemEntity
{
public:
    UnderwaterVehicle(std::string uniqueName);
    virtual ~UnderwaterVehicle();
    
    void AddInternalPart(SolidEntity* solid, const btTransform& position);
    void AddExternalPart(SolidEntity* solid, const btTransform& position);
    
    virtual void AddToDynamicsWorld(btMultiBodyDynamicsWorld* world, const btTransform& worldTransform);
	virtual void GetAABB(btVector3& min, btVector3& max);
    virtual void UpdateAcceleration();
    virtual void UpdateSensors(btScalar dt);
    virtual void UpdateControllers(btScalar dt);
    virtual void UpdateActuators(btScalar dt);
    virtual void ApplyGravity(const btVector3& g);
    virtual void ApplyFluidForces(Ocean* fluid);
    
    virtual btTransform getTransform() const;
    
    virtual void Render();
    void BuildGraphicalObjects();
    
private:
    std::vector<VehiclePart> bodyParts; //Parts of the vehicle body
    std::vector<SimpleSensor*> sensors;
    std::vector<Thruster*> thrusters; // + FINS?
    std::vector<Manipulator*> manipulators;
    
    btRigidBody* vehicleBody;
    btTransform localTransform; //...of vehicle body
    btScalar vehicleBodyMass;     //Mass of vehicle body (internal + external parts)
    btVector3 vehicleBodyInertia; //Inertia of vehicle body (internal + external parts)
    btVector3 linearAcc;
    btVector3 angularAcc;
    
    GLint internalList;
    GLint externalList;
    GLint collisionList;
    bool showInternals;
};

#endif
