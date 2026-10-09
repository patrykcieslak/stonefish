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
//  Copyright(c) 2018-2026 Patryk Cieslak. All rights reserved.
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

FeatherstoneRobot::FeatherstoneRobot(std::string_view uniqueName, bool fixedBase) : Robot(uniqueName, fixedBase)
{
    dynamics_ = nullptr;
}

RobotType FeatherstoneRobot::GetType() const
{
    return RobotType::FEATHERSTONE;    
}

int FeatherstoneRobot::GetJoint(std::string_view jname)
{
    if(dynamics_ == nullptr)
        cCritical("Robot links not defined!");
    
    for(size_t i=0; i<dynamics_->GetNumOfJoints(); ++i)
        if(dynamics_->GetJointName(i) == jname) return i;
    
    return -1;
}

Transform FeatherstoneRobot::GetTransform() const
{
    if(dynamics_ != nullptr)
        return dynamics_->GetLink(0).solid->GetOTransform();
    else
        return Transform::getIdentity();
}

int FeatherstoneRobot::GetLinkIndex(std::string_view lname) const
{
    int index = -2;
    if(dynamics_ != nullptr)
    {
        for(int i=0; i<(int)dynamics_->GetNumOfLinks(); ++i)
            if(dynamics_->GetLink(i).solid->GetName() == lname)
            {
                index = i-1;
                break;
            }
    }
    return index;
}

FeatherstoneEntity* FeatherstoneRobot::GetDynamics()
{
    return dynamics_;
}

void FeatherstoneRobot::DefineLinks(std::unique_ptr<SolidEntity> baseLink, std::vector<std::unique_ptr<SolidEntity>> otherLinks, bool selfCollision)
{
    if(dynamics_ != nullptr)
        cCritical("Robot cannot be redefined!");
    
    links_.push_back(baseLink.get()); // Save pointer to base link
    detachedLinks_ = std::move(otherLinks);

    // This is later encapsulated in a unique_ptr
    dynamics_ = new FeatherstoneEntity(name_ + "_Dynamics", (unsigned short)detachedLinks_.size() + 1, std::move(baseLink), fixed_);
    dynamics_->SetSelfCollision(selfCollision);
}

void FeatherstoneRobot::BuildKinematicStructure()
{
    cInfo("Building kinematic tree of robot '%s', consisting of %d links and %d joints.", GetName().c_str(), detachedLinks_.size()+1, jointsData_.size());
    
    //Sort joints
    std::vector<JointData> sortedJoints;
    
    //---Add joints connected to base
    for(int i=(int)jointsData_.size()-1; i>=0; --i)
    {
        if(jointsData_[i].parent == links_[0]->GetName())
        {
            sortedJoints.push_back(jointsData_[i]);
            jointsData_.erase(jointsData_.begin() + i);
            cInfo("Joint %ld: %s<-->%s", sortedJoints.size(), 
                                         sortedJoints.back().parent.c_str(), 
                                         sortedJoints.back().child.c_str());
        }
    }
    
    //---Traverse through tree
    size_t i0=0;
    size_t i1=sortedJoints.size()-1;
    
    while(jointsData_.size() > 0)
    {
        for(size_t i=i0; i<=i1; ++i)
        {
            for(int h=(int)jointsData_.size()-1; h>=0; --h)
            {
                if(jointsData_[h].parent == sortedJoints[i].child)
                {
                    sortedJoints.push_back(jointsData_[h]);
                    jointsData_.erase(jointsData_.begin() + h);
                    cInfo("Joint %ld: %s<-->%s", sortedJoints.size(), 
                                         sortedJoints.back().parent.c_str(), 
                                         sortedJoints.back().child.c_str());
                }
            }
        }
        i0 = i1+1;
        i1 = sortedJoints.size()-1;
    }
    
    jointsData_ = sortedJoints;
    
    //Build kinematic tree
    for(size_t i=0; i<jointsData_.size(); ++i)
    {
        Transform linkTrans;

        // Check if connected links are already part of the model (parallel mechanisms)
        unsigned int parentId = UINT32_MAX;
        unsigned int childId = UINT32_MAX;
        for(size_t h=0; h<dynamics_->GetNumOfLinks(); ++h)
        {
            if(dynamics_->GetLink(h).solid->GetName() == jointsData_[i].parent)
                parentId = h;
            else if(dynamics_->GetLink(h).solid->GetName() == jointsData_[i].child)
                childId = h;
        }

        if(parentId < dynamics_->GetNumOfLinks() 
            && childId < dynamics_->GetNumOfLinks()) // Kinematic loop
        {
            cCritical("Featherstone's algorithm does not support kinematic loops!");
        }
        else // Standard joint
        {
            if(parentId >= dynamics_->GetNumOfLinks())
                cCritical("Parent link '%s' not yet joined with robot!", jointsData_[i].parent.c_str());

            // Find child link
            for(size_t h=0; h<detachedLinks_.size(); ++h)
                if(detachedLinks_[h]->GetName() == jointsData_[i].child)
                childId = h;

            if(childId >= detachedLinks_.size())
            {
                cCritical("Child link '%s' doesn't exist!", jointsData_[i].child.c_str());
            }
            
            // Add link
            linkTrans = dynamics_->GetLinkTransform(parentId) * dynamics_->GetLink(parentId).solid->GetCG2OTransform() * jointsData_[i].origin;
            links_.push_back(detachedLinks_[childId].get()); // Save pointer to child link
            dynamics_->AddLink(std::move(detachedLinks_[childId]), linkTrans);
            detachedLinks_.erase(detachedLinks_.begin()+childId);
            childId = dynamics_->GetNumOfLinks()-1;
        }

        // Add joint
        switch(jointsData_[i].jtype)
        {
            case JointType::FIXED:
            {
                dynamics_->AddFixedJoint(jointsData_[i].name, parentId, childId, linkTrans.getOrigin());
            }
                break;
            
            case JointType::REVOLUTE:
            {
                dynamics_->AddRevoluteJoint(jointsData_[i].name, parentId, childId, linkTrans.getOrigin(), linkTrans.getBasis() * jointsData_[i].axis);
                dynamics_->AddJointLimit(dynamics_->GetNumOfJoints()-1, jointsData_[i].posLim.first, jointsData_[i].posLim.second);
        
                if(jointsData_[i].damping > Scalar(0))
                {
                    dynamics_->SetJointDamping(dynamics_->GetNumOfJoints()-1, 0.0, jointsData_[i].damping);
                }
            }
                break;
            
            case JointType::PRISMATIC:
            {
                dynamics_->AddPrismaticJoint(jointsData_[i].name, parentId, childId, linkTrans.getBasis() * jointsData_[i].axis);
                dynamics_->AddJointLimit(dynamics_->GetNumOfJoints()-1, jointsData_[i].posLim.first, jointsData_[i].posLim.second);

                if(jointsData_[i].damping > Scalar(0))
                {
                    dynamics_->SetJointDamping(dynamics_->GetNumOfJoints()-1, 0.0, jointsData_[i].damping);
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
    if(detachedLinks_.size() > 0)
        cCritical("Detected unconnected links!");

    Robot::AddToSimulation(sm, origin);
    sm->AddFeatherstoneEntity(std::unique_ptr<FeatherstoneEntity>(dynamics_), origin);
}

void FeatherstoneRobot::Respawn(SimulationManager* sm, const Transform& origin)
{
    dynamics_->Respawn(origin);
}

JointSensor* FeatherstoneRobot::AddJointSensor(std::unique_ptr<Sensor, SensorDeleter> s, std::string_view monitoredJointName)
{
    if (s == nullptr || s->GetType() != SensorType::JOINT)
    {
        cCritical("Sensor does not exist or is not a joint sensor!");
        return nullptr;
    }

    int jointId = GetJoint(monitoredJointName);
    if(jointId > -1)
    {
        static_cast<JointSensor*>(s.get())->AttachToJoint(dynamics_, jointId);
        detachedSensors_.push_back(std::move(s));
        sensors_.push_back(detachedSensors_.back().get());
        return static_cast<JointSensor*>(sensors_.back());
    }
    else
    {
        cCritical("Joint '%s' doesn't exist. Sensor '%s' cannot be attached!", std::string(monitoredJointName).c_str(), s->GetName().c_str());
        return nullptr;
    }
}

JointSensor* FeatherstoneRobot::AddJointSensor(std::unique_ptr<Sensor> s, std::string_view monitoredJointName)
{
    return AddJointSensor(std::unique_ptr<Sensor, SensorDeleter>(s.release(), Sensor::DefaultDeleter), monitoredJointName);
}

JointActuator* FeatherstoneRobot::AddJointActuator(std::unique_ptr<Actuator, ActuatorDeleter> a, std::string_view actuatedJointName)
{
    if (a == nullptr || a->GetType() != ActuatorType::JOINT)
    {
        cCritical("Actuator does not exist or is not a joint actuator!");
        return nullptr;
    }

    int jointId = GetJoint(actuatedJointName);
    if(jointId > -1)
    {
        static_cast<JointActuator*>(a.get())->AttachToJoint(dynamics_, jointId);
        detachedActuators_.push_back(std::move(a));
        actuators_.push_back(detachedActuators_.back().get());
        return static_cast<JointActuator*>(actuators_.back());
    }
    else
    {
        cCritical("Joint '%s' doesn't exist. Actuator '%s' cannot be attached!", std::string(actuatedJointName).c_str(), a->GetName().c_str());
        return nullptr;
    }
}

JointActuator* FeatherstoneRobot::AddJointActuator(std::unique_ptr<Actuator> a, std::string_view actuatedJointName)
{
    return AddJointActuator(std::unique_ptr<Actuator, ActuatorDeleter>(a.release(), Actuator::DefaultDeleter), actuatedJointName);
}

LinkActuator* FeatherstoneRobot::AddLinkActuator(std::unique_ptr<Actuator, ActuatorDeleter> a, std::string_view actuatedLinkName, const Transform& origin)
{
    if (a == nullptr || a->GetType() != ActuatorType::LINK)
    {
        cCritical("Actuator does not exist or is not a link actuator!");
        return nullptr;
    }

    int linkId = GetLinkIndex(actuatedLinkName);
    if(linkId < -1)
    {
        cCritical("Link '%s' doesn't exist. Actuator '%s' cannot be attached!", std::string(actuatedLinkName).c_str(), a->GetName().c_str());
        return nullptr;
    }
    if(static_cast<LinkActuator*>(a.get())->GetLinkActuatorType() == LinkActuatorType::SUCTION_CUP) // Special case
    {
        static_cast<SuctionCup*>(a.get())->AttachToLink(GetDynamics(), linkId);
    }
    else
    {
        static_cast<LinkActuator*>(a.get())->AttachToSolid(GetLink(actuatedLinkName), origin);
    }
    detachedActuators_.push_back(std::move(a));
    actuators_.push_back(detachedActuators_.back().get());
    return static_cast<LinkActuator*>(actuators_.back());
}

LinkActuator* FeatherstoneRobot::AddLinkActuator(std::unique_ptr<Actuator> a, std::string_view actuatedLinkName, const Transform& origin)
{
    return AddLinkActuator(std::unique_ptr<Actuator, ActuatorDeleter>(a.release(), Actuator::DefaultDeleter), actuatedLinkName, origin);
}

}