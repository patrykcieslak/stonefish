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
//  VisionSensor.cpp
//  Stonefish
//
//  Created by Patryk Cieślak on 21/11/2018.
//  Copyright (c) 2018-2025 Patryk Cieslak. All rights reserved.
//

#include "sensors/VisionSensor.h"

#include "core/SimulationApp.h"
#include "core/SimulationManager.h"
#include "entities/StaticEntity.h"
#include "entities/SolidEntity.h"

namespace sf
{

VisionSensor::VisionSensor(std::string uniqueName, Scalar frequency) : Sensor(uniqueName, frequency)
{
    if(!SimulationApp::getApp()->hasGraphics())
        cCritical("Not possible to use vision sensors in console simulation! Use graphical simulation if possible.");
    
    attach = nullptr;
    o2s = Transform::getIdentity();
}

VisionSensor::~VisionSensor()
{
}

void VisionSensor::setRelativeSensorFrame(const Transform& origin)
{
    o2s = origin;
}

Transform VisionSensor::getSensorFrame() const
{
    if(attach != nullptr)
    {
        if(attach->getType() == EntityType::STATIC)
            return ((StaticEntity*)attach)->getTransform() * o2s;
        else
            return ((MovingEntity*)attach)->getOTransform() * o2s;
    }
    else
        return o2s;
}

void VisionSensor::getSensorVelocity(Vector3& linear, Vector3& angular) const
{
    if(attach != nullptr && attach->getType() != EntityType::STATIC)
    {
        linear = ((MovingEntity*)attach)->getLinearVelocity();
        angular = ((MovingEntity*)attach)->getAngularVelocity();
    }
    else 
    {
        linear = V0();
        angular = V0();
    }
}

SensorType VisionSensor::getType() const
{
    return SensorType::VISION;
}

void VisionSensor::AttachToWorld(const Transform& origin)
{
    attach = nullptr;
    o2s = origin;
    InitGraphics();
    if(SimulationApp::getApp()->getSimulationManager()->isOceanEnabled())
            SimulationApp::getApp()->getSimulationManager()->getOcean()->getOpenGLOcean()->AllocateParticles(getOpenGLView());
}

void VisionSensor::AttachToStatic(StaticEntity* body, const Transform& origin)
{
    if(body != nullptr)
    {
        attach = body;
        o2s = origin;
        InitGraphics();
        if(SimulationApp::getApp()->getSimulationManager()->isOceanEnabled())
            SimulationApp::getApp()->getSimulationManager()->getOcean()->getOpenGLOcean()->AllocateParticles(getOpenGLView());
    }
}

void VisionSensor::AttachToSolid(MovingEntity* body, const Transform& origin)
{
    if(body != nullptr)
    {
        attach = body;
        o2s = origin;
        InitGraphics();
        if(SimulationApp::getApp()->getSimulationManager()->isOceanEnabled())
            SimulationApp::getApp()->getSimulationManager()->getOcean()->getOpenGLOcean()->AssignParticles(getOpenGLView(), body->getOceanParticles());
    }
}

}
