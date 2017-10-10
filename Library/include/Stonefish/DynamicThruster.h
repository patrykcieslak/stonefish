//
//  DynamicThruster.h
//  Stonefish
//
//  Created by Patryk Cieslak on 16/09/17.
//  Copyright (c) 2017 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_DynamicThruster__
#define __Stonefish_DynamicThruster__

#include "SystemEntity.h"
#include "SolidEntity.h"
#include "FeatherstoneEntity.h"
#include "Motor.h"
#include "FakeRotaryEncoder.h"
#include "SpeedController.h"

class UnderwaterVehicle;

class DynamicThruster : public SystemEntity
{
public:
	//Thruster(std::string uniqueName, SolidEntity* solid, SolidEntity* duct, SolidEntity* propeller, const btTransform& location, btScalar propDiameter, btScalar thrustCoeff);
	DynamicThruster(std::string uniqueName, UnderwaterVehicle* vehicle, SolidEntity* duct, SolidEntity* propeller, btScalar propDiameter, btScalar thrustCoeff);
	~DynamicThruster();
	
	void SetDesiredSpeed(btScalar s);
	
	void AddToDynamicsWorld(btMultiBodyDynamicsWorld* world, const btTransform& worldTransform);
	void UpdateAcceleration(btScalar dt);
    void UpdateSensors(btScalar dt);
    void UpdateControllers(btScalar dt);
    void UpdateActuators(btScalar dt);
    void ApplyGravity(const btVector3& g);
	void ApplyDamping();
    
	btTransform getTransform() const;
    void GetAABB(btVector3& min, btVector3& max);
	std::vector<Renderable> Render();
	
private:
	//Components of system
	btTransform propLocation;
	UnderwaterVehicle* actuatedVehicle;
	FeatherstoneEntity* thruster;
	SolidEntity* ductSolid;
    SolidEntity* propSolid;
	RevoluteJoint* propRev;
	Motor* motor;
	FakeRotaryEncoder* enc;
	SpeedController* ctrl;
	
	//Parameters of hydrodynamic model
	btScalar D;
	btScalar KT;
	btScalar KQ;
};

#endif