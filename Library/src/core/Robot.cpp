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
//  Copyright(c) 2018-2020 Patryk Cieslak. All rights reserved.
//

#include "core/Robot.h"

#include "core/SimulationApp.h"
#include "core/SimulationManager.h"
#include "entities/SolidEntity.h"
#include "entities/FeatherstoneEntity.h"
#include "actuators/LinkActuator.h"
#include "actuators/JointActuator.h"
#include "sensors/scalar/LinkSensor.h"
#include "sensors/scalar/JointSensor.h"
#include "sensors/VisionSensor.h"
#include "comms/Comm.h"

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

Actuator* Robot::getActuator(unsigned int index)
{
    if(index < actuators.size())
        return actuators[index];
    else
        return NULL;
}
    
Sensor* Robot::getSensor(std::string name)
{
    for(size_t i=0; i<sensors.size(); ++i)
        if(sensors[i]->getName() == name)
            return sensors[i];
    
    return NULL;
}

Sensor* Robot::getSensor(unsigned int index)
{
    if(index < sensors.size())
        return sensors[index];
    else
        return NULL;
}

Comm* Robot::getComm(std::string name)
{
    for(size_t i=0; i<comms.size(); ++i)
        if(comms[i]->getName() == name)
            return comms[i];
    
    return NULL;
}

Comm* Robot::getComm(unsigned int index)
{
    if(index < comms.size())
        return comms[index];
    else
        return NULL;
}

SolidEntity* Robot::getBaseLink()
{
    return links[0];
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

void Robot::DefineRevoluteJoint(std::string jointName, std::string parentName, std::string childName, const Transform& origin, const Vector3& axis, std::pair<Scalar,Scalar> positionLimits, Scalar damping)
{
    JointData jd;
    jd.jtype = 1;
    jd.name = jointName;
    jd.parent = parentName;
    jd.child = childName;
    jd.origin = origin;
    jd.axis = axis;
    jd.posLim = positionLimits;
    jd.damping = damping;
    joints.push_back(jd);
}

void Robot::DefinePrismaticJoint(std::string jointName, std::string parentName, std::string childName, const Transform& origin, const Vector3& axis, std::pair<Scalar,Scalar> positionLimits, Scalar damping)
{
    JointData jd;
    jd.jtype = 2;
    jd.name = jointName;
    jd.parent = parentName;
    jd.child = childName;
    jd.origin = origin;
    jd.axis = axis;
    jd.posLim = positionLimits;
    jd.damping = damping;
    joints.push_back(jd);
}

void Robot::DefineFixedJoint(std::string jointName, std::string parentName, std::string childName, const Transform& origin)
{
    JointData jd;
    jd.jtype = 0;
    jd.name = jointName;
    jd.parent = parentName;
    jd.child = childName;
    jd.origin = origin;
    joints.push_back(jd);
}

void Robot::BuildKinematicTree()
{
    cInfo("Building kinematic tree of robot '%s', consisting of %d links and %d joints.", getName().c_str(), detachedLinks.size()+1, joints.size());
    
    //Sort joints
    std::vector<JointData> sortedJoints;
    
    //---Add joints connected to base
    for(int i=(int)joints.size()-1; i>=0; --i)
    {
        if(joints[i].parent == links[0]->getName())
        {
            sortedJoints.push_back(joints[i]);
            joints.erase(joints.begin() + i);
            cInfo("Joint %ld: %s<-->%s", sortedJoints.size(), 
                                         sortedJoints.back().parent.c_str(), 
                                         sortedJoints.back().child.c_str());
        }
    }
    
    //---Traverse through tree
    size_t i0=0;
    size_t i1=sortedJoints.size()-1;
    
    while(joints.size() > 0)
    {
        for(size_t i=i0; i<=i1; ++i)
        {
            for(int k=(int)joints.size()-1; k>=0; --k)
            {
                if(joints[k].parent == sortedJoints[i].child)
                {
                    sortedJoints.push_back(joints[k]);
                    joints.erase(joints.begin() + k);
                    cInfo("Joint %ld: %s<-->%s", sortedJoints.size(), 
                                         sortedJoints.back().parent.c_str(), 
                                         sortedJoints.back().child.c_str());
                }
            }
        }
        i0 = i1+1;
        i1 = sortedJoints.size()-1;
    }
    
    joints = sortedJoints;
    
    //Build kinematic tree
    for(size_t i=0; i<joints.size(); ++i)
    {
        unsigned int parentId, childId;
        getFreeLinkPair(joints[i].parent, joints[i].child, parentId, childId);
        
        Transform linkTrans = dynamics->getLinkTransform(parentId) * dynamics->getLink(parentId).solid->getCG2OTransform() * joints[i].origin;
        dynamics->AddLink(detachedLinks[childId], linkTrans);
        links.push_back(detachedLinks[childId]);
        detachedLinks.erase(detachedLinks.begin()+childId);
        
        switch(joints[i].jtype)
        {
            case 0: //FIXED
            {
                dynamics->AddFixedJoint(joints[i].name, parentId, dynamics->getNumOfLinks()-1, linkTrans.getOrigin());
            }
                break;
            
            case 1: //REVOLUTE
            {
                dynamics->AddRevoluteJoint(joints[i].name, parentId, dynamics->getNumOfLinks()-1, linkTrans.getOrigin(), linkTrans.getBasis() * joints[i].axis);
                dynamics->AddJointLimit(dynamics->getNumOfJoints()-1, joints[i].posLim.first, joints[i].posLim.second);
        
                if(joints[i].damping > Scalar(0))
                {
                    dynamics->AddJointMotor(dynamics->getNumOfJoints()-1, joints[i].damping);
                    dynamics->MotorPositionSetpoint(dynamics->getNumOfJoints()-1, Scalar(0), Scalar(0));
                    dynamics->MotorVelocitySetpoint(dynamics->getNumOfJoints()-1, Scalar(0), Scalar(1));
                }
            }
                break;
            
            case 2: //PRISMATIC
            {
                dynamics->AddPrismaticJoint(joints[i].name, parentId, dynamics->getNumOfLinks()-1, linkTrans.getBasis() * joints[i].axis);
                dynamics->AddJointLimit(dynamics->getNumOfJoints()-1, joints[i].posLim.first, joints[i].posLim.second);
        
                if(joints[i].damping > Scalar(0))
                {
                    dynamics->AddJointMotor(dynamics->getNumOfJoints()-1, joints[i].damping);
                    dynamics->MotorPositionSetpoint(dynamics->getNumOfJoints()-1, Scalar(0), Scalar(0));
                    dynamics->MotorVelocitySetpoint(dynamics->getNumOfJoints()-1, Scalar(0), Scalar(1));
                }
            }
                break;
                
            default:
                break;
        }
    }
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

void Robot::AddComm(Comm* c, const std::string& attachmentLinkName, const Transform& origin)
{
    SolidEntity* link = getLink(attachmentLinkName);
    if(link != NULL)
    {
        c->AttachToSolid(link, origin);
        comms.push_back(c);
    }
    else
        cCritical("Link '%s' doesn't exist. Communication device '%s' cannot be attached!", attachmentLinkName.c_str(), c->getName().c_str());
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
    for(size_t i=0; i<comms.size(); ++i)
        sm->AddComm(comms[i]);
}

}
