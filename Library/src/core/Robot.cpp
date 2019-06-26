/*    
    This file is a part of Stonefish.

    Stonefish is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Stonefish is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

//
//  Robot.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 5/11/2018.
//  Copyright(c) 2018 Patryk Cieslak. All rights reserved.
//

#include "core/Robot.h"

#include "core/SimulationApp.h"
#include "core/SimulationManager.h"
#include "core/Console.h"
#include "entities/SolidEntity.h"
#include "entities/FeatherstoneEntity.h"
#include "actuators/LinkActuator.h"
#include "actuators/JointActuator.h"
#include "sensors/scalar/LinkSensor.h"
#include "sensors/scalar/JointSensor.h"
#include "sensors/VisionSensor.h"

namespace sf
{

Robot::Robot(std::string uniqueName, bool fixedBase)
{
    name = SimulationApp::getApp()->getSimulationManager()->getNameManager()->AddName(uniqueName);
    dynamics = NULL;
	fixed = fixedBase;
}

Robot::~Robot()
{
    if(SimulationApp::getApp() != NULL)
        SimulationApp::getApp()->getSimulationManager()->getNameManager()->RemoveName(name);
}

std::string Robot::getName()
{
    return name;
}

void Robot::getFreeLinkPair(const std::string& parentName, const std::string& childName, unsigned int& parentId, unsigned int& childId)
{
	if(dynamics == NULL)
		cCritical("Robot links not defined!");
		
	if(detachedLinks.size() == 0)
		cCritical("No more free links allocated!");
	
	//Find parent ID
	parentId = UINT32_MAX;
	for(unsigned int i=0; i<dynamics->getNumOfLinks(); ++i)
		if(dynamics->getLink(i).solid->getName() == parentName)
			parentId = i;
	
	if(parentId >= dynamics->getNumOfLinks())
		cCritical("Parent link '%s' not yet joined with robot!", parentName.c_str());
	
	//Find child ID
	childId = UINT32_MAX;
	for(unsigned int i=0; i<detachedLinks.size(); ++i)
		if(detachedLinks[i]->getName() == childName)
			childId = i;
			
	if(childId >= detachedLinks.size())
		cCritical("Child link '%s' doesn't exist!", childName.c_str());
}

SolidEntity* Robot::getLink(const std::string& name)
{
    for(size_t i=0; i<links.size(); ++i)
        if(links[i]->getName() == name) return links[i];
    
    for(size_t i=0; i<detachedLinks.size(); ++i)
        if(detachedLinks[i]->getName() == name) return detachedLinks[i];
    
    return NULL;
}

int Robot::getJoint(const std::string& name)
{
    if(dynamics == NULL)
        cCritical("Robot links not defined!");
    
    for(unsigned int i=0; i<dynamics->getNumOfJoints(); ++i)
        if(dynamics->getJointName(i) == name) return i;
    
    return -1;
}
    
Actuator* Robot::getActuator(std::string name)
{
    for(size_t i=0; i<actuators.size(); ++i)
        if(actuators[i]->getName() == name)
            return actuators[i];

    return NULL;
}
    
Sensor* Robot::getSensor(std::string name)
{
    for(size_t i=0; i<sensors.size(); ++i)
        if(sensors[i]->getName() == name)
            return sensors[i];
    
    return NULL;
}

Transform Robot::getTransform() const
{
	if(dynamics != NULL)
		return dynamics->getLinkTransform(0);
	else
		return Transform::getIdentity();
}

void Robot::DefineLinks(SolidEntity* baseLink, std::vector<SolidEntity*> otherLinks, bool selfCollision)
{
	if(dynamics != NULL)
		cCritical("Robot cannot be redefined!");
	
    links.push_back(baseLink);
    detachedLinks = otherLinks;
	dynamics = new FeatherstoneEntity(name + "_Dynamics", (unsigned short)detachedLinks.size() + 1, baseLink, fixed);
    dynamics->setSelfCollision(selfCollision);
}

void Robot::DefineRevoluteJoint(std::string jointName, std::string parentName, std::string childName, const Transform& origin, const Vector3& axis, std::pair<Scalar,Scalar> positionLimits)
{
	unsigned int parentId, childId;
	getFreeLinkPair(parentName, childName, parentId, childId);
	
	//Add link to the dynamic tree
    Transform linkTrans = dynamics->getLinkTransform(parentId) * dynamics->getLink(parentId).solid->getCG2OTransform() * origin;
	dynamics->AddLink(detachedLinks[childId], linkTrans);
    links.push_back(detachedLinks[childId]);
    detachedLinks.erase(detachedLinks.begin()+childId);
	dynamics->AddRevoluteJoint(jointName, parentId, dynamics->getNumOfLinks()-1, linkTrans.getOrigin(), linkTrans.getBasis() * axis);
       
	if(positionLimits.first < positionLimits.second)
		dynamics->AddJointLimit(dynamics->getNumOfJoints()-1, positionLimits.first, positionLimits.second);
	
	//dynamics->setJointDamping(dynamics->getNumOfJoints()-1, 0, 0.5);
}

void Robot::DefinePrismaticJoint(std::string jointName, std::string parentName, std::string childName, const Transform& origin, const Vector3& axis, std::pair<Scalar,Scalar> positionLimits)
{
	unsigned int parentId, childId;
	getFreeLinkPair(parentName, childName, parentId, childId);
	
	//Add link to the dynamic tree
	Transform linkTrans = dynamics->getLinkTransform(parentId) * dynamics->getLink(parentId).solid->getCG2OTransform() * origin;
	dynamics->AddLink(detachedLinks[childId], linkTrans);
    links.push_back(detachedLinks[childId]);
	detachedLinks.erase(detachedLinks.begin()+childId);
    dynamics->AddPrismaticJoint(jointName, parentId, dynamics->getNumOfLinks()-1, linkTrans.getBasis() * axis);
       
	if(positionLimits.first < positionLimits.second)
		dynamics->AddJointLimit(dynamics->getNumOfJoints()-1, positionLimits.first, positionLimits.second);
	
	//dynamics->setJointDamping(dynamics->getNumOfJoints()-1, 0, 0.5);
}

void Robot::DefineFixedJoint(std::string jointName, std::string parentName, std::string childName, const Transform& origin)
{
	unsigned int parentId, childId;
	getFreeLinkPair(parentName, childName, parentId, childId);
	
	//Add link to the dynamic tree
	Transform linkTrans = dynamics->getLinkTransform(parentId) * dynamics->getLink(parentId).solid->getCG2OTransform() * origin;
	dynamics->AddLink(detachedLinks[childId], linkTrans);
    links.push_back(detachedLinks[childId]);
    detachedLinks.erase(detachedLinks.begin()+childId);
	dynamics->AddFixedJoint(jointName, parentId, dynamics->getNumOfLinks()-1, linkTrans.getOrigin());
}

void Robot::AddLinkSensor(LinkSensor* s, const std::string& monitoredLinkName, const Transform& origin)
{
    SolidEntity* link = getLink(monitoredLinkName);
    if(link != NULL)
    {
        s->AttachToSolid(link, origin);
        sensors.push_back(s);
    }
    else
        cCritical("Link '%s' doesn't exist. Sensor '%s' cannot be attached!", monitoredLinkName.c_str(), s->getName().c_str());
}

void Robot::AddJointSensor(JointSensor* s, const std::string& monitoredJointName)
{
    int jointId = getJoint(monitoredJointName);
    if(jointId > -1)
    {
        s->AttachToJoint(dynamics, jointId);
        sensors.push_back(s);
    }
    else
        cCritical("Joint '%s' doesn't exist. Sensor '%s' cannot be attached!", monitoredJointName.c_str(), s->getName().c_str());
}

void Robot::AddVisionSensor(VisionSensor* s, const std::string& attachmentLinkName, const Transform& origin)
{
    SolidEntity* link = getLink(attachmentLinkName);
    if(link != NULL)
    {
        s->AttachToSolid(link, origin);
        sensors.push_back(s);
    }
    else
        cCritical("Link '%s' doesn't exist. Sensor '%s' cannot be attached!", attachmentLinkName.c_str(), s->getName().c_str());
}

void Robot::AddLinkActuator(LinkActuator* a, const std::string& actuatedLinkName, const Transform& origin)
{
    SolidEntity* link = getLink(actuatedLinkName);
    if(link != NULL)
    {
        a->AttachToSolid(link, origin);
        actuators.push_back(a);
    }
    else
        cCritical("Link '%s' doesn't exist. Actuator '%s' cannot be attached!", actuatedLinkName.c_str(), a->getName().c_str());
}

void Robot::AddJointActuator(JointActuator* a, const std::string& actuatedJointName)
{
    int jointId = getJoint(actuatedJointName);
    if(jointId > -1)
    {
        a->AttachToJoint(dynamics, jointId);
        actuators.push_back(a);
    }
    else
        cCritical("Joint '%s' doesn't exist. Actuator '%s' cannot be attached!", actuatedJointName.c_str(), a->getName().c_str());
}

void Robot::AddToSimulation(SimulationManager* sm, const Transform& origin)
{
    if(detachedLinks.size() > 0)
        cCritical("Detected unconnected links!");
    
    sm->AddFeatherstoneEntity(dynamics, origin);
    for(size_t i=0; i<sensors.size(); ++i)
        sm->AddSensor(sensors[i]);
    for(size_t i=0; i<actuators.size(); ++i)
        sm->AddActuator(actuators[i]);
}

}
