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
//  Multibeam.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 1/08/2018.
//  Copyright (c) 2018-2026 Patryk Cieslak. All rights reserved.
//

#include "sensors/scalar/Multibeam.h"

#include "BulletCollision/NarrowPhaseCollision/btRaycastCallback.h"
#include "core/SimulationApp.h"
#include "core/SimulationManager.h"
#include "core/DeviceFactory.h"
#include "utils/UnitSystem.h"
#include "sensors/Sample.h"
#include "graphics/OpenGLPipeline.h"

namespace sf
{

Multibeam::Multibeam(const std::string& uniqueName, Scalar angleRangeDeg, unsigned int angleSteps, Scalar frequency, int historyLength) : LinkSensor(uniqueName, frequency, historyLength)
{
    angRange_ = UnitSystem::Angle(true, angleRangeDeg);
    angSteps_ = angleSteps;
    
    for(unsigned int i=0; i <= angSteps_; ++i)
    {
        angles_.push_back(i/(Scalar)angSteps_ * angRange_ - Scalar(0.5) * angRange_);
    
        channels_.push_back(SensorChannel("Distance", QuantityType::LENGTH));
        channels_.back().rangeMin = Scalar(0);
        channels_.back().rangeMax = BT_LARGE_FLOAT;
    }
    
    distances_ = std::vector<Scalar>(angSteps_+1, Scalar(0));
}
    
void Multibeam::InternalUpdate(Scalar dt)
{
    //get sensor frame in world
    Transform mbTrans = getSensorFrame();
    
    //shoot rays
    for(unsigned int i=0; i<=angSteps_; ++i)
    {
        Vector3 dir = mbTrans.getBasis().getColumn(0) * btCos(angles_[i]) + mbTrans.getBasis().getColumn(1) * btSin(angles_[i]);
        Vector3 from = mbTrans.getOrigin() + dir * channels_[1].rangeMin;
        Vector3 to = mbTrans.getOrigin() + dir * channels_[1].rangeMax;
    
        btCollisionWorld::ClosestRayResultCallback closest(from, to);
        closest.m_collisionFilterGroup = MASK_DYNAMIC;
        closest.m_collisionFilterMask = MASK_STATIC | MASK_DYNAMIC | MASK_ANIMATED_COLLIDING;
        SimulationApp::getApp()->getSimulationManager()->getDynamicsWorld()->rayTest(from, to, closest);
        
        if(closest.hasHit())
        {
            Vector3 p = from.lerp(to, closest.m_closestHitFraction);
            distances_[i] = (p - mbTrans.getOrigin()).length();
        }
        else
            distances_[i] = channels_[i].rangeMax;
    }
    
    //record sample
    AddSampleToHistory(std::make_unique<Sample>(distances_));
}

std::vector<Renderable> Multibeam::Render()
{
    std::vector<Renderable> items = Sensor::Render();
    if(isRenderable())
    {
        Renderable item;
        item.type = RenderableType::SENSOR_LINES;
        item.model = glMatrixFromTransform(getSensorFrame());    
        item.data = std::make_shared<std::vector<glm::vec3>>();
        auto points = item.getDataAsPoints();
        for(unsigned int i=0; i <= angSteps_; ++i)
        {
            Vector3 dir = Vector3(1, 0, 0) * btCos(angles_[i]) + Vector3(0, 1, 0) * btSin(angles_[i]);
            points->push_back(glm::vec3(0,0,0));
            points->push_back(glm::vec3(dir.x() * distances_[i], dir.y() * distances_[i], dir.z() * distances_[i]));
        }        
        items.push_back(item);
    }
    return items;
}

void Multibeam::setRange(Scalar rangeMin, Scalar rangeMax)
{
    btClamp(rangeMin, Scalar(0), Scalar(BT_LARGE_FLOAT));
    btClamp(rangeMax, Scalar(0), Scalar(BT_LARGE_FLOAT)); 

    for(unsigned int i=0; i<=angSteps_; ++i)
    {
        channels_[i].rangeMin = rangeMin;
        channels_[i].rangeMax = rangeMax;
    }
}

void Multibeam::setNoise(Scalar rangeStdDev)
{
    btClamp(rangeStdDev, Scalar(0), Scalar(BT_LARGE_FLOAT));

    for(unsigned int i=0; i<=angSteps_; ++i)
        channels_[i].setStdDev(rangeStdDev);
}

ScalarSensorType Multibeam::getScalarSensorType() const
{
    return ScalarSensorType::MULTIBEAM;
}

Scalar Multibeam::getAngleRange() const
{
    return angRange_;
}

// Statics

ConstructInfo Multibeam::getConstructInfo()
{
    ConstructInfo info;
    ConstructInfoNode node;
    
    // History
    node.optional = true;
    node.attributes.insert({"samples", {ConstructInfoValueType::INT, false}});
    info.nodes.insert({"history", node});

    // Specs
    node.attributes.clear();
    node.optional = false;
    node.attributes.insert({"fov", {ConstructInfoValueType::SCALAR, false}});
    node.attributes.insert({"steps", {ConstructInfoValueType::INT, false}});
    info.nodes.insert({"specs", node});

    // Range
    node.attributes.clear();
    node.optional = true;
    node.attributes.insert({"distance_min", {ConstructInfoValueType::SCALAR, false}});
    node.attributes.insert({"distance_max", {ConstructInfoValueType::SCALAR, false}});
    info.nodes.insert({"range", node});

    // Noise
    node.attributes.clear();
    node.optional = true;
    node.attributes.insert({"distance", {ConstructInfoValueType::SCALAR, false}});
    info.nodes.insert({"noise", node});

    return info;
}

std::unique_ptr<Multibeam> Multibeam::Construct(const std::string& uniqueName, Scalar frequency, ConstructInfo& info)
{
    // History (optional)
    int history = -1;
    ConstructInfoValue& value = info.nodes.at("history").attributes.at("samples");
    if (value.valid)
        history = std::get<int>(value.value);
    
    // Specs
    Scalar fov = std::get<Scalar>(info.nodes.at("specs").attributes.at("fov").value);
    int steps = std::get<int>(info.nodes.at("specs").attributes.at("steps").value);

    // Create sensor
    std::unique_ptr<Multibeam> sensor = std::make_unique<Multibeam>(uniqueName, fov, steps, frequency, history);    

    // Range (optional)
    Scalar distanceMin (0.);
    Scalar distanceMax (BT_LARGE_FLOAT);
    
    value = info.nodes.at("range").attributes.at("distance_min");
    if (value.valid)
        distanceMin = std::get<Scalar>(value.value);
    
    value = info.nodes.at("range").attributes.at("distance_max");
    if (value.valid)
        distanceMax = std::get<Scalar>(value.value);

    sensor->setRange(distanceMin, distanceMax);

    // Noise (optional)
    value = info.nodes.at("noise").attributes.at("distance");
    if (value.valid)
        sensor->setNoise(std::get<Scalar>(value.value));

    return sensor;
}

REGISTER_SENSOR("multibeam", Multibeam)

}
