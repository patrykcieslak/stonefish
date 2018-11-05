//
//  DVL.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 30/10/2017.
//  Copyright (c) 2017 Patryk Cieslak. All rights reserved.
//

#include <sensors/DVL.h>

#include <core/SimulationApp.h>
#include <utils/MathsUtil.hpp>
#include <BulletCollision/NarrowPhaseCollision/btRaycastCallback.h>

DVL::DVL(std::string uniqueName, SolidEntity* attachment, const btTransform& geomToSensor, btScalar beamSpreadAngle, btScalar frequency, int historyLength) : SimpleSensor(uniqueName, geomToSensor, frequency, historyLength)
{
    attach = attachment;
    range[0] = range[1] = range[2] = range[3] = btScalar(0.);
    beamAngle = UnitSystem::SetAngle(beamSpreadAngle);
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
    btScalar minRange = channels[3].rangeMax;
    btTransform dvlTrans = attach->getTransform() * attach->getGeomToCOGTransform().inverse() * g2s;
    
    //Simulate 4 beam DVL (typical design)
    btVector3 dir[4];
    btVector3 from[4];
    btVector3 to[4];
    dir[0] = dvlTrans.getBasis().getColumn(2) * btCos(beamAngle/btScalar(2)) + dvlTrans.getBasis().getColumn(0) * btSin(beamAngle/btScalar(2));
    dir[1] = dvlTrans.getBasis().getColumn(2) * btCos(beamAngle/btScalar(2)) - dvlTrans.getBasis().getColumn(0) * btSin(beamAngle/btScalar(2));
    dir[2] = dvlTrans.getBasis().getColumn(2) * btCos(beamAngle/btScalar(2)) + dvlTrans.getBasis().getColumn(1) * btSin(beamAngle/btScalar(2));
    dir[3] = dvlTrans.getBasis().getColumn(2) * btCos(beamAngle/btScalar(2)) - dvlTrans.getBasis().getColumn(1) * btSin(beamAngle/btScalar(2));
    
    for(unsigned int i=0; i<4; ++i)
    {
        from[i] = dvlTrans.getOrigin() - dir[i] * channels[3].rangeMin;
        to[i] = dvlTrans.getOrigin() - dir[i] * channels[3].rangeMax;
        
        btCollisionWorld::ClosestRayResultCallback closest(from[i], to[i]);
        SimulationApp::getApp()->getSimulationManager()->getDynamicsWorld()->rayTest(from[i], to[i], closest);
        
        if(closest.hasHit())
        {
            btVector3 p = from[i].lerp(to[i], closest.m_closestHitFraction);
            range[i] = (p - dvlTrans.getOrigin()).length();
        }
        else
            range[i] = channels[3].rangeMax;
            
        if(range[i] < minRange)
            minRange = range[i];
    }
   
    //Get velocity
    btVector3 v = dvlTrans.getBasis().inverse() * attach->getLinearVelocityInLocalPoint(dvlTrans.getOrigin() - attach->getTransform().getOrigin());
    
    //Record sample
    btScalar data[4] = {v.x(),v.y(),v.z(), minRange * btCos(beamAngle/btScalar(2))};
    Sample s(4, data);
    AddSampleToHistory(s);
}

std::vector<Renderable> DVL::Render()
{
    std::vector<Renderable> items(0);
    
    btTransform dvlTrans = attach->getTransform() * attach->getGeomToCOGTransform().inverse() * g2s;
    
    btVector3 dir[4];
    dir[0] = btVector3(0,0,1) * btCos(beamAngle/btScalar(2)) + btVector3(1,0,0) * btSin(beamAngle/btScalar(2));
    dir[1] = btVector3(0,0,1) * btCos(beamAngle/btScalar(2)) - btVector3(1,0,0) * btSin(beamAngle/btScalar(2));
    dir[2] = btVector3(0,0,1) * btCos(beamAngle/btScalar(2)) + btVector3(0,1,0) * btSin(beamAngle/btScalar(2));
    dir[3] = btVector3(0,0,1) * btCos(beamAngle/btScalar(2)) - btVector3(0,1,0) * btSin(beamAngle/btScalar(2));
    
    Renderable item;
    item.type = RenderableType::SENSOR_LINES;
    item.model = glMatrixFromBtTransform(dvlTrans);
    item.points.push_back(glm::vec3(0,0,0));
    item.points.push_back(glm::vec3(-dir[0].x()*range[0], -dir[0].y()*range[0], -dir[0].z()*range[0]));
    item.points.push_back(glm::vec3(0,0,0));
    item.points.push_back(glm::vec3(-dir[1].x()*range[1], -dir[1].y()*range[1], -dir[1].z()*range[1]));
    item.points.push_back(glm::vec3(0,0,0));
    item.points.push_back(glm::vec3(-dir[2].x()*range[2], -dir[2].y()*range[2], -dir[2].z()*range[2]));
    item.points.push_back(glm::vec3(0,0,0));
    item.points.push_back(glm::vec3(-dir[3].x()*range[3], -dir[3].y()*range[3], -dir[3].z()*range[3]));
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