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
//  Thruster.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 07/04/2024.
//  Copyright (c) 2024 Patryk Cieslak. All rights reserved.
//

#include "actuators/SimpleThruster.h"

#include "core/SimulationApp.h"
#include "core/SimulationManager.h"
#include "graphics/GLSLShader.h"
#include "graphics/OpenGLContent.h"
#include "entities/SolidEntity.h"

namespace sf
{

SimpleThruster::SimpleThruster(std::string uniqueName, SolidEntity* propeller, bool rightHand, bool inverted) : LinkActuator(uniqueName)
{
    RH = rightHand;
    inv = inverted;
    theta = Scalar(0);
    thrust = Scalar(0);
    torque = Scalar(0);
    setThrustLimits(1, -1); // No limits
    
    prop = propeller;
    prop->BuildGraphicalObject();
}

SimpleThruster::~SimpleThruster()
{
    if(prop != nullptr)
        delete prop;
}

ActuatorType SimpleThruster::getType() const
{
    return ActuatorType::SIMPLE_THRUSTER;
}

void SimpleThruster::setSetpoint(Scalar _thrust, Scalar _torque)
{
    if(limits.second > limits.first) // Limitted
        sThrust = _thrust < limits.first ? limits.first : (_thrust > limits.second ? limits.second : _thrust);
    
    sTorque = _torque;

    if(inv)
    { 
        sThrust *= Scalar(-1);
        sTorque *= Scalar(-1);
    }

    ResetWatchdog();
}

void SimpleThruster::setThrustLimits(Scalar lower, Scalar upper)
{
    limits.first = lower;
    limits.second = upper;
}

Scalar SimpleThruster::getAngle() const
{
    return theta;
}

Scalar SimpleThruster::getThrust() const
{
    return thrust;
}

Scalar SimpleThruster::getTorque() const
{
    return torque;
}

void SimpleThruster::Update(Scalar dt)
{
    Actuator::Update(dt);

    if(attach != nullptr)
    {
        //Update thruster rotation (visualization only)
        Scalar omega(0);
        if(!btFuzzyZero(sThrust))
        {
            omega = sThrust > Scalar(0) ? Scalar(2.0*M_PI) : Scalar(-2.0*M_PI);
            omega = RH ? omega : -omega; 
        }
        theta += omega * dt; //Just for animation
    
        //Get transforms
        Transform solidTrans = attach->getCGTransform();
        Transform thrustTrans = attach->getOTransform() * o2a;
        Vector3 relPos = thrustTrans.getOrigin() - solidTrans.getOrigin();
        Vector3 velocity = attach->getLinearVelocityInLocalPoint(relPos);
        
        //Calculate thrust
        Ocean* ocn = SimulationApp::getApp()->getSimulationManager()->getOcean();
        if(ocn != nullptr && ocn->IsInsideFluid(thrustTrans.getOrigin()))
        {
            thrust = sThrust;
            torque = sTorque;

            //Apply forces and torques
            Vector3 thrustV(thrust, 0, 0);
            Vector3 torqueV(torque, 0, 0);
            attach->ApplyCentralForce(thrustTrans.getBasis() * thrustV);
            attach->ApplyTorque((thrustTrans.getOrigin() - solidTrans.getOrigin()).cross(thrustTrans.getBasis() * thrustV));
            attach->ApplyTorque(thrustTrans.getBasis() * torqueV);
        }
        else
        {
            thrust = Scalar(0);
            torque = Scalar(0);
        }
    }
}

std::vector<Renderable> SimpleThruster::Render()
{
    Transform thrustTrans = Transform::getIdentity();
    if(attach != nullptr)
        thrustTrans = attach->getOTransform() * o2a;
    else
        LinkActuator::Render();
    
    //Rotate propeller
    thrustTrans *= Transform(Quaternion(0, 0, theta), Vector3(0,0,0));
    
    //Add renderable
    std::vector<Renderable> items(0);
    Renderable item;
    item.type = RenderableType::SOLID;
    item.materialName = prop->getMaterial().name;
    item.objectId = prop->getGraphicalObject();
    item.lookId = dm == DisplayMode::GRAPHICAL ? prop->getLook() : -1;
	item.model = glMatrixFromTransform(thrustTrans);
    items.push_back(item);
    
    item.type = RenderableType::ACTUATOR_LINES;
    item.points.push_back(glm::vec3(0,0,0));
    item.points.push_back(glm::vec3(0.1f*thrust,0,0));
    items.push_back(item);
    
    return items;
}

void SimpleThruster::WatchdogTimeout()
{
    setSetpoint(Scalar(0), Scalar(0));
}
    
}
