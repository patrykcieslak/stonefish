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

Robot::Robot(std::string_view uniqueName, bool fixedBase)
{
    name_ = SimulationApp::GetApp()->GetSimulationManager()->GetNameManager()->AddName(uniqueName);
    fixed_ = fixedBase;
}

Robot::~Robot()
{
    if(SimulationApp::GetApp() != nullptr)
        SimulationApp::GetApp()->GetSimulationManager()->GetNameManager()->RemoveName(name_);
}

const std::string& Robot::GetName() const
{
    return name_;
}

SolidEntity* Robot::GetLink(std::string_view lname)
{
    auto it = std::find_if(links_.begin(), links_.end(), [&lname](SolidEntity* link) { return link->GetName() == lname; });
    if(it != links_.end())
        return *it;
    else
        return nullptr;
}

SolidEntity* Robot::GetLink(size_t index)
{
    if(index < links_.size())
        return links_[index];
    else
        return nullptr;
}
    
Actuator* Robot::GetActuator(std::string_view aname)
{
    auto it = std::find_if(actuators_.begin(), actuators_.end(), [&aname](Actuator* act) { return act->GetName() == aname; });
    if(it != actuators_.end())
        return *it;
    else
        return nullptr;
}

Actuator* Robot::GetActuator(size_t index)
{
    if(index < actuators_.size())
        return actuators_[index];
    else
        return nullptr;
}
    
Sensor* Robot::GetSensor(std::string_view sname)
{
    auto it = std::find_if(sensors_.begin(), sensors_.end(), [&sname](Sensor* sens) { return sens->GetName() == sname; });
    if(it != sensors_.end())
        return *it;
    else
        return nullptr;
}

Sensor* Robot::GetSensor(size_t index)
{
    if(index < sensors_.size())
        return sensors_[index];
    else
        return nullptr;
}

Comm* Robot::GetComm(std::string_view cname)
{
    auto it = std::find_if(comms_.begin(), comms_.end(), [&cname](Comm* comm) { return comm->GetName() == cname; });
    if(it != comms_.end())
        return *it;
    else
        return nullptr;
}

Comm* Robot::GetComm(size_t index)
{
    if(index < comms_.size())
        return comms_[index];
    else
        return nullptr;
}

SolidEntity* Robot::GetBaseLink()
{
    return links_[0];
}

void Robot::DefineRevoluteJoint(std::string_view jointName, std::string_view parentName, std::string_view childName, 
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

void Robot::DefinePrismaticJoint(std::string_view jointName, std::string_view parentName, std::string_view childName, 
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

void Robot::DefineFixedJoint(std::string_view jointName, std::string_view parentName, std::string_view childName, const Transform& origin)
{
    JointData jd;
    jd.jtype = JointType::FIXED;
    jd.name = jointName;
    jd.parent = parentName;
    jd.child = childName;
    jd.origin = origin;
    jointsData_.push_back(jd);
}

LinkSensor* Robot::AddLinkSensor(std::unique_ptr<Sensor, SensorDeleter> s, std::string_view monitoredLinkName, const Transform& origin)
{
    if (s == nullptr || s->GetType() != SensorType::LINK)
    {
        cCritical("Sensor does not exist or is not a link sensor!");
        return nullptr;
    }

    SolidEntity* link = GetLink(monitoredLinkName);
    if(link != nullptr)
    {
        static_cast<LinkSensor*>(s.get())->AttachToSolid(link, origin);
        detachedSensors_.push_back(std::move(s));
        sensors_.push_back(detachedSensors_.back().get());
        return static_cast<LinkSensor*>(sensors_.back());
    }
    else
    {
        cCritical("Link '%s' doesn't exist. Sensor '%s' cannot be attached!", std::string(monitoredLinkName).c_str(), s->GetName().c_str());
        return nullptr;
    }
}

LinkSensor* Robot::AddLinkSensor(std::unique_ptr<Sensor> s, std::string_view monitoredLinkName, const Transform& origin)
{
    return AddLinkSensor(std::unique_ptr<Sensor, SensorDeleter>(s.release(), Sensor::DefaultDeleter), monitoredLinkName, origin);
}

VisionSensor* Robot::AddVisionSensor(std::unique_ptr<Sensor, SensorDeleter> s, std::string_view attachmentLinkName, const Transform& origin)
{
    if (s == nullptr || s->GetType() != SensorType::VISION)
    {
        cCritical("Sensor does not exist or is not a vision sensor!");
        return nullptr;
    }

    SolidEntity* link = GetLink(attachmentLinkName);
    if(link != nullptr)
    {
        static_cast<VisionSensor*>(s.get())->AttachToSolid(link, origin);
        detachedSensors_.push_back(std::move(s));
        sensors_.push_back(detachedSensors_.back().get());
        return static_cast<VisionSensor*>(sensors_.back());
    }
    else
    {
        cCritical("Link '%s' doesn't exist. Sensor '%s' cannot be attached!", std::string(attachmentLinkName).c_str(), s->GetName().c_str());
        return nullptr;
    }
}

VisionSensor* Robot::AddVisionSensor(std::unique_ptr<Sensor> s, std::string_view attachmentLinkName, const Transform& origin)
{
    return AddVisionSensor(std::unique_ptr<Sensor, SensorDeleter>(s.release(), Sensor::DefaultDeleter), attachmentLinkName, origin);
}

LinkActuator* Robot::AddLinkActuator(std::unique_ptr<Actuator, ActuatorDeleter> a, std::string_view actuatedLinkName, const Transform& origin)
{
    if (a == nullptr || a->GetType() != ActuatorType::LINK)
    {
        cCritical("Actuator does not exist or is not a link actuator!");
        return nullptr;
    }

    SolidEntity* link = GetLink(actuatedLinkName);
    if(link != nullptr)
    {
        static_cast<LinkActuator*>(a.get())->AttachToSolid(link, origin);
        detachedActuators_.push_back(std::move(a));
        actuators_.push_back(detachedActuators_.back().get());
        return static_cast<LinkActuator*>(actuators_.back());
    }
    else
    {
        cCritical("Link '%s' doesn't exist. Actuator '%s' cannot be attached!", std::string(actuatedLinkName).c_str(), a->GetName().c_str());
        return nullptr;
    }
}

LinkActuator* Robot::AddLinkActuator(std::unique_ptr<Actuator> a, std::string_view actuatedLinkName, const Transform& origin)
{
    return AddLinkActuator(std::unique_ptr<Actuator, ActuatorDeleter>(a.release(), Actuator::DefaultDeleter), actuatedLinkName, origin);
}

Comm* Robot::AddComm(std::unique_ptr<Comm, CommDeleter> c, std::string_view attachmentLinkName, const Transform& origin)
{
    SolidEntity* link = GetLink(attachmentLinkName);
    if(link != nullptr)
    {
        c->AttachToSolid(link, origin);
        detachedComms_.push_back(std::move(c));
        comms_.push_back(detachedComms_.back().get());
        return comms_.back();
    }
    else
    {
        cCritical("Link '%s' doesn't exist. Communication device '%s' cannot be attached!", std::string(attachmentLinkName).c_str(), c->GetName().c_str());
        return nullptr;
    }
}

Comm* Robot::AddComm(std::unique_ptr<Comm> c, std::string_view attachmentLinkName, const Transform& origin)
{
    return AddComm(std::unique_ptr<Comm, CommDeleter>(c.release(), Comm::DefaultDeleter), attachmentLinkName, origin);
}

void Robot::AddToSimulation(SimulationManager* sm, const Transform& origin)
{
    for(size_t i=0; i<detachedSensors_.size(); ++i)
        sm->AddSensor(std::move(detachedSensors_[i]));
    detachedSensors_.clear();

    for(size_t i=0; i<detachedActuators_.size(); ++i)
        sm->AddActuator(std::move(detachedActuators_[i]));
    detachedActuators_.clear();

    for(size_t i=0; i<detachedComms_.size(); ++i)
        sm->AddComm(std::move(detachedComms_[i]));
    detachedComms_.clear();
}

void Robot::Respawn(SimulationManager* sm, const Transform& origin)
{
}

}
