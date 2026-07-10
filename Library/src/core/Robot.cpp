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
//  Copyright(c) 2018-2026 Patryk Cieslak. All rights reserved.
//

#include "core/Robot.h"

#include <algorithm>
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

Robot::Robot(const std::string& uniqueName, bool fixedBase)
{
    name_ = SimulationApp::getApp()->getSimulationManager()->getNameManager()->AddName(uniqueName);
    fixed_ = fixedBase;
}

Robot::~Robot()
{
    if(SimulationApp::getApp() != nullptr)
        SimulationApp::getApp()->getSimulationManager()->getNameManager()->RemoveName(name_);
}

const std::string& Robot::getName() const
{
    return name_;
}

SolidEntity* Robot::getLink(const std::string& lname)
{
    auto it = std::find_if(links_.begin(), links_.end(), [&lname](SolidEntity* link) { return link->getName() == lname; });
    if(it != links_.end())
        return *it;
    else
        return nullptr;
}

SolidEntity* Robot::getLink(size_t index)
{
    if(index < links_.size())
        return links_[index];
    else
        return nullptr;
}
    
Actuator* Robot::getActuator(const std::string& aname)
{
    auto it = std::find_if(actuators_.begin(), actuators_.end(), [&aname](Actuator* act) { return act->getName() == aname; });
    if(it != actuators_.end())
        return *it;
    else
        return nullptr;
}

Actuator* Robot::getActuator(size_t index)
{
    if(index < actuators_.size())
        return actuators_[index];
    else
        return nullptr;
}
    
Sensor* Robot::getSensor(const std::string& sname)
{
    auto it = std::find_if(sensors_.begin(), sensors_.end(), [&sname](Sensor* sens) { return sens->getName() == sname; });
    if(it != sensors_.end())
        return *it;
    else
        return nullptr;
}

Sensor* Robot::getSensor(size_t index)
{
    if(index < sensors_.size())
        return sensors_[index];
    else
        return nullptr;
}

Comm* Robot::getComm(const std::string& cname)
{
    auto it = std::find_if(comms_.begin(), comms_.end(), [&cname](Comm* comm) { return comm->getName() == cname; });
    if(it != comms_.end())
        return *it;
    else
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

void Robot::DefineRevoluteJoint(const std::string& jointName, const std::string& parentName, const std::string& childName, 
    const Transform& origin, const Vector3& axis, std::pair<Scalar,Scalar> positionLimits, Scalar damping)
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

void Robot::DefinePrismaticJoint(const std::string& jointName, const std::string& parentName, const std::string& childName, 
    const Transform& origin, const Vector3& axis, std::pair<Scalar,Scalar> positionLimits, Scalar damping)
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

void Robot::DefineFixedJoint(const std::string& jointName, const std::string& parentName, const std::string& childName, const Transform& origin)
{
    JointData jd;
    jd.jtype = JointType::FIXED;
    jd.name = jointName;
    jd.parent = parentName;
    jd.child = childName;
    jd.origin = origin;
    jointsData_.push_back(jd);
}

LinkSensor* Robot::AddLinkSensor(std::unique_ptr<LinkSensor> s, const std::string& monitoredLinkName, const Transform& origin)
{
    SolidEntity* link = getLink(monitoredLinkName);
    if(link != nullptr)
    {
        s->AttachToSolid(link, origin);
        sensors_.push_back(s.release());
        return static_cast<LinkSensor*>(sensors_.back());
    }
    else
    {
        cCritical("Link '%s' doesn't exist. Sensor '%s' cannot be attached!", monitoredLinkName.c_str(), s->getName().c_str());
        return nullptr;
    }
}

VisionSensor* Robot::AddVisionSensor(std::unique_ptr<VisionSensor> s, const std::string& attachmentLinkName, const Transform& origin)
{
    SolidEntity* link = getLink(attachmentLinkName);
    if(link != nullptr)
    {
        s->AttachToSolid(link, origin);
        sensors_.push_back(s.release());
        return static_cast<VisionSensor*>(sensors_.back());
    }
    else
    {
        cCritical("Link '%s' doesn't exist. Sensor '%s' cannot be attached!", attachmentLinkName.c_str(), s->getName().c_str());
        return nullptr;
    }
}

LinkActuator* Robot::AddLinkActuator(std::unique_ptr<LinkActuator> a, const std::string& actuatedLinkName, const Transform& origin)
{
    SolidEntity* link = getLink(actuatedLinkName);
    if(link != nullptr)
    {
        a->AttachToSolid(link, origin);
        actuators_.push_back(a.release());
        return static_cast<LinkActuator*>(actuators_.back());
    }
    else
    {
        cCritical("Link '%s' doesn't exist. Actuator '%s' cannot be attached!", actuatedLinkName.c_str(), a->getName().c_str());
        return nullptr;
    }
}

Comm* Robot::AddComm(std::unique_ptr<Comm> c, const std::string& attachmentLinkName, const Transform& origin)
{
    SolidEntity* link = getLink(attachmentLinkName);
    if(link != nullptr)
    {
        c->AttachToSolid(link, origin);
        comms_.push_back(c.release());
        return comms_.back();
    }
    else
    {
        cCritical("Link '%s' doesn't exist. Communication device '%s' cannot be attached!", attachmentLinkName.c_str(), c->getName().c_str());
        return nullptr;
    }
}

void Robot::AddToSimulation(SimulationManager* sm, const Transform& origin)
{
    for(size_t i=0; i<sensors_.size(); ++i)
        sm->AddSensor(std::unique_ptr<Sensor>(sensors_[i]));
    for(size_t i=0; i<actuators_.size(); ++i)
        sm->AddActuator(std::unique_ptr<Actuator>(actuators_[i]));
    for(size_t i=0; i<comms_.size(); ++i)
        sm->AddComm(std::unique_ptr<Comm>(comms_[i]));
}

void Robot::Respawn(SimulationManager* sm, const Transform& origin)
{
}

}
