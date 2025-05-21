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
//  FeatherstoneRobot.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 5/11/2018.
//  Copyright(c) 2018-2024 Patryk Cieslak. All rights reserved.
//

#include "core/FeatherstoneRobot.h"

#include "core/SimulationApp.h"
#include "core/SimulationManager.h"
#include "entities/SolidEntity.h"
#include "entities/FeatherstoneEntity.h"
#include "actuators/LinkActuator.h"
#include "actuators/JointActuator.h"
#include "actuators/SuctionCup.h"
#include "sensors/scalar/LinkSensor.h"
#include "sensors/scalar/JointSensor.h"
#include "sensors/VisionSensor.h"
#include "comms/Comm.h"

namespace sf
{

FeatherstoneRobot::FeatherstoneRobot(std::string uniqueName, bool fixedBase) : Robot(uniqueName, fixedBase)
{
    dynamics = nullptr;
}

FeatherstoneRobot::~FeatherstoneRobot()
{
}

RobotType FeatherstoneRobot::getType() const
{
    return RobotType::FEATHERSTONE;    
}

int FeatherstoneRobot::getJoint(const std::string& name)
{
    if(dynamics == nullptr)
        cCritical("Robot links not defined!");
    
    for(unsigned int i=0; i<dynamics->getNumOfJoints(); ++i)
        if(dynamics->getJointName(i) == name) return i;
    
    return -1;
}

Transform FeatherstoneRobot::getTransform() const
{
    if(dynamics != nullptr)
        return dynamics->getLink(0).solid->getOTransform();
    else
        return Transform::getIdentity();
}

int FeatherstoneRobot::getLinkIndex(const std::string& name) const
{
    int index = -2;
    if(dynamics != nullptr)
    {
        for(int i=0; i<(int)dynamics->getNumOfLinks(); ++i)
            if(dynamics->getLink(i).solid->getName() == name)
            {
                index = i-1;
                break;
            }
    }
    return index;
}

FeatherstoneEntity* FeatherstoneRobot::getDynamics()
{
    return dynamics;
}

void FeatherstoneRobot::DefineLinks(SolidEntity* baseLink, std::vector<SolidEntity*> otherLinks, bool selfCollision)
{
    if(dynamics != nullptr)
        cCritical("Robot cannot be redefined!");
    
    links.push_back(baseLink);
    detachedLinks = otherLinks;
    dynamics = new FeatherstoneEntity(name + "_Dynamics", (unsigned short)detachedLinks.size() + 1, baseLink, fixed);
    dynamics->setSelfCollision(selfCollision);
}

void FeatherstoneRobot::BuildKinematicStructure()
{
    cInfo("Building kinematic tree of robot '%s', consisting of %d links and %d joints.", getName().c_str(), detachedLinks.size()+1, jointsData.size());
    
    //Sort joints
    std::vector<JointData> sortedJoints;
    
    //---Add joints connected to base
    for(int i=(int)jointsData.size()-1; i>=0; --i)
    {
        if(jointsData[i].parent == links[0]->getName())
        {
            sortedJoints.push_back(jointsData[i]);
            jointsData.erase(jointsData.begin() + i);
            cInfo("Joint %ld: %s<-->%s", sortedJoints.size(), 
                                         sortedJoints.back().parent.c_str(), 
                                         sortedJoints.back().child.c_str());
        }
    }
    
    //---Traverse through tree
    size_t i0=0;
    size_t i1=sortedJoints.size()-1;
    
    while(jointsData.size() > 0)
    {
        for(size_t i=i0; i<=i1; ++i)
        {
            for(int h=(int)jointsData.size()-1; h>=0; --h)
            {
                if(jointsData[h].parent == sortedJoints[i].child)
                {
                    sortedJoints.push_back(jointsData[h]);
                    jointsData.erase(jointsData.begin() + h);
                    cInfo("Joint %ld: %s<-->%s", sortedJoints.size(), 
                                         sortedJoints.back().parent.c_str(), 
                                         sortedJoints.back().child.c_str());
                }
            }
        }
        i0 = i1+1;
        i1 = sortedJoints.size()-1;
    }
    
    jointsData = sortedJoints;
    
    //Build kinematic tree
    for(size_t i=0; i<jointsData.size(); ++i)
    {
        Transform linkTrans;

        // Check if connected links are already part of the model (parallel mechanisms)
        unsigned int parentId = UINT32_MAX;
        unsigned int childId = UINT32_MAX;
        for(size_t h=0; h<dynamics->getNumOfLinks(); ++h)
        {
            if(dynamics->getLink(h).solid->getName() == jointsData[i].parent)
                parentId = h;
            else if(dynamics->getLink(h).solid->getName() == jointsData[i].child)
                childId = h;
        }

        if(parentId < dynamics->getNumOfLinks() 
            && childId < dynamics->getNumOfLinks()) // Kinematic loop
        {
            cCritical("Featherstone's algorithm does not support kinematic loops!");
        }
        else // Standard joint
        {
            if(parentId >= dynamics->getNumOfLinks())
                cCritical("Parent link '%s' not yet joined with robot!", jointsData[i].parent.c_str());

            // Find child link
            for(size_t h=0; h<detachedLinks.size(); ++h)
                if(detachedLinks[h]->getName() == jointsData[i].child)
                childId = h;

            if(childId >= detachedLinks.size())
            {
                cCritical("Child link '%s' doesn't exist!", jointsData[i].child.c_str());
            }
            
            // Add link
            linkTrans = dynamics->getLinkTransform(parentId) * dynamics->getLink(parentId).solid->getCG2OTransform() * jointsData[i].origin;
            dynamics->AddLink(detachedLinks[childId], linkTrans);
            links.push_back(detachedLinks[childId]);
            detachedLinks.erase(detachedLinks.begin()+childId);
            childId = dynamics->getNumOfLinks()-1;
        }

        // Add joint
        switch(jointsData[i].jtype)
        {
            case JointType::FIXED:
            {
                dynamics->AddFixedJoint(jointsData[i].name, parentId, childId, linkTrans.getOrigin());
            }
                break;
            
            case JointType::REVOLUTE:
            {
                dynamics->AddRevoluteJoint(jointsData[i].name, parentId, childId, linkTrans.getOrigin(), linkTrans.getBasis() * jointsData[i].axis);
                dynamics->AddJointLimit(dynamics->getNumOfJoints()-1, jointsData[i].posLim.first, jointsData[i].posLim.second);
        
                if(jointsData[i].damping > Scalar(0))
                {
                    dynamics->setJointDamping(dynamics->getNumOfJoints()-1, 0.0, jointsData[i].damping);
                }
            }
                break;
            
            case JointType::PRISMATIC:
            {
                dynamics->AddPrismaticJoint(jointsData[i].name, parentId, childId, linkTrans.getBasis() * jointsData[i].axis);
                dynamics->AddJointLimit(dynamics->getNumOfJoints()-1, jointsData[i].posLim.first, jointsData[i].posLim.second);

                if(jointsData[i].damping > Scalar(0))
                {
                    dynamics->setJointDamping(dynamics->getNumOfJoints()-1, 0.0, jointsData[i].damping);
                }
            }
                break;
                
            default:
                break;
        }
    }
}

void FeatherstoneRobot::AddToSimulation(SimulationManager* sm, const Transform& origin)
{
    if(detachedLinks.size() > 0)
        cCritical("Detected unconnected links!");

    Robot::AddToSimulation(sm, origin);
    sm->AddFeatherstoneEntity(dynamics, origin);
}

void FeatherstoneRobot::Respawn(SimulationManager* sm, const Transform& origin)
{
    dynamics->Respawn(origin);
}

void FeatherstoneRobot::AddJointSensor(JointSensor* s, const std::string& monitoredJointName)
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

void FeatherstoneRobot::AddJointActuator(JointActuator* a, const std::string& actuatedJointName)
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

void FeatherstoneRobot::AddLinkActuator(LinkActuator* a, const std::string& actuatedLinkName, const Transform& origin)
{
    int linkId = getLinkIndex(actuatedLinkName);
    if(linkId < -1)
    {
        cCritical("Link '%s' doesn't exist. Actuator '%s' cannot be attached!", actuatedLinkName.c_str(), a->getName().c_str());
        return;
    }
    if(a->getType() == ActuatorType::SUCTION_CUP) // Special case
    {
        static_cast<SuctionCup*>(a)->AttachToLink(getDynamics(), linkId);
    }
    else
    {
        a->AttachToSolid(getLink(actuatedLinkName), origin);
    }
    actuators.push_back(a);
}

}