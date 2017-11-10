//
//  DVL.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 30/10/2017.
//  Copyright (c) 2017 Patryk Cieslak. All rights reserved.
//

#include "DVL.h"
#include "SimulationApp.h"
#include <BulletCollision/NarrowPhaseCollision/btRaycastCallback.h>
#include "MathsUtil.hpp"

DVL::DVL(std::string uniqueName, SolidEntity* attachment, const btTransform& geomToSensor, btScalar frequency, unsigned int historyLength) : SimpleSensor(uniqueName, geomToSensor, frequency, historyLength)
{
    attach = attachment;
    channels.push_back(SensorChannel("Velocity X", QUANTITY_VELOCITY));
    channels.push_back(SensorChannel("Velocity Y", QUANTITY_VELOCITY));
    channels.push_back(SensorChannel("Velocity Z", QUANTITY_VELOCITY));
    channels.push_back(SensorChannel("Altitude", QUANTITY_LENGTH));
    channels[3].rangeMin = btScalar(0.0);
    channels[3].rangeMax = btScalar(1000.0);
}    
    
void DVL::InternalUpdate(btScalar dt)
{
    //Check hit with bottom
    btScalar altitude(channels[3].rangeMax);
    btTransform dvlTrans = attach->getTransform() * attach->getGeomToCOGTransform().inverse() * g2s;
    btVector3 from = dvlTrans.getOrigin() - dvlTrans.getBasis().getColumn(2) * channels[3].rangeMin;
    btVector3 to = from - dvlTrans.getBasis().getColumn(2) * channels[3].rangeMax;
    
    btCollisionWorld::ClosestRayResultCallback closest(from, to);
    SimulationApp::getApp()->getSimulationManager()->getDynamicsWorld()->rayTest(from, to, closest);
                
    if(closest.hasHit())
    {
        btVector3 p = from.lerp(to, closest.m_closestHitFraction);
        altitude = (p - dvlTrans.getOrigin()).length();
    }
    
    //Get velocity
    btVector3 v = dvlTrans.getBasis().inverse() * attach->getLinearVelocity();
    
    //Record sample
    btScalar data[4] = {v.x(),v.y(),v.z(),altitude};
    Sample s(4, data);
    AddSampleToHistory(s);
}

std::vector<Renderable> DVL::Render()
{
    std::vector<Renderable> items(0);
    
    btTransform dvlTrans = attach->getTransform() * attach->getGeomToCOGTransform().inverse() * g2s;
    
    Renderable item;
    item.type = RenderableType::SENSOR_LINES;
    item.model = glMatrixFromBtTransform(dvlTrans);
    item.points.push_back(glm::vec3(0,0,0));
    item.points.push_back(glm::vec3(0,0,-getLastSample().getData()[3]));
    items.push_back(item);

    return items;
}

void DVL::SetRange(const btVector3& velocityMax, btScalar altitudeMin, btScalar altitudeMax)
{
    channels[0].rangeMin = -velocityMax.getX();
    channels[1].rangeMin = -velocityMax.getY();
    channels[2].rangeMin = -velocityMax.getZ();
    
    channels[0].rangeMax = velocityMax.getX();
    channels[1].rangeMax = velocityMax.getY();
    channels[2].rangeMax = velocityMax.getZ();
    
    channels[3].rangeMin = altitudeMin;
    channels[3].rangeMax = altitudeMax;
}

void DVL::SetNoise(btScalar velocityStdDev, btScalar altitudeStdDev)
{
    channels[0].setStdDev(velocityStdDev);
    channels[1].setStdDev(velocityStdDev);
    channels[2].setStdDev(velocityStdDev);
    channels[3].setStdDev(altitudeStdDev);
}
    
btTransform DVL::getSensorFrame()
{
    return attach->getTransform() * attach->getGeomToCOGTransform().inverse() * g2s;
}