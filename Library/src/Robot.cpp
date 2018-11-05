//
//  Robot.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 5/11/2018.
//  Copyright(c) 2018 Patryk Cieslak. All rights reserved.
//

#include <core/Robot.h>

#include <core/SimulationApp.h>
#include <graphics/Console.h>

NameManager Robot::nameManager;

Robot::Robot(std::string uniqueName, bool fixedBase)
{
	name = nameManager.AddName(uniqueName);
    renderable = true;
	dynamics = NULL;
	fixed = fixedBase;
}

Robot::~Robot()
{
	nameManager.RemoveName(name);
	if(dynamics != NULL)
		delete dynamics;
}

void Robot::getLinkPair(const std::string& parentName, const std::string& childName, unsigned int& parentId, unsigned int& childId)
{
	if(dynamics == NULL)
		cCritical("Robot links not defined!");
		
	if(links.size() == 0)
		cCritical("No more links allocated!");
	
	//Find parent ID
	parentId = UINT32_MAX;
	for(unsigned int i=0; i<dynamics->getNumOfLinks(); ++i)
		if(dynamics->getLink(i).solid->getName() == parentName)
			parentId = i;
	
	if(parentId >= dynamics->getNumOfLinks())
		cCritical("Parent link '%s' not yet joined with robot!", parentName.c_str());
	
	//Find child ID
	childId = UINT32_MAX;
	for(unsigned int i=0; i<links.size(); ++i)
		if(links[i]->getName() == childName)
			childId = i;
			
	if(childId >= links.size())
		cCritical("Child link '%s' doesn't exist!", childName.c_str());
}

btTransform Robot::getTransform() const
{
	if(dynamics != NULL)
		return dynamics->getLinkTransform(0);
	else
		return btTransform::getIdentity();
}

void Robot::DefineLinks(SolidEntity* baseLink, std::vector<SolidEntity*> otherLinks)
{
	if(dynamics != NULL)
		cCritical("Robot cannot be redefined!");
	
	links = otherLinks;
	dynamics = new FeatherstoneEntity(name + "_Dynamics", links.size() + 1, baseLink, fixed);
}

void Robot::DefineRevoluteJoint(std::string jointName, std::string parentName, std::string childName, const btTransform& T, const btVector3& axis, std::pair<btScalar,btScalar> positionLimits, btScalar torqueLimit)
{
	unsigned int parentId, childId;
	getLinkPair(parentName, childName, parentId, childId);
	
	//Add link to the dynamic tree
	btTransform linkTrans = dynamics->getLinkTransform(parentId) * T;
	dynamics->AddLink(links[childId], linkTrans);
	links.erase(links.begin()+childId);
	dynamics->AddRevoluteJoint(jointName, parentId, dynamics->getNumOfLinks()-1, linkTrans.getOrigin(), linkTrans.getBasis() * axis);
       
	if(positionLimits.first < positionLimits.second)
		dynamics->AddJointLimit(dynamics->getNumOfJoints()-1, positionLimits.first, positionLimits.second);
	
	dynamics->AddJointMotor(dynamics->getNumOfJoints()-1, torqueLimit/SimulationApp::getApp()->getSimulationManager()->getStepsPerSecond());
	//dynamics->setJointDamping(dynamics->getNumOfJoints()-1, 0, 0.5);
}

void Robot::DefinePrismaticJoint(std::string jointName, std::string parentName, std::string childName, const btTransform& T, const btVector3& axis, std::pair<btScalar,btScalar> positionLimits, btScalar forceLimit)
{
	unsigned int parentId, childId;
	getLinkPair(parentName, childName, parentId, childId);
	
	//Add link to the dynamic tree
	btTransform linkTrans = dynamics->getLinkTransform(parentId) * T;
	dynamics->AddLink(links[childId], linkTrans);
	links.erase(links.begin()+childId);
	dynamics->AddPrismaticJoint(jointName, parentId, dynamics->getNumOfLinks()-1, linkTrans.getBasis() * axis);
       
	if(positionLimits.first < positionLimits.second)
		dynamics->AddJointLimit(dynamics->getNumOfJoints()-1, positionLimits.first, positionLimits.second);
	
	dynamics->AddJointMotor(dynamics->getNumOfJoints()-1, forceLimit/SimulationApp::getApp()->getSimulationManager()->getStepsPerSecond());
	//dynamics->setJointDamping(dynamics->getNumOfJoints()-1, 0, 0.5);
}

void Robot::DefineFixedJoint(std::string jointName, std::string parentName, std::string childName, const btTransform& T)
{
	unsigned int parentId, childId;
	getLinkPair(parentName, childName, parentId, childId);
	
	//Add link to the dynamic tree
	btTransform linkTrans = dynamics->getLinkTransform(parentId) * T;
	dynamics->AddLink(links[childId], linkTrans);
	links.erase(links.begin()+childId);
	dynamics->AddFixedJoint(jointName, parentId, dynamics->getNumOfLinks()-1, linkTrans.getOrigin());
}

void Robot::AddToDynamicsWorld(btMultiBodyDynamicsWorld* world, const btTransform& worldTransform)
{
	if(links.size() > 0)
		cCritical("Not all links connected to robot!");
	
	dynamics->AddToDynamicsWorld(world, worldTransform);
}

void Robot::AddLinkActuator(Actuator* act, const std::string& actuatedLinkName)
{
	
}
 
void Robot::AddLinkSensor(Sensor* sens, const std::string& monitoredLinkName)
{
	
}

void Robot::AddJointActuator(Actuator* act, const std::string& actuatedJointName)
{
	
}

void Robot::AddJointSensor(Sensor* sens, const std::string& monitoredJointName)
{
	
}

void Robot::GetAABB(btVector3& min, btVector3& max)
{
	dynamics->GetAABB(min, max);
}

std::vector<Renderable> Robot::Render()
{
	return std::vector<Renderable>(0);
}