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
//  Copyright(c) 2023-2026 Patryk Cieslak. All rights reserved.
//

#include "core/GeneralRobot.h"

#include <algorithm>
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

RobotType GeneralRobot::getType() const
{
    return RobotType::GENERAL;
}

Joint* GeneralRobot::getJoint(const std::string& name)
{
    auto it = std::find_if(joints_.begin(), joints_.end(), [&name](Joint* joint) { return joint->getName() == name; });
    if(it != joints_.end())
        return *it;
    else
        return nullptr;
}

Transform GeneralRobot::getTransform() const
{
    if(links_.size() > 0)
        return links_[0]->getOTransform();
    else
        return Transform::getIdentity();
}

void GeneralRobot::DefineLinks(std::unique_ptr<SolidEntity> baseLink, std::vector<std::unique_ptr<SolidEntity>> otherLinks, bool selfCollision)
{
    detachedLinks_.push_back(std::move(baseLink));
    detachedLinks_.insert(detachedLinks_.end(), std::make_move_iterator(otherLinks.begin()), std::make_move_iterator(otherLinks.end()));

    // Save pointers to links only referencing the memory managed originals
    for (size_t i = 0; i < detachedLinks_.size(); ++i)
        links_.push_back(detachedLinks_[i].get());
}
        
void GeneralRobot::BuildKinematicStructure()
{   
}
        
void GeneralRobot::AddJointSensor(JointSensor* s, const std::string& monitoredJointName)
{
    for(size_t i = 0; i < jointsData_.size(); ++i)
        if(jointsData_[i].name == monitoredJointName)
        {
            jsAttachments_.push_back(std::make_pair(s, monitoredJointName));
            sensors_.push_back(s);    
            break;
        }
}

void GeneralRobot::AddJointActuator(JointActuator* a, const std::string& actuatedJointName)
{
    for(size_t i = 0; i < jointsData_.size(); ++i)
        if(jointsData_[i].name == actuatedJointName)
        {
            jaAttachments_.push_back(std::make_pair(a, actuatedJointName));
            actuators_.push_back(a);    
            break;
        }
}

void GeneralRobot::AddToSimulation(SimulationManager* sm, const Transform& origin)
{
    Robot::AddToSimulation(sm, origin);   
    
    // All links
    for(size_t i = 0; i < detachedLinks_.size(); ++i)
        sm->AddSolidEntity(std::move(detachedLinks_[i]), origin);
    detachedLinks_.clear();

    // Base link joint    
    if(fixed_)
    {
        FixedJoint* fix = new FixedJoint(name_ + "_fix", links_[0]);
        joints_.push_back(fix);
    }

    // Rest of the links and joints
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
                joints_.push_back(fix);
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
                joints_.push_back(rev);
            }
                break;

            case JointType::PRISMATIC:
            {
                PrismaticJoint* prism = new PrismaticJoint(jointsData_[i].name,
                                                            links_[parentId],
                                                            links_[childId],
                                                            (origin * jointsData_[i].origin).getBasis() * jointsData_[i].axis, false);
                prism->setLimits(jointsData_[i].posLim.first, jointsData_[i].posLim.second);
                joints_.push_back(prism);
            }
                break;

            default:
                break;
        }
    }

    for(size_t i = 0; i < joints_.size(); ++i)
        sm->AddJoint(std::unique_ptr<Joint>(joints_[i]));

    // Attach joint sensors
    for(size_t i = 0; i < jsAttachments_.size(); ++i)
    {
        Joint* j = getJoint(jsAttachments_[i].second);
        if(j != nullptr)
            jsAttachments_[i].first->AttachToJoint(j);
        else
            cCritical("Joint '%s' doesn't exist. Sensor '%s' cannot be attached!", 
                        jsAttachments_[i].second.c_str(), 
                        jsAttachments_[i].first->getName().c_str());
    }    

    // Attach joint actuators
    for(size_t i = 0; i < jaAttachments_.size(); ++i)
    {
        Joint* j = getJoint(jaAttachments_[i].second);
        if(j != nullptr)
            jaAttachments_[i].first->AttachToJoint(j);
        else
            cCritical("Joint '%s' doesn't exist. Actuator '%s' cannot be attached!", 
                        jaAttachments_[i].second.c_str(), 
                        jaAttachments_[i].first->getName().c_str());
    }
}

}