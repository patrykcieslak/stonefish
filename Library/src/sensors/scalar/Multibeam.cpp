//
//  Multibeam.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 1/08/2018.
//  Copyright (c) 2018-2019 Patryk Cieslak. All rights reserved.
//

#include "sensors/scalar/Multibeam.h"

#include <BulletCollision/NarrowPhaseCollision/btRaycastCallback.h>
#include "core/SimulationApp.h"
#include "core/SimulationManager.h"
#include "utils/UnitSystem.h"
#include "sensors/Sample.h"
#include "graphics/OpenGLPipeline.h"

namespace sf
{

Multibeam::Multibeam(std::string uniqueName, Scalar angleRangeDeg, unsigned int angleSteps, Scalar frequency, int historyLength) : LinkSensor(uniqueName, frequency, historyLength)
{
    angRange = UnitSystem::Angle(true, angleRangeDeg);
    angSteps = angleSteps;
    
    for(unsigned int i=0; i <= angSteps; ++i)
    {
        angles.push_back(i/(Scalar)angSteps * angRange - Scalar(0.5) * angRange);
    
        channels.push_back(SensorChannel("Distance", QUANTITY_LENGTH));
        channels.back().rangeMin = Scalar(0);
        channels.back().rangeMax = BT_LARGE_FLOAT;
    }
    
    distances = std::vector<Scalar>(angSteps+1, Scalar(0));
}
    
void Multibeam::InternalUpdate(Scalar dt)
{
    //get sensor frame in world
    Transform mbTrans = getSensorFrame();
    
    //shoot rays
    for(unsigned int i=0; i<=angSteps; ++i)
    {
        Vector3 dir = mbTrans.getBasis().getColumn(0) * btCos(angles[i]) + mbTrans.getBasis().getColumn(1) * btSin(angles[i]);
        Vector3 from = mbTrans.getOrigin() + dir * channels[1].rangeMin;
        Vector3 to = mbTrans.getOrigin() + dir * channels[1].rangeMax;
    
        btCollisionWorld::ClosestRayResultCallback closest(from, to);
        SimulationApp::getApp()->getSimulationManager()->getDynamicsWorld()->rayTest(from, to, closest);
        
        if(closest.hasHit())
        {
            Vector3 p = from.lerp(to, closest.m_closestHitFraction);
            distances[i] = (p - mbTrans.getOrigin()).length();
        }
        else
            distances[i] = channels[i].rangeMax;
    }
    
    //record sample
    Sample s(angSteps+1, distances.data());
    AddSampleToHistory(s);
}

std::vector<Renderable> Multibeam::Render()
{
    std::vector<Renderable> items(0);
    
    Renderable item;
    item.type = RenderableType::SENSOR_LINES;
    item.model = glMatrixFromTransform(getSensorFrame());
    
    for(unsigned int i=0; i <= angSteps; ++i)
    {
        Vector3 dir = Vector3(1, 0, 0) * btCos(angles[i]) + Vector3(0, 1, 0) * btSin(angles[i]);
        item.points.push_back(glm::vec3(0,0,0));
        item.points.push_back(glm::vec3(dir.x() * distances[i], dir.y() * distances[i], dir.z() * distances[i]));
    }        
    
    items.push_back(item);
    return items;
}

void Multibeam::SetRange(Scalar rangeMin, Scalar rangeMax)
{
    for(unsigned int i=0; i<=angSteps; ++i)
    {
        channels[i].rangeMin = rangeMin;
        channels[i].rangeMax = rangeMax;
    }
}

void Multibeam::SetNoise(Scalar stdDev)
{
    for(unsigned int i=0; i<=angSteps; ++i)
        channels[i].setStdDev(stdDev);
}

}
