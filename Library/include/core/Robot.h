//
//  Robot.h
//  Stonefish
//
//  Created by Patryk Cieslak on 5/11/2018.
//  Copyright(c) 2018 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Robot__
#define __Stonefish_Robot__

#include <utility>
#include "core/UnitSystem.h"
#include "core/NameManager.h"
#include "graphics/OpenGLPipeline.h"
#include "entities/FeatherstoneEntity.h"
#include "actuators/LinkActuator.h"
#include "actuators/JointActuator.h"
#include "sensors/scalar/LinkSensor.h"
#include "sensors/scalar/JointSensor.h"
#include "sensors/VisionSensor.h"

namespace sf
{

class SimulationManager;

//! A class inplementing a robotic system composed of a dynamic rigid-body tree, actuators and sensors.
class Robot
{
public:
	//! A constructor
    Robot(std::string uniqueName, bool fixedBase = false);
	//! A destructor
    virtual ~Robot();
    
    //DYNAMICS
	//! A method used to define a list of rigid bodies constituting the mechanical part of the robot (dynamic tree)
	void DefineLinks(SolidEntity* baseLink, std::vector<SolidEntity*> otherLinks); 
	//! A method used to define a revolute joint between two mechanical parts of the robot
	void DefineRevoluteJoint(std::string jointName, std::string parentName, std::string childName, const btTransform& T, const btVector3& axis, std::pair<btScalar, btScalar> positionLimits, btScalar torqueLimit);
	//! A method used to define a prismatic joint between two mechanical parts of the robot
	void DefinePrismaticJoint(std::string jointName, std::string parentName, std::string childName, const btTransform& T, const btVector3& axis, std::pair<btScalar, btScalar> positionLimits, btScalar forceLimit);
	//! A method used to define a fixed joint between two mechanical parts of the robot
	void DefineFixedJoint(std::string jointName, std::string parentName, std::string childName, const btTransform& T);
	
    //PERCEPTION
    //!
    void AddLinkSensor(LinkSensor* s, const std::string& monitoredLinkName, const btTransform& origin);
    //!
    void AddJointSensor(JointSensor* s, const std::string& monitoredJointName);
    //!
    void AddVisionSensor(VisionSensor* s, const std::string& attachmentLinkName, const btTransform& origin);
    
    //ACTUATORS
    //!
	void AddLinkActuator(LinkActuator* a, const std::string& actuatedLinkName, const btTransform& origin);
	//!
	void AddJointActuator(JointActuator* a, const std::string& actuatedJointName);
	
    //GENERAL
    //! A method checking the consistency of the defined robotic system and adding it to the simulation world
    void AddToSimulation(SimulationManager* sm, const btTransform& worldTransform);
	//! Get world transform of robot base 
	virtual btTransform getTransform() const;
	//! Get name of the robot
	std::string getName();
  
private:
	void getFreeLinkPair(const std::string& parentName, const std::string& childName, unsigned int& parentId, unsigned int& childId);
    SolidEntity* getLink(const std::string& name);
    int getJoint(const std::string& name);
    
	FeatherstoneEntity* dynamics;
	bool fixed;
    std::vector<SolidEntity*> detachedLinks;
    std::vector<SolidEntity*> links;
    std::vector<Sensor*> sensors;
    std::vector<Actuator*> actuators;
    std::string name;
    
    static NameManager nameManager;
};

}
    
#endif
