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

class UnderwaterVehicle : public SystemEntity
{
public:
    UnderwaterVehicle(std::string uniqueName, SolidEntity* bodySolid, const btTransform& worldTrans);
    virtual ~UnderwaterVehicle();
    
    virtual void AddToDynamicsWorld(btMultiBodyDynamicsWorld* world, const btTransform& worldTransform);
	virtual void GetAABB(btVector3& min, btVector3& max);
    virtual void UpdateAcceleration(btScalar dt);
    virtual void UpdateSensors(btScalar dt);
    virtual void UpdateControllers(btScalar dt);
    virtual void UpdateActuators(btScalar dt);
    virtual void ApplyGravity(const btVector3& g);
	virtual void ApplyDamping();
    
    virtual btTransform getTransform() const;
	FeatherstoneEntity* getVehicleBody();
    
    virtual std::vector<Renderable> Render();
    
private:
    //Subsystems
	std::vector<SimpleSensor*> sensors;
    std::vector<Thruster*> thrusters; // + FINS?
    std::vector<Manipulator*> manipulators;
    
	//Vehicle body
    FeatherstoneEntity* vehicleBody;
	btScalar vehicleBodyMass;     //Mass of vehicle body (internal + external parts)
    btVector3 vehicleBodyInertia; //Inertia of vehicle body (internal + external parts)
	btTransform localTransform; //...of vehicle body (CoG)
    btMatrixXu aMass;
	btMatrixXu dCoeff;
	btVector3 CoB;
	
	//Motion
	btVector3 lastLinearVel;
	btVector3 lastAngularVel;
    btVector3 linearAcc;
    btVector3 angularAcc;
    
	//Rendering
    bool showInternals;
};

#endif
