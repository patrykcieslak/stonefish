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
//  Copyright (c) 2018-2021 Patryk Cieslak. All rights reserved.
//

#include "sensors/scalar/Profiler.h"

#include "BulletCollision/NarrowPhaseCollision/btRaycastCallback.h"
#include "core/SimulationApp.h"
#include "core/SimulationManager.h"
#include "utils/UnitSystem.h"
#include "sensors/Sample.h"
#include "graphics/OpenGLContent.h"

namespace sf
{

Profiler::Profiler(std::string uniqueName, Scalar angleRangeDeg, unsigned int angleSteps, Scalar frequency, int historyLength) : LinkSensor(uniqueName, frequency, historyLength)
{
    angRange = UnitSystem::Angle(true, angleRangeDeg);
    angSteps = angleSteps;
    channels.push_back(SensorChannel("Angle", QuantityType::ANGLE));
    channels.push_back(SensorChannel("Distance", QuantityType::LENGTH));
    channels[1].rangeMin = Scalar(0);
    channels[1].rangeMax = BT_LARGE_FLOAT;
    currentAngStep = 0;
    distance = 0;
    clockwise = true;
}
    
void Profiler::InternalUpdate(Scalar dt)
{
    Transform profTrans = getSensorFrame();
    Scalar currentAngle = currentAngStep/(Scalar)angSteps * angRange - Scalar(0.5) * angRange;
    
    //Simulate 1 beam rotating profiler
    Vector3 dir = profTrans.getBasis().getColumn(0) * btCos(currentAngle) + profTrans.getBasis().getColumn(1) * btSin(currentAngle);
    Vector3 from = profTrans.getOrigin() + dir * channels[1].rangeMin;
    Vector3 to = profTrans.getOrigin() + dir * channels[1].rangeMax;
    
    btCollisionWorld::ClosestRayResultCallback closest(from, to);
    closest.m_collisionFilterGroup = MASK_DYNAMIC;
    closest.m_collisionFilterMask = MASK_STATIC | MASK_DYNAMIC | MASK_ANIMATED_COLLIDING;
    SimulationApp::getApp()->getSimulationManager()->getDynamicsWorld()->rayTest(from, to, closest);
        
    if(closest.hasHit())
    {
        Vector3 p = from.lerp(to, closest.m_closestHitFraction);
        distance = (p - profTrans.getOrigin()).length();
    }
    else
        distance = channels[1].rangeMax;
   
    //Record sample
    Scalar data[2] = {currentAngle, distance};
    Sample s(2, data);
    AddSampleToHistory(s);
    
    //Rotate beam
    if(clockwise)
    {
        if(currentAngStep == angSteps)
        {
            --currentAngStep;
            clockwise = false;
        }
        else
            ++currentAngStep;
    }
    else
    {
        if(currentAngStep == 0)
        {
            ++currentAngStep;
            clockwise = true;
        }
        else
            --currentAngStep;
    }
}

std::vector<Renderable> Profiler::Render()
{
    std::vector<Renderable> items = Sensor::Render();
    if(isRenderable())
    {
        Scalar currentAngle = currentAngStep/(Scalar)angSteps * angRange - Scalar(0.5) * angRange;
        Vector3 dir = Vector3(1, 0, 0) * btCos(currentAngle) + Vector3(0, 1, 0) * btSin(currentAngle);
        
        Renderable item;
        item.type = RenderableType::SENSOR_LINES;
        item.model = glMatrixFromTransform(getSensorFrame());
        item.points.push_back(glm::vec3(0,0,0));
        item.points.push_back(glm::vec3(dir.x()*distance, dir.y()*distance, dir.z()*distance));
        items.push_back(item);
    }
    return items;
}

void Profiler::setRange(Scalar rangeMin, Scalar rangeMax)
{
    channels[1].rangeMin = btClamped(rangeMin, Scalar(0), Scalar(BT_LARGE_FLOAT));
    channels[1].rangeMax = btClamped(rangeMax, Scalar(0), Scalar(BT_LARGE_FLOAT));
}

void Profiler::setNoise(Scalar rangeStdDev)
{
    channels[1].setStdDev(btClamped(rangeStdDev, Scalar(0), Scalar(BT_LARGE_FLOAT)));
}

ScalarSensorType Profiler::getScalarSensorType()
{
    return ScalarSensorType::PROFILER;
}

}
