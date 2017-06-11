//
//  Manipulator.h
//  Stonefish
//
//  Created by Patryk Cieslak on 17/05/17.
//  Copyright(c) 2017 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Manipulator__
#define __Stonefish_Manipulator__

#include "SystemEntity.h"
#include "FeatherstoneEntity.h"
#include "Gripper.h"
#include "Motor.h"
#include "RotaryEncoder.h"
#include "ServoController.h"
#include "FixedJoint.h"

class Manipulator : public SystemEntity
{
public:
    Manipulator(std::string uniqueName, unsigned int numOfLinks, SolidEntity* baseLink, const btTransform& worldTrans); //Fixed base maniupulator
	Manipulator(std::string uniqueName, unsigned int numOfLinks, SolidEntity* baseLink, const btTransform& worldTrans, btRigidBody* attachment); //Attached to a rigid body
    virtual ~Manipulator();
    
	//Manipulator
	void AddRotLinkDH(SolidEntity* link, const btTransform& geomToJoint, btScalar d, btScalar a, btScalar alpha);
	//void AddTransLinkDH(SolidEntity* link, btScalar theta, btScalar a, btScalar alpha); //There should be two solids changing distance
	
	//System
	void AddToDynamicsWorld(btMultiBodyDynamicsWorld* world, const btTransform& worldTransform);
	void GetAABB(btVector3& min, btVector3& max);
    void UpdateAcceleration();
    void UpdateSensors(btScalar dt);
    void UpdateControllers(btScalar dt);
    void UpdateActuators(btScalar dt);
    void ApplyGravity(const btVector3& g);
    void ApplyFluidForces(Ocean* fluid);
    btTransform getTransform() const;
    virtual void Render();
    
private:
    FeatherstoneEntity* chain;
	std::vector<Motor*> motors;
	std::vector<RotaryEncoder*> encoders;
	std::vector<ServoController*> controllers;
    Gripper* gripper;
	unsigned int nLinks;
	unsigned int nTotalLinks;
	
	btTransform lastDHTrans; //Temp used while creating the structure
	btRigidBody* attach;
};

#endif
