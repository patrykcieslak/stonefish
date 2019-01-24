//
//  Profiler.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 31/07/2018.
//  Copyright (c) 2018-2019 Patryk Cieslak. All rights reserved.
//

#include "sensors/scalar/Profiler.h"

#include <BulletCollision/NarrowPhaseCollision/btRaycastCallback.h>
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
    channels.push_back(SensorChannel("Angle", QUANTITY_ANGLE));
    channels.push_back(SensorChannel("Distance", QUANTITY_LENGTH));
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
    std::vector<Renderable> items(0);
    
    Scalar currentAngle = currentAngStep/(Scalar)angSteps * angRange - Scalar(0.5) * angRange;
    Vector3 dir = Vector3(1, 0, 0) * btCos(currentAngle) + Vector3(0, 1, 0) * btSin(currentAngle);
    
    Renderable item;
    item.type = RenderableType::SENSOR_LINES;
    item.model = glMatrixFromTransform(getSensorFrame());
    item.points.push_back(glm::vec3(0,0,0));
    item.points.push_back(glm::vec3(dir.x()*distance, dir.y()*distance, dir.z()*distance));
    items.push_back(item);

    return items;
}

void Profiler::SetRange(Scalar rangeMin, Scalar rangeMax)
{
    channels[1].rangeMin = rangeMin;
    channels[1].rangeMax = rangeMax;
}

void Profiler::SetNoise(Scalar stdDev)
{
    channels[1].setStdDev(stdDev);
}

}
