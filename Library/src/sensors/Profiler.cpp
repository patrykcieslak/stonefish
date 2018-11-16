//
//  Profiler.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 31/07/2018.
//  Copyright (c) 2018 Patryk Cieslak. All rights reserved.
//

#include "sensors/Profiler.h"

#include <BulletCollision/NarrowPhaseCollision/btRaycastCallback.h>
#include "core/SimulationApp.h"
#include "utils/MathsUtil.hpp"

Profiler::Profiler(std::string uniqueName, SolidEntity* attachment, const btTransform& geomToSensor, btScalar angleRangeDeg, unsigned int angleSteps, btScalar frequency, int historyLength) : SimpleSensor(uniqueName, geomToSensor, frequency, historyLength)
{
    attach = attachment;
    angRange = UnitSystem::Angle(true, angleRangeDeg);
    angSteps = angleSteps;
    channels.push_back(SensorChannel("Angle", QUANTITY_ANGLE));
    channels.push_back(SensorChannel("Distance", QUANTITY_LENGTH));
    channels[1].rangeMin = btScalar(0);
    channels[1].rangeMax = BT_LARGE_FLOAT;
    currentAngStep = 0;
    distance = 0;
    clockwise = true;
}
    
void Profiler::InternalUpdate(btScalar dt)
{
    btTransform profTrans = attach->getTransform() * attach->getGeomToCOGTransform().inverse() * g2s;
    btScalar currentAngle = currentAngStep/(btScalar)angSteps * angRange - btScalar(0.5) * angRange;
    
    //Simulate 1 beam rotating profiler
    btVector3 dir = profTrans.getBasis().getColumn(0) * btCos(currentAngle) + profTrans.getBasis().getColumn(1) * btSin(currentAngle);
    btVector3 from = profTrans.getOrigin() + dir * channels[1].rangeMin;
    btVector3 to = profTrans.getOrigin() + dir * channels[1].rangeMax;
    
    btCollisionWorld::ClosestRayResultCallback closest(from, to);
    SimulationApp::getApp()->getSimulationManager()->getDynamicsWorld()->rayTest(from, to, closest);
        
    if(closest.hasHit())
    {
        btVector3 p = from.lerp(to, closest.m_closestHitFraction);
        distance = (p - profTrans.getOrigin()).length();
    }
    else
        distance = channels[1].rangeMax;
   
    //Record sample
    btScalar data[2] = {currentAngle, distance};
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
    
    btTransform profTrans = attach->getTransform() * attach->getGeomToCOGTransform().inverse() * g2s;
    
    btScalar currentAngle = currentAngStep/(btScalar)angSteps * angRange - btScalar(0.5) * angRange;
    btVector3 dir = btVector3(1, 0, 0) * btCos(currentAngle) + btVector3(0, 1, 0) * btSin(currentAngle);
    
    Renderable item;
    item.type = RenderableType::SENSOR_LINES;
    item.model = glMatrixFromBtTransform(profTrans);
    item.points.push_back(glm::vec3(0,0,0));
    item.points.push_back(glm::vec3(dir.x()*distance, dir.y()*distance, dir.z()*distance));
    items.push_back(item);

    return items;
}

void Profiler::SetRange(btScalar distanceMin, btScalar distanceMax)
{
    channels[1].rangeMin = distanceMin;
    channels[1].rangeMax = distanceMax;
}

void Profiler::SetNoise(btScalar distanceStdDev)
{
    channels[1].setStdDev(distanceStdDev);
}
    
btTransform Profiler::getSensorFrame()
{
    return attach->getTransform() * attach->getGeomToCOGTransform().inverse() * g2s;
}
