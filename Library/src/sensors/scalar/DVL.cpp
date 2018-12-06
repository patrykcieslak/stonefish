//
//  DVL.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 30/10/2017.
//  Copyright (c) 2017-2018 Patryk Cieslak. All rights reserved.
//

#include "sensors/scalar/DVL.h"

#include <BulletCollision/NarrowPhaseCollision/btRaycastCallback.h>
#include "core/SimulationApp.h"
#include "core/SimulationManager.h"
#include "entities/SolidEntity.h"
#include "sensors/Sample.h"
#include "graphics/OpenGLPipeline.h"

namespace sf
{

DVL::DVL(std::string uniqueName, Scalar beamSpreadAngle, Scalar frequency, int historyLength) : LinkSensor(uniqueName, frequency, historyLength)
{
    range[0] = range[1] = range[2] = range[3] = Scalar(0.);
    beamAngle = beamSpreadAngle;
    channels.push_back(SensorChannel("Velocity X", QUANTITY_VELOCITY));
    channels.push_back(SensorChannel("Velocity Y", QUANTITY_VELOCITY));
    channels.push_back(SensorChannel("Velocity Z", QUANTITY_VELOCITY));
    channels.push_back(SensorChannel("Altitude", QUANTITY_LENGTH));
    channels[3].rangeMin = Scalar(0.0);
    channels[3].rangeMax = Scalar(1000.0);
}    
    
void DVL::InternalUpdate(Scalar dt)
{
    //check hit with bottom
    Scalar minRange = channels[3].rangeMax;
    Transform dvlTrans = getSensorFrame();
    
    //simulate 4 beam DVL (typical design)
    Vector3 dir[4];
    Vector3 from[4];
    Vector3 to[4];
    dir[0] = dvlTrans.getBasis().getColumn(2) * btCos(beamAngle/Scalar(2)) + dvlTrans.getBasis().getColumn(0) * btSin(beamAngle/Scalar(2));
    dir[1] = dvlTrans.getBasis().getColumn(2) * btCos(beamAngle/Scalar(2)) - dvlTrans.getBasis().getColumn(0) * btSin(beamAngle/Scalar(2));
    dir[2] = dvlTrans.getBasis().getColumn(2) * btCos(beamAngle/Scalar(2)) + dvlTrans.getBasis().getColumn(1) * btSin(beamAngle/Scalar(2));
    dir[3] = dvlTrans.getBasis().getColumn(2) * btCos(beamAngle/Scalar(2)) - dvlTrans.getBasis().getColumn(1) * btSin(beamAngle/Scalar(2));
    
    for(unsigned int i=0; i<4; ++i)
    {
        from[i] = dvlTrans.getOrigin() - dir[i] * channels[3].rangeMin;
        to[i] = dvlTrans.getOrigin() - dir[i] * channels[3].rangeMax;
        
        btCollisionWorld::ClosestRayResultCallback closest(from[i], to[i]);
        SimulationApp::getApp()->getSimulationManager()->getDynamicsWorld()->rayTest(from[i], to[i], closest);
        
        if(closest.hasHit())
        {
            Vector3 p = from[i].lerp(to[i], closest.m_closestHitFraction);
            range[i] = (p - dvlTrans.getOrigin()).length();
        }
        else
            range[i] = channels[3].rangeMax;
            
        if(range[i] < minRange)
            minRange = range[i];
    }
   
    //get velocity
    Vector3 v = dvlTrans.getBasis().inverse() * attach->getLinearVelocityInLocalPoint(dvlTrans.getOrigin() - attach->getCGTransform().getOrigin());
    
    //record sample
    Scalar data[4] = {v.x(),v.y(),v.z(), minRange * btCos(beamAngle/Scalar(2))};
    Sample s(4, data);
    AddSampleToHistory(s);
}

std::vector<Renderable> DVL::Render()
{
    std::vector<Renderable> items(0);
    
    Vector3 dir[4];
    dir[0] = Vector3(0,0,1) * btCos(beamAngle/Scalar(2)) + Vector3(1,0,0) * btSin(beamAngle/Scalar(2));
    dir[1] = Vector3(0,0,1) * btCos(beamAngle/Scalar(2)) - Vector3(1,0,0) * btSin(beamAngle/Scalar(2));
    dir[2] = Vector3(0,0,1) * btCos(beamAngle/Scalar(2)) + Vector3(0,1,0) * btSin(beamAngle/Scalar(2));
    dir[3] = Vector3(0,0,1) * btCos(beamAngle/Scalar(2)) - Vector3(0,1,0) * btSin(beamAngle/Scalar(2));
    
    Renderable item;
    item.type = RenderableType::SENSOR_LINES;
    item.model = glMatrixFromTransform(getSensorFrame());
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

void DVL::SetRange(const Vector3& velocityMax, Scalar altitudeMin, Scalar altitudeMax)
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

void DVL::SetNoise(Scalar velocityStdDev, Scalar altitudeStdDev)
{
    channels[0].setStdDev(velocityStdDev);
    channels[1].setStdDev(velocityStdDev);
    channels[2].setStdDev(velocityStdDev);
    channels[3].setStdDev(altitudeStdDev);
}

}
