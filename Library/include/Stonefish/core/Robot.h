//
//  Robot.h
//  Stonefish
//
//  Created by Patryk Cieslak on 5/11/2018.
//  Copyright(c) 2018 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Robot__
#define __Stonefish_Robot__

#include <core/UnitSystem.h>
#include <core/NameManager.h>
#include <graphics/OpenGLPipeline.h>
#include <entities/FeatherstoneEntity.h>
#include <actuators/Actuator.h>
#include <sensors/Sensor.h>
#include <utility>

//! A class inplementing a robotic system composed of a dynamic rigid-body tree, actuators and sensors.
class Robot
{
public:
	//! A constructor
    Robot(std::string uniqueName, bool fixedBase = false);
	//! A destructor
    virtual ~Robot();
    
	//! A method used to define a list of rigid bodies constituting the mechanical part of the robot (dynamic tree)
	void DefineLinks(SolidEntity* baseLink, std::vector<SolidEntity*> otherLinks); 
	//! A method used to define a revolute joint between two mechanical parts of the robot
	void DefineRevoluteJoint(std::string jointName, std::string parentName, std::string childName, const btTransform& T, const btVector3& axis, std::pair<btScalar, btScalar> positionLimits, btScalar torqueLimit);
	//! A method used to define a prismatic joint between two mechanical parts of the robot
	void DefinePrismaticJoint(std::string jointName, std::string parentName, std::string childName, const btTransform& T, const btVector3& axis, std::pair<btScalar, btScalar> positionLimits, btScalar forceLimit);
	//! A method used to define a fixed joint between two mechanical parts of the robot
	void DefineFixedJoint(std::string jointName, std::string parentName, std::string childName, const btTransform& T);
	//! A method checking the consistency of the defined robotic system and adding it to the simulation world
    void AddToDynamicsWorld(btMultiBodyDynamicsWorld* world, const btTransform& worldTransform);
	//!
	void AddLinkActuator(Actuator* act, const std::string& actuatedLinkName);
	//! 
	void AddLinkSensor(Sensor* sens, const std::string& monitoredLinkName);
	//! 
	void AddJointActuator(Actuator* act, const std::string& actuatedJointName);
	//!
	void AddJointSensor(Sensor* sens, const std::string& monitoredJointName);
	
	//! Get world transform of robot base 
	virtual btTransform getTransform() const;
	//! Get name of the robot
	std::string getName();
    
	//! Get the graphical bounding box of the robot
	virtual void GetAABB(btVector3& min, btVector3& max);
	//! Render the robot
    virtual std::vector<Renderable> Render();
	//! Configure if robot should be renderable
	void setRenderable(bool render);
	//! Check if robot is renderable
    bool isRenderable();
    
private:
	void getLinkPair(const std::string& parentName, const std::string& childName, unsigned int& parentId, unsigned int& childId);

	FeatherstoneEntity* dynamics;
	std::vector<SolidEntity*> links;
	bool fixed;
	bool renderable;
    std::string name;
	
    static NameManager nameManager;
};

#endif
