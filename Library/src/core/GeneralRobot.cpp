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
//  Copyright(c) 2023-2025 Patryk Cieslak. All rights reserved.
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
    if(links_.size() > 0)
        return links_[0]->getOTransform();
    else
        return Transform::getIdentity();
}

void GeneralRobot::DefineLinks(SolidEntity* baseLink, std::vector<SolidEntity*> otherLinks, bool selfCollision)
{
    links_.push_back(baseLink);
    links_.insert(links_.end(), otherLinks.begin(), otherLinks.end());
}
        
void GeneralRobot::BuildKinematicStructure()
{   
}
        
void GeneralRobot::AddJointSensor(JointSensor* s, const std::string& monitoredJointName)
{
    for(size_t i = 0; i < jointsData_.size(); ++i)
        if(jointsData_[i].name == monitoredJointName)
        {
            jsAttachments.push_back(std::make_pair(s, monitoredJointName));
            sensors_.push_back(s);    
            break;
        }
}

void GeneralRobot::AddJointActuator(JointActuator* a, const std::string& actuatedJointName)
{
    for(size_t i = 0; i < jointsData_.size(); ++i)
        if(jointsData_[i].name == actuatedJointName)
        {
            jaAttachments.push_back(std::make_pair(a, actuatedJointName));
            actuators_.push_back(a);    
            break;
        }
}

void GeneralRobot::AddToSimulation(SimulationManager* sm, const Transform& origin)
{
    Robot::AddToSimulation(sm, origin);   
    
    // Base link
    sm->AddSolidEntity(links_[0], origin);
    if(fixed_)
    {
        FixedJoint* fix = new FixedJoint(name_ + "_fix", links_[0]);
        joints.push_back(fix);
    }

    // Rest of the links and joints
    for(size_t i = 1; i < links_.size(); ++i)
    {
        sm->AddSolidEntity(links_[i], origin);       
    }

    for(size_t i = 0; i < jointsData_.size(); ++i)
    {
        size_t parentId;
        size_t childId;
        
        for(size_t h = 0; h < links_.size(); ++h)
        {            
            if(links_[h]->getName() == jointsData_[i].parent)
                parentId = h;
            else if(links_[h]->getName() == jointsData_[i].child)
                childId = h;
        }
        if(parentId >= links_.size())
            cCritical("Parent link '%s' doesn't exist!", jointsData_[i].parent.c_str());
        if(childId >= links_.size())
            cCritical("Child link '%s' doesn't exist!", jointsData_[i].child.c_str());

        switch(jointsData_[i].jtype)
        {
            case JointType::FIXED:
            {
                FixedJoint* fix = new FixedJoint(jointsData_[i].name, 
                                                    links_[parentId], 
                                                    links_[childId]);
                joints.push_back(fix);
            }
                break;

            case JointType::REVOLUTE:
            {
                RevoluteJoint* rev = new RevoluteJoint(jointsData_[i].name, 
                                                        links_[parentId], 
                                                        links_[childId], 
                                                        (origin * jointsData_[i].origin).getOrigin(),
                                                        (origin * jointsData_[i].origin).getBasis() * jointsData_[i].axis, false);
                rev->setLimits(jointsData_[i].posLim.first, jointsData_[i].posLim.second);
                joints.push_back(rev);
            }
                break;

            case JointType::PRISMATIC:
            {
                PrismaticJoint* prism = new PrismaticJoint(jointsData_[i].name,
                                                            links_[parentId],
                                                            links_[childId],
                                                            (origin * jointsData_[i].origin).getBasis() * jointsData_[i].axis, false);
                prism->setLimits(jointsData_[i].posLim.first, jointsData_[i].posLim.second);
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