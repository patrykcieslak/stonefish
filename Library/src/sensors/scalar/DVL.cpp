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
//  DVL.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 30/10/2017.
//  Copyright (c) 2017-2018 Patryk Cieslak. All rights reserved.
//

#include "sensors/scalar/DVL.h"

#include "BulletCollision/NarrowPhaseCollision/btRaycastCallback.h"
#include "core/SimulationApp.h"
#include "core/SimulationManager.h"
#include "entities/SolidEntity.h"
#include "sensors/Sample.h"
#include "graphics/OpenGLPipeline.h"

namespace sf
{

DVL::DVL(std::string uniqueName, Scalar beamSpreadAngleDeg, Scalar frequency, int historyLength) : LinkSensor(uniqueName, frequency, historyLength)
{
    range[0] = range[1] = range[2] = range[3] = Scalar(0.);
    beamAngle = beamSpreadAngleDeg/Scalar(180)*M_PI;
    channels.push_back(SensorChannel("Velocity X", QUANTITY_VELOCITY));
    channels.push_back(SensorChannel("Velocity Y", QUANTITY_VELOCITY));
    channels.push_back(SensorChannel("Velocity Z", QUANTITY_VELOCITY));
    channels.push_back(SensorChannel("Altitude", QUANTITY_LENGTH));
    channels[3].rangeMin = Scalar(0.0);
    channels[3].rangeMax = Scalar(1000.0);
}    
    
void DVL::InternalUpdate(Scalar dt)
{
    //Check hit with bottom
    Scalar minRange(-1);
    Transform dvlTrans = getSensorFrame();
    
    //Simulate 4 beam DVL (typical design)
    Vector3 dir[4];
    Vector3 from[4];
    Vector3 to[4];
    dir[0] = dvlTrans.getBasis().getColumn(2) * btCos(beamAngle/Scalar(2)) + dvlTrans.getBasis().getColumn(0) * btSin(beamAngle/Scalar(2));
    dir[1] = dvlTrans.getBasis().getColumn(2) * btCos(beamAngle/Scalar(2)) - dvlTrans.getBasis().getColumn(0) * btSin(beamAngle/Scalar(2));
    dir[2] = dvlTrans.getBasis().getColumn(2) * btCos(beamAngle/Scalar(2)) + dvlTrans.getBasis().getColumn(1) * btSin(beamAngle/Scalar(2));
    dir[3] = dvlTrans.getBasis().getColumn(2) * btCos(beamAngle/Scalar(2)) - dvlTrans.getBasis().getColumn(1) * btSin(beamAngle/Scalar(2));
    
    unsigned int divs = ceil(channels[3].rangeMax - channels[3].rangeMin);
    
    for(unsigned int i=0; i<4; ++i)
    {
        from[i] = dvlTrans.getOrigin() - dir[i] * channels[3].rangeMin;
        to[i] = dvlTrans.getOrigin() - dir[i] * channels[3].rangeMax;

        Vector3 step = (to[i]-from[i])/Scalar(divs);
        range[i] = Scalar(-1);

        for(unsigned int h=0; h<divs; ++h)
        {
            Vector3 from_ = from[i] + step*Scalar(h);
            Vector3 to_ = from[i] + step*Scalar(h+1);
            btCollisionWorld::ClosestRayResultCallback closest(from_, to_);
            SimulationApp::getApp()->getSimulationManager()->getDynamicsWorld()->rayTest(from_, to_, closest);
            
            if(closest.hasHit())
            {
                Vector3 p = from_.lerp(to_, closest.m_closestHitFraction);
                range[i] = (p - dvlTrans.getOrigin()).length();
                break;
            }
        }

        if(range[i] > Scalar(0) && (range[i] < minRange || minRange < Scalar(0)))
                minRange = range[i];
    }
   
    //Get velocity
    Vector3 v = dvlTrans.getBasis().inverse() * attach->getLinearVelocityInLocalPoint(dvlTrans.getOrigin() - attach->getCGTransform().getOrigin());
    
    //Record sample
    Scalar data[4] = {v.x(),v.y(),v.z(), minRange * btCos(beamAngle/Scalar(2))};
    Sample s(4, data);
    AddSampleToHistory(s);
    
    //Hack to set invalid altitude when all of the beams miss (needed because range limit is applied when adding sample to history)
    if(minRange < Scalar(0))
        history.back()->getDataPointer()[3] = Scalar(-1); 
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
    
    if(range[0] > Scalar(0))
    {
        item.points.push_back(glm::vec3(0,0,0));
        item.points.push_back(glm::vec3(-dir[0].x()*range[0], -dir[0].y()*range[0], -dir[0].z()*range[0]));
    }
    
    if(range[1] > Scalar(0))
    {
        item.points.push_back(glm::vec3(0,0,0));
        item.points.push_back(glm::vec3(-dir[1].x()*range[1], -dir[1].y()*range[1], -dir[1].z()*range[1]));
    }
    
    if(range[2] > Scalar(0))
    {
        item.points.push_back(glm::vec3(0,0,0));
        item.points.push_back(glm::vec3(-dir[2].x()*range[2], -dir[2].y()*range[2], -dir[2].z()*range[2]));
    }
    
    if(range[3] > Scalar(0))
    {
        item.points.push_back(glm::vec3(0,0,0));
        item.points.push_back(glm::vec3(-dir[3].x()*range[3], -dir[3].y()*range[3], -dir[3].z()*range[3]));
    }
    
    items.push_back(item);

    return items;
}

void DVL::setRange(const Vector3& velocityMax, Scalar altitudeMin, Scalar altitudeMax)
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

void DVL::setNoise(Scalar velocityStdDev, Scalar altitudeStdDev)
{
    channels[0].setStdDev(velocityStdDev);
    channels[1].setStdDev(velocityStdDev);
    channels[2].setStdDev(velocityStdDev);
    channels[3].setStdDev(altitudeStdDev);
}

ScalarSensorType DVL::getScalarSensorType()
{
    return ScalarSensorType::DVL;
}

}
