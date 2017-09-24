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
    Manipulator(std::string uniqueName, unsigned int numOfLinks, SolidEntity* baseLink); //Fixed base maniupulator
	Manipulator(std::string uniqueName, unsigned int numOfLinks, SolidEntity* baseLink, FeatherstoneEntity* attachment); //Attached to base of multibody
    virtual ~Manipulator();
    
	//Manipulator
	void AddRotLinkDH(SolidEntity* link, const btTransform& geomToJoint, btScalar d, btScalar a, btScalar alpha);
	//void AddTransLinkDH(SolidEntity* link, btScalar theta, btScalar a, btScalar alpha); //There should be two solids changing distance
	btScalar getJointPosition(unsigned int jointId);
	btScalar getDesiredJointPosition(unsigned int jointId);
	void setDesiredJointPosition(unsigned int jointId, btScalar position);
	
	//System
	void AddToDynamicsWorld(btMultiBodyDynamicsWorld* world, const btTransform& worldTransform);
	void GetAABB(btVector3& min, btVector3& max);
    void UpdateAcceleration(btScalar dt);
    void UpdateSensors(btScalar dt);
    void UpdateControllers(btScalar dt);
    void UpdateActuators(btScalar dt);
    void ApplyGravity(const btVector3& g);
	void ApplyDamping();
    btTransform getTransform() const;
    virtual std::vector<Renderable> Render();
    
private:
    FeatherstoneEntity* chain;
	std::vector<Motor*> motors;
	std::vector<RotaryEncoder*> encoders;
	std::vector<ServoController*> controllers;
    Gripper* gripper;
	unsigned int nLinks;
	unsigned int nTotalLinks;
	
	btTransform lastDHTrans; //Temp used while creating the structure
	FeatherstoneEntity* attach;
};

#endif
