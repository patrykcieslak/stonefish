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
//  Copyright(c) 2018-2022 Patryk Cieslak. All rights reserved.
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
#include <btBulletDynamicsCommon.h>
#include <BulletSoftBody/btSoftBodyHelpers.h>
#include <BulletSoftBody/btSoftRigidDynamicsWorld.h>
#include <BulletDynamics/ConstraintSolver/btSequentialImpulseConstraintSolver.h>
#include <iostream>


namespace sf
{

Robot::Robot(std::string uniqueName, bool fixedBase)
{
    name = SimulationApp::getApp()->getSimulationManager()->getNameManager()->AddName(uniqueName);
    fixed = fixedBase;
}

Robot::~Robot()
{
    if(SimulationApp::getApp() != nullptr)
        SimulationApp::getApp()->getSimulationManager()->getNameManager()->RemoveName(name);
}

std::string Robot::getName()
{
    return name;
}

SolidEntity* Robot::getLink(const std::string& name)
{
    for(size_t i=0; i<links.size(); ++i){
        std::cout<<"LINK NAME: "<< links[i]->getName()<<std::endl;
        if(links[i]->getName() == name) return links[i];
    }
    
    for(size_t i=0; i<detachedLinks.size(); ++i)
        if(detachedLinks[i]->getName() == name) return detachedLinks[i];
    
    return nullptr;
}

SolidEntity* Robot::getLink(size_t index)
{
    if(index < links.size())
        return links[index];
    else
        return nullptr;
}
    
Actuator* Robot::getActuator(std::string name)
{
    for(size_t i=0; i<actuators.size(); ++i)
        if(actuators[i]->getName() == name)
            return actuators[i];

    return nullptr;
}

Actuator* Robot::getActuator(size_t index)
{
    if(index < actuators.size())
        return actuators[index];
    else
        return nullptr;
}
    
Sensor* Robot::getSensor(std::string name)
{
    for(size_t i=0; i<sensors.size(); ++i)
        if(sensors[i]->getName() == name)
            return sensors[i];
    
    return nullptr;
}

Sensor* Robot::getSensor(size_t index)
{
    if(index < sensors.size())
        return sensors[index];
    else
        return nullptr;
}

Comm* Robot::getComm(std::string name)
{
    for(size_t i=0; i<comms.size(); ++i)
        if(comms[i]->getName() == name)
            return comms[i];
    
    return nullptr;
}

Comm* Robot::getComm(size_t index)
{
    if(index < comms.size())
        return comms[index];
    else
        return nullptr;
}

SolidEntity* Robot::getBaseLink()
{
    return links[0];
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
    jointsData.push_back(jd);
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
    jointsData.push_back(jd);
}

void Robot::DefineFixedJoint(std::string jointName, std::string parentName, std::string childName, const Transform& origin)
{
    JointData jd;
    jd.jtype = JointType::FIXED;
    jd.name = jointName;
    jd.parent = parentName;
    jd.child = childName;
    jd.origin = origin;
    Scalar x = origin.getOrigin()[0];
    Scalar y = origin.getOrigin()[1];
    Scalar z = origin.getOrigin()[2];
    std::cout<<"origin: " << x <<" " << y  << "  " << "  " << z << "  "<< parentName << "  " << childName << std::endl;
    jointsData.push_back(jd);
}

void Robot::AddLinkSensor(LinkSensor* s, const std::string& monitoredLinkName, const Transform& origin)
{
    SolidEntity* link = getLink(monitoredLinkName);
    if(link != nullptr)
    {
        s->AttachToSolid(link, origin);
        sensors.push_back(s);
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
        sensors.push_back(s);
    }
    else
        cCritical("Link '%s' doesn't exist. Sensor '%s' cannot be attached!", attachmentLinkName.c_str(), s->getName().c_str());
}

void Robot::AddLinkActuator(LinkActuator* a, const std::string& actuatedLinkName, const Transform& origin)
{
    SolidEntity* link = getLink(actuatedLinkName);
    if(link != nullptr)
    {
        a->AttachToSolid(link, origin);
        actuators.push_back(a);
    }
    else
        cCritical("Link '%s' doesn't exist. Actuator '%s' cannot be attached!", actuatedLinkName.c_str(), a->getName().c_str());
}

void Robot::AddComm(Comm* c, const std::string& attachmentLinkName, const Transform& origin)
{
    SolidEntity* link = getLink(attachmentLinkName);
    if(link != nullptr)
    {
        c->AttachToSolid(link, origin);
        comms.push_back(c);
    }
    else
        cCritical("Link '%s' doesn't exist. Communication device '%s' cannot be attached!", attachmentLinkName.c_str(), c->getName().c_str());
}

void Robot::AddToSimulation(SimulationManager* sm, const Transform& origin)
{
    for(size_t i=0; i<sensors.size(); ++i)
        sm->AddSensor(sensors[i]);
    for(size_t i=0; i<actuators.size(); ++i)
        sm->AddActuator(actuators[i]);
    for(size_t i=0; i<comms.size(); ++i)
        sm->AddComm(comms[i]);
        // Simulate underwater rope
   /* // Create a chain of rigid bodies as the rope
    // Parameters for the rope
   int numLinks = 10;  // Adjust as needed
   btScalar linkLength = 1.0;  // Adjust as needed
   btRigidBody* previousBody = nullptr;
   for (int i = 0; i < numLinks; ++i) {
        btVector3 segmentPosition(0, i * linkLength, 0);
        btCollisionShape* shape = new btCapsuleShape(0.1, 1.0); // Adjust the shape as needed
        btTransform startTransform;
        startTransform.setIdentity();
        startTransform.setOrigin(segmentPosition);
        btScalar mass = 1.0; // Adjust the mass as needed
        btVector3 localInertia(0, 0, 0);
        shape->calculateLocalInertia(mass, localInertia);
        btDefaultMotionState* motionState = new btDefaultMotionState(startTransform);
        btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionState, shape, localInertia);
        btRigidBody* currentBody = new btRigidBody(rbInfo);
        btVector3 pivotA(0, 0, 0);  // Pivot point on current body in local coordinates
        btVector3 pivotB(0, 0, -linkLength);  // Pivot point on previous body in local coordinates
        btPoint2PointConstraint* p2pConstraint = new btPoint2PointConstraint(*currentBody, pivotA);
        if (previousBody) {
          p2pConstraint->setPivotB(pivotB);
          sm->getDynamicsWorld()->addConstraint(p2pConstraint, true);
        }
       sm->getDynamicsWorld()->addRigidBody(currentBody);
       previousBody = currentBody;
    }
    std::cout << "ADDED CHAIN OF RIGID BODIES" << std::endl;
    */
}

}
