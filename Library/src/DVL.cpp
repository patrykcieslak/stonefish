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

DVL::DVL(std::string uniqueName, SolidEntity* attachment, const btTransform& geomToSensor, btScalar frequency, unsigned int historyLength) : SimpleSensor(uniqueName, frequency, historyLength)
{
    attach = attachment;
    g2s = UnitSystem::SetTransform(geomToSensor);
    channels.push_back(SensorChannel("Velocity X", QUANTITY_VELOCITY));
    channels.push_back(SensorChannel("Velocity Y", QUANTITY_VELOCITY));
    channels.push_back(SensorChannel("Velocity Z", QUANTITY_VELOCITY));
    channels.push_back(SensorChannel("Altitude", QUANTITY_LENGTH));
}    
    
void DVL::InternalUpdate(btScalar dt)
{
    //Check hit with bottom
    btScalar altitude(-1);
    btTransform dvlTrans = attach->getTransform() * attach->getGeomToCOGTransform().inverse() * g2s;
    btVector3 from = dvlTrans.getOrigin();
    btVector3 to = from + dvlTrans.getBasis().getColumn(2) * btScalar(10000);
    
    btCollisionWorld::ClosestRayResultCallback closest(from, to);
    SimulationApp::getApp()->getSimulationManager()->getDynamicsWorld()->rayTest(from, to, closest);
                
    if(closest.hasHit())
    {
        btVector3 p = from.lerp(to, closest.m_closestHitFraction);
        altitude = (p-from).length();
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
    item.type = RenderableType::SENSOR;
    item.model = glMatrixFromBtTransform(dvlTrans);
    item.points.push_back(glm::vec3(0,0,0));
    item.points.push_back(glm::vec3(0,0, getLastSample().getData()[3]));
    items.push_back(item);

    return items;
}