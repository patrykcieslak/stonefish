//
//  Multibeam.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 1/08/2018.
//  Copyright (c) 2018 Patryk Cieslak. All rights reserved.
//

#include <sensors/Multibeam.h>

#include <core/SimulationApp.h>
#include <utils/MathsUtil.hpp>
#include <BulletCollision/NarrowPhaseCollision/btRaycastCallback.h>

Multibeam::Multibeam(std::string uniqueName, SolidEntity* attachment, const btTransform& geomToSensor, btScalar angleRangeDeg, unsigned int angleSteps, btScalar frequency, int historyLength) : SimpleSensor(uniqueName, geomToSensor, frequency, historyLength)
{
    attach = attachment;
    angRange = UnitSystem::Angle(true, angleRangeDeg);
    angSteps = angleSteps;
    
    for(unsigned int i=0; i <= angSteps; ++i)
    {
        angles.push_back(i/(btScalar)angSteps * angRange - btScalar(0.5) * angRange);
    
        channels.push_back(SensorChannel("Distance", QUANTITY_LENGTH));
        channels.back().rangeMin = btScalar(0);
        channels.back().rangeMax = BT_LARGE_FLOAT;
    }
    
    distances = std::vector<btScalar>(angSteps+1, btScalar(0));
}
    
void Multibeam::InternalUpdate(btScalar dt)
{
    btTransform mbTrans = attach->getTransform() * attach->getGeomToCOGTransform().inverse() * g2s;
    
    for(unsigned int i=0; i<=angSteps; ++i)
    {
        btVector3 dir = mbTrans.getBasis().getColumn(0) * btCos(angles[i]) + mbTrans.getBasis().getColumn(1) * btSin(angles[i]);
        btVector3 from = mbTrans.getOrigin() + dir * channels[1].rangeMin;
        btVector3 to = mbTrans.getOrigin() + dir * channels[1].rangeMax;
    
        btCollisionWorld::ClosestRayResultCallback closest(from, to);
        SimulationApp::getApp()->getSimulationManager()->getDynamicsWorld()->rayTest(from, to, closest);
        
        if(closest.hasHit())
        {
            btVector3 p = from.lerp(to, closest.m_closestHitFraction);
            distances[i] = (p - mbTrans.getOrigin()).length();
        }
        else
            distances[i] = channels[i].rangeMax;
    }
    
    Sample s(angSteps+1, distances.data());
    AddSampleToHistory(s);
}

std::vector<Renderable> Multibeam::Render()
{
    std::vector<Renderable> items(0);
    btTransform profTrans = attach->getTransform() * attach->getGeomToCOGTransform().inverse() * g2s;
    
    Renderable item;
    item.type = RenderableType::SENSOR_LINES;
    item.model = glMatrixFromBtTransform(profTrans);
    
    for(unsigned int i=0; i <= angSteps; ++i)
    {
        btVector3 dir = btVector3(1, 0, 0) * btCos(angles[i]) + btVector3(0, 1, 0) * btSin(angles[i]);
        item.points.push_back(glm::vec3(0,0,0));
        item.points.push_back(glm::vec3(dir.x() * distances[i], dir.y() * distances[i], dir.z() * distances[i]));
    }        
    
    items.push_back(item);
    return items;
}

void Multibeam::SetRange(btScalar distanceMin, btScalar distanceMax)
{
    for(unsigned int i=0; i<=angSteps; ++i)
    {
        channels[i].rangeMin = distanceMin;
        channels[i].rangeMax = distanceMax;
    }
}

void Multibeam::SetNoise(btScalar distanceStdDev)
{
    for(unsigned int i=0; i<=angSteps; ++i)
        channels[i].setStdDev(distanceStdDev);
}
    
btTransform Multibeam::getSensorFrame()
{
    return attach->getTransform() * attach->getGeomToCOGTransform().inverse() * g2s;
}