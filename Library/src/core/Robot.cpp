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
//  Copyright(c) 2018-2025 Patryk Cieslak. All rights reserved.
//

#include "core/Robot.h"

#include "core/SimulationApp.h"
#include "core/SimulationManager.h"
#include "entities/SolidEntity.h"
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
    name_ = SimulationApp::getApp()->getSimulationManager()->getNameManager()->AddName(uniqueName);
    fixed_ = fixedBase;
}

Robot::~Robot()
{
    if(SimulationApp::getApp() != nullptr)
        SimulationApp::getApp()->getSimulationManager()->getNameManager()->RemoveName(name_);
}

std::string Robot::getName()
{
    return name_;
}

SolidEntity* Robot::getLink(const std::string& lname)
{
    for(size_t i=0; i<links_.size(); ++i)
        if(links_[i]->getName() == lname) return links_[i];

    for(size_t i=0; i<detachedLinks_.size(); ++i)
        if(detachedLinks_[i]->getName() == lname) return detachedLinks_[i];
    
    return nullptr;
}

SolidEntity* Robot::getLink(size_t index)
{
    if(index < links_.size())
        return links_[index];
    else
        return nullptr;
}
    
Actuator* Robot::getActuator(std::string aname)
{
    for(size_t i=0; i<actuators_.size(); ++i)
        if(actuators_[i]->getName() == aname)
            return actuators_[i];

    return nullptr;
}

Actuator* Robot::getActuator(size_t index)
{
    if(index < actuators_.size())
        return actuators_[index];
    else
        return nullptr;
}
    
Sensor* Robot::getSensor(std::string sname)
{
    for(size_t i=0; i<sensors_.size(); ++i)
        if(sensors_[i]->getName() == sname)
            return sensors_[i];
    
    return nullptr;
}

Sensor* Robot::getSensor(size_t index)
{
    if(index < sensors_.size())
        return sensors_[index];
    else
        return nullptr;
}

Comm* Robot::getComm(std::string cname)
{
    for(size_t i=0; i<comms_.size(); ++i)
        if(comms_[i]->getName() == cname)
            return comms_[i];
    
    return nullptr;
}

Comm* Robot::getComm(size_t index)
{
    if(index < comms_.size())
        return comms_[index];
    else
        return nullptr;
}

SolidEntity* Robot::getBaseLink()
{
    return links_[0];
}

void Robot::DefineRevoluteJoint(std::string jointName, std::string parentName, std::string childName, const Transform& origin, const Vector3& axis, std::pair<Scalar,Scalar> positionLimits, Scalar damping)
{
    JointData jd;
    jd.jtype = JointType::REVOLUTE;
    jd.name = jointName;
    jd.parent = parentName;
    jd.child = childName;
    jd.origin = origin;
    jd.axis = axis;
    jd.posLim = positionLimits;
    jd.damping = damping;
    jointsData_.push_back(jd);
}

void Robot::DefinePrismaticJoint(std::string jointName, std::string parentName, std::string childName, const Transform& origin, const Vector3& axis, std::pair<Scalar,Scalar> positionLimits, Scalar damping)
{
    JointData jd;
    jd.jtype = JointType::PRISMATIC;
    jd.name = jointName;
    jd.parent = parentName;
    jd.child = childName;
    jd.origin = origin;
    jd.axis = axis;
    jd.posLim = positionLimits;
    jd.damping = damping;
    jointsData_.push_back(jd);
}

void Robot::DefineFixedJoint(std::string jointName, std::string parentName, std::string childName, const Transform& origin)
{
    JointData jd;
    jd.jtype = JointType::FIXED;
    jd.name = jointName;
    jd.parent = parentName;
    jd.child = childName;
    jd.origin = origin;
    jointsData_.push_back(jd);
}

void Robot::AddLinkSensor(LinkSensor* s, const std::string& monitoredLinkName, const Transform& origin)
{
    SolidEntity* link = getLink(monitoredLinkName);
    if(link != nullptr)
    {
        s->AttachToSolid(link, origin);
        sensors_.push_back(s);
    }
    else
        cCritical("Link '%s' doesn't exist. Sensor '%s' cannot be attached!", monitoredLinkName.c_str(), s->getName().c_str());
}

void Robot::AddVisionSensor(VisionSensor* s, const std::string& attachmentLinkName, const Transform& origin)
{
    SolidEntity* link = getLink(attachmentLinkName);
    if(link != nullptr)
    {
        s->AttachToSolid(link, origin);
        sensors_.push_back(s);
    }
    else
        cCritical("Link '%s' doesn't exist. Sensor '%s' cannot be attached!", attachmentLinkName.c_str(), s->getName().c_str());
}

void Robot::AddLinkActuator(LinkActuator* a, const std::string& actuatedLinkName, const Transform& origin)
{
    SolidEntity* link = getLink(actuatedLinkName);
    if(link == nullptr)
    {
        cCritical("Link '%s' doesn't exist. Actuator '%s' cannot be attached!", actuatedLinkName.c_str(), a->getName().c_str());
        return;
    }
    a->AttachToSolid(link, origin);
    actuators_.push_back(a);
}

void Robot::AddComm(Comm* c, const std::string& attachmentLinkName, const Transform& origin)
{
    SolidEntity* link = getLink(attachmentLinkName);
    if(link != nullptr)
    {
        c->AttachToSolid(link, origin);
        comms_.push_back(c);
    }
    else
        cCritical("Link '%s' doesn't exist. Communication device '%s' cannot be attached!", attachmentLinkName.c_str(), c->getName().c_str());
}

void Robot::AddToSimulation(SimulationManager* sm, const Transform& origin)
{
    for(size_t i=0; i<sensors_.size(); ++i)
        sm->AddSensor(sensors_[i]);
    for(size_t i=0; i<actuators_.size(); ++i)
        sm->AddActuator(actuators_[i]);
    for(size_t i=0; i<comms_.size(); ++i)
        sm->AddComm(comms_[i]);
}

void Robot::Respawn(SimulationManager* sm, const Transform& origin)
{
}

}
