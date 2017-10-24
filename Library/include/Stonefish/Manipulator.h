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

//! A dynamical model of an electric manipulator
/*! 
 * This class implements a dynamical model of a rigid body manipulator, equipped with servo motors. 
 * The manipulator can be controlled in position or velocity. It has force feedback on all joints.
 * It can be equipped with a gripper (Gripper class). 
 * The model is constructed from solids, using the Denavit-Hartenberg notation.
 */
class Manipulator : public SystemEntity
{
public:
    Manipulator(std::string uniqueName, unsigned int numOfLinks, SolidEntity* baseLink, const btTransform& geomToJoint); //Fixed base maniupulator
	Manipulator(std::string uniqueName, unsigned int numOfLinks, SolidEntity* baseLink, const btTransform& geomToJoint, FeatherstoneEntity* attachment); //Attached to base of multibody
    virtual ~Manipulator();
    
	//Manipulator
	void AddRotLinkDH(SolidEntity* link, Motor* motor, const btTransform& geomToJoint, btScalar d, btScalar a, btScalar alpha, btScalar lowerLimit = btScalar(1.0), btScalar upperLimit = btScalar(-1.0));
    void AddTransformDH(btScalar d, btScalar a, btScalar alpha);
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
    
    SystemType getSystemType();
    btTransform getTransform() const;
    const std::vector<btTransform>& getDH();
    virtual std::vector<Renderable> Render();
    
private:
    FeatherstoneEntity* chain;
	std::vector<Motor*> motors;
	std::vector<RotaryEncoder*> encoders;
	std::vector<ServoController*> controllers;
    Gripper* gripper;
	unsigned int nLinks;
	unsigned int nTotalLinks;
	
	std::vector<btTransform> DH; //Succesive transforms from DH notation (multiplied) 
	FeatherstoneEntity* attach;
};

#endif
