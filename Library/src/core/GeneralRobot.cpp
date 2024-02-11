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
//  GeneralRobot.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 27/01/2023.
//  Copyright(c) 2023 Patryk Cieslak. All rights reserved.
//

#include "core/GeneralRobot.h"

#include "core/SimulationApp.h"
#include "core/SimulationManager.h"
#include "entities/SolidEntity.h"
#include "joints/Joint.h"
#include "actuators/JointActuator.h"
#include "sensors/scalar/JointSensor.h"
#include "joints/FixedJoint.h"
#include "joints/PrismaticJoint.h"
#include "joints/RevoluteJoint.h"

namespace sf
{

GeneralRobot::GeneralRobot(std::string uniqueName, bool fixedBase) : Robot(uniqueName, fixedBase)
{
}       

GeneralRobot::~GeneralRobot()
{
}

RobotType GeneralRobot::getType() const
{
    return RobotType::GENERAL;
}

Joint* GeneralRobot::getJoint(const std::string& name)
{
    for(size_t i=0; i<joints.size(); ++i)
        if(joints[i]->getName() == name) return joints[i];
 
    return nullptr;
}

Transform GeneralRobot::getTransform() const
{
    if(links.size() > 0)
        return links[0]->getCGTransform();
    else
        return Transform::getIdentity();
}

void GeneralRobot::DefineLinks(SolidEntity* baseLink, std::vector<SolidEntity*> otherLinks, bool selfCollision)
{
    links.push_back(baseLink);
    links.insert(links.end(), otherLinks.begin(), otherLinks.end());
}

void GeneralRobot::BuildKinematicStructure()
{   
}
        
void GeneralRobot::AddJointSensor(JointSensor* s, const std::string& monitoredJointName)
{
    for(size_t i = 0; i < jointsData.size(); ++i)
        if(jointsData[i].name == monitoredJointName)
        {
            jsAttachments.push_back(std::make_pair(s, monitoredJointName));
            sensors.push_back(s);    
            break;
        }
}

void GeneralRobot::AddJointActuator(JointActuator* a, const std::string& actuatedJointName)
{
    for(size_t i = 0; i < jointsData.size(); ++i)
        if(jointsData[i].name == actuatedJointName)
        {
            jaAttachments.push_back(std::make_pair(a, actuatedJointName));
            actuators.push_back(a);    
            break;
        }
}

void GeneralRobot::AddToSimulation(SimulationManager* sm, const Transform& origin)
{
    Robot::AddToSimulation(sm, origin);   
    
    // Base link
    sm->AddSolidEntity(links[0], origin);
    if(fixed)
    {
        FixedJoint* fix = new FixedJoint(name + "_fix", links[0]);
        joints.push_back(fix);
    }

    // Rest of the links and joints
    for(size_t i = 1; i < links.size(); ++i)
    {
        sm->AddSolidEntity(links[i], origin);       
    }

    for(size_t i = 0; i < jointsData.size(); ++i)
    {
        size_t parentId;
        size_t childId;
        
        for(size_t h = 0; h < links.size(); ++h)
        {            
            if(links[h]->getName() == jointsData[i].parent)
                parentId = h;
            else if(links[h]->getName() == jointsData[i].child)
                childId = h;
        }
        if(parentId >= links.size())
            cCritical("Parent link '%s' doesn't exist!", jointsData[i].parent.c_str());
        if(childId >= links.size())
            cCritical("Child link '%s' doesn't exist!", jointsData[i].child.c_str());

        switch(jointsData[i].jtype)
        {
            case JointType::FIXED:
            {
                FixedJoint* fix = new FixedJoint(jointsData[i].name, 
                                                    links[parentId], 
                                                    links[childId]);
                joints.push_back(fix);
            }
                break;

            case JointType::REVOLUTE:
            {
                RevoluteJoint* rev = new RevoluteJoint(jointsData[i].name, 
                                                        links[parentId], 
                                                        links[childId], 
                                                        (origin * jointsData[i].origin).getOrigin(),
                                                        (origin * jointsData[i].origin).getBasis() * jointsData[i].axis, false);
                rev->setLimits(jointsData[i].posLim.first, jointsData[i].posLim.second);
                joints.push_back(rev);
            }
                break;

            case JointType::PRISMATIC:
            {
                PrismaticJoint* prism = new PrismaticJoint(jointsData[i].name,
                                                            links[parentId],
                                                            links[childId],
                                                            (origin * jointsData[i].origin).getBasis() * jointsData[i].axis, false);
                prism->setLimits(jointsData[i].posLim.first, jointsData[i].posLim.second);
                joints.push_back(prism);
            }
                break;

            default:
                break;
        }
    }

    for(size_t i = 0; i < joints.size(); ++i)
        sm->AddJoint(joints[i]);

    // Attach joint sensors
    for(size_t i = 0; i < jsAttachments.size(); ++i)
    {
        Joint* j = getJoint(jsAttachments[i].second);
        if(j != nullptr)
            jsAttachments[i].first->AttachToJoint(j);
        else
            cCritical("Joint '%s' doesn't exist. Sensor '%s' cannot be attached!", 
                        jsAttachments[i].second.c_str(), 
                        jsAttachments[i].first->getName().c_str());
    }    

    // Attach joint actuators
    for(size_t i = 0; i < jaAttachments.size(); ++i)
    {
        Joint* j = getJoint(jaAttachments[i].second);
        if(j != nullptr)
            jaAttachments[i].first->AttachToJoint(j);
        else
            cCritical("Joint '%s' doesn't exist. Actuator '%s' cannot be attached!", 
                        jaAttachments[i].second.c_str(), 
                        jaAttachments[i].first->getName().c_str());
    }
}

}
