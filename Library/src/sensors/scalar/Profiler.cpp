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
//  Profiler.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 31/07/2018.
//  Copyright (c) 2018-2026 Patryk Cieslak. All rights reserved.
//

#include "sensors/scalar/Profiler.h"

#include "BulletCollision/NarrowPhaseCollision/btRaycastCallback.h"
#include "core/SimulationApp.h"
#include "core/SimulationManager.h"
#include "core/DeviceFactory.h"
#include "utils/UnitSystem.h"
#include "sensors/Sample.h"
#include "graphics/OpenGLContent.h"

namespace sf
{

Profiler::Profiler(const std::string& uniqueName, Scalar angleRangeDeg, unsigned int angleSteps, Scalar frequency, int historyLength) : LinkSensor(uniqueName, frequency, historyLength)
{
    angRange_ = UnitSystem::Angle(true, angleRangeDeg);
    angSteps_ = angleSteps;
    channels_.push_back(SensorChannel("Angle", QuantityType::ANGLE));
    channels_.push_back(SensorChannel("Distance", QuantityType::LENGTH));
    channels_[1].rangeMin = Scalar(0);
    channels_[1].rangeMax = BT_LARGE_FLOAT;
    currentAngStep_ = 0;
    distance_ = 0;
    clockwise_ = true;
}
    
void Profiler::InternalUpdate(Scalar dt)
{
    Transform profTrans = getSensorFrame();
    Scalar currentAngle = currentAngStep_/(Scalar)angSteps_ * angRange_ - Scalar(0.5) * angRange_;
    
    //Simulate 1 beam rotating profiler
    Vector3 dir = profTrans.getBasis().getColumn(0) * btCos(currentAngle) + profTrans.getBasis().getColumn(1) * btSin(currentAngle);
    Vector3 from = profTrans.getOrigin() + dir * channels_[1].rangeMin;
    Vector3 to = profTrans.getOrigin() + dir * channels_[1].rangeMax;
    
    btCollisionWorld::ClosestRayResultCallback closest(from, to);
    closest.m_collisionFilterGroup = MASK_DYNAMIC;
    closest.m_collisionFilterMask = MASK_STATIC | MASK_DYNAMIC | MASK_ANIMATED_COLLIDING;
    SimulationApp::getApp()->getSimulationManager()->getDynamicsWorld()->rayTest(from, to, closest);
        
    if(closest.hasHit())
    {
        Vector3 p = from.lerp(to, closest.m_closestHitFraction);
        distance_ = (p - profTrans.getOrigin()).length();
    }
    else
        distance_ = channels_[1].rangeMax;
   
    //Record sample
    AddSampleToHistory(std::make_unique<Sample>(
        std::vector<Scalar>({currentAngle, distance_})
    ));
    
    //Rotate beam
    if(clockwise_)
    {
        if(currentAngStep_ == angSteps_)
        {
            --currentAngStep_;
            clockwise_ = false;
        }
        else
            ++currentAngStep_;
    }
    else
    {
        if(currentAngStep_ == 0)
        {
            ++currentAngStep_;
            clockwise_ = true;
        }
        else
            --currentAngStep_;
    }
}

std::vector<Renderable> Profiler::Render()
{
    std::vector<Renderable> items = Sensor::Render();
    if(isRenderable())
    {
        Scalar currentAngle = currentAngStep_/(Scalar)angSteps_ * angRange_ - Scalar(0.5) * angRange_;
        Vector3 dir = Vector3(1, 0, 0) * btCos(currentAngle) + Vector3(0, 1, 0) * btSin(currentAngle);
        
        Renderable item;
        item.type = RenderableType::SENSOR_LINES;
        item.model = glMatrixFromTransform(getSensorFrame());
        item.data = std::make_shared<std::vector<glm::vec3>>();
        auto points = item.getDataAsPoints();
        points->push_back(glm::vec3(0,0,0));
        points->push_back(glm::vec3(dir.x()*distance_, dir.y()*distance_, dir.z()*distance_));
        items.push_back(item);
    }
    return items;
}

void Profiler::setRange(Scalar rangeMin, Scalar rangeMax)
{
    channels_[1].rangeMin = btClamped(rangeMin, Scalar(0), Scalar(BT_LARGE_FLOAT));
    channels_[1].rangeMax = btClamped(rangeMax, Scalar(0), Scalar(BT_LARGE_FLOAT));
}

void Profiler::setNoise(Scalar rangeStdDev)
{
    channels_[1].setStdDev(btClamped(rangeStdDev, Scalar(0), Scalar(BT_LARGE_FLOAT)));
}

ScalarSensorType Profiler::getScalarSensorType() const
{
    return ScalarSensorType::PROFILER;
}

// Statics

ConstructInfo Profiler::getConstructInfo()
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

std::unique_ptr<Profiler> Profiler::Construct(const std::string& uniqueName, Scalar frequency, ConstructInfo& info)
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
    std::unique_ptr<Profiler> sensor = std::make_unique<Profiler>(uniqueName, fov, steps, frequency, history);    

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

REGISTER_SENSOR("profiler", Profiler)

}
