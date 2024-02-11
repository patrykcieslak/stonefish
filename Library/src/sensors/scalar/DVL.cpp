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
//  Copyright (c) 2017-2021 Patryk Cieslak. All rights reserved.
//

#include "sensors/scalar/DVL.h"

#include "BulletCollision/NarrowPhaseCollision/btRaycastCallback.h"
#include "core/SimulationApp.h"
#include "core/SimulationManager.h"
#include "entities/MovingEntity.h"
#include "sensors/Sample.h"
#include "graphics/OpenGLPipeline.h"

namespace sf
{

DVL::DVL(std::string uniqueName, Scalar beamAngleDeg, bool beamPositiveZ, Scalar frequency, int historyLength) : LinkSensor(uniqueName, frequency, historyLength)
{
    range[0] = range[1] = range[2] = range[3] = Scalar(0.);
    beamAngle = btRadians(beamAngleDeg);
    beamPosZ = beamPositiveZ;
    channels.push_back(SensorChannel("Velocity X", QuantityType::VELOCITY));
    channels.push_back(SensorChannel("Velocity Y", QuantityType::VELOCITY));
    channels.push_back(SensorChannel("Velocity Z", QuantityType::VELOCITY));
    channels.push_back(SensorChannel("Altitude", QuantityType::LENGTH));
    channels.push_back(SensorChannel("Water velocity X", QuantityType::VELOCITY));
    channels.push_back(SensorChannel("Water velocity Y", QuantityType::VELOCITY));
    channels.push_back(SensorChannel("Water velocity Z", QuantityType::VELOCITY));
    channels.push_back(SensorChannel("Status", QuantityType::UNITLESS));
    channels[3].rangeMin = Scalar(0);
    channels[3].rangeMax = Scalar(1000);
    addNoiseStdDev[0] = addNoiseStdDev[1] = Scalar(0);
    mulNoiseFactor[0] = mulNoiseFactor[1] = Scalar(0);
    setWaterLayer(0, 0, 0);
}

void DVL::setWaterLayer(Scalar minSize, Scalar nearBoundary, Scalar farBoundary)
{
    waterLayer.setX(btClamped(minSize, Scalar(0), channels[3].rangeMax - channels[3].rangeMin));
    waterLayer.setY(btClamped(nearBoundary, channels[3].rangeMin, channels[3].rangeMax - waterLayer.getX()));
    waterLayer.setZ(btClamped(farBoundary, waterLayer.getX() + waterLayer.getY(), channels[3].rangeMax));
}
    
void DVL::InternalUpdate(Scalar dt)
{
    /*
    Status:
    0 - bottom ping only
    1 - water ping only
    2 - bottom and water ping
    3 - no ping at all
    */
    //Check hit with bottom
    unsigned short status = 0;
    Transform dvlTrans = getSensorFrame();
    
    //Simulate 4 beam DVL (typical design)
    Vector3 dir[4];
    Vector3 from[4];
    Vector3 to[4];
    for(unsigned int i=0; i<4; ++i)
    {
        Scalar alpha = M_PI_4 + i * M_PI_2;
        dir[i] = dvlTrans.getBasis().getColumn(2) * btCos(beamAngle) 
                 + (dvlTrans.getBasis().getColumn(0) * btCos(alpha) + dvlTrans.getBasis().getColumn(1) * btSin(alpha)) * btSin(beamAngle);
    }
    
    Scalar dirFactor = beamPosZ ? Scalar(1) : Scalar(-1);

    unsigned int divs = ceil(channels[3].rangeMax - channels[3].rangeMin);
    Scalar minRange(-1);

    for(unsigned int i=0; i<4; ++i)
    {
        from[i] = dvlTrans.getOrigin() + dirFactor * dir[i] * channels[3].rangeMin;
        to[i] = dvlTrans.getOrigin() + dirFactor * dir[i] * channels[3].rangeMax;

        Vector3 step = (to[i]-from[i])/Scalar(divs);
        range[i] = Scalar(-1);

        for(unsigned int h=0; h<divs; ++h)
        {
            Vector3 from_ = from[i] + step*Scalar(h);
            Vector3 to_ = from[i] + step*Scalar(h+1);
            btCollisionWorld::ClosestRayResultCallback closest(from_, to_);
            closest.m_collisionFilterGroup = MASK_DYNAMIC;
            closest.m_collisionFilterMask = MASK_STATIC | MASK_DYNAMIC | MASK_ANIMATED_COLLIDING;
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

    //Get altitude
    Scalar altitude = channels[3].rangeMax;
    Vector3 v = V0();
    status = 3;

    //Check where the out of range happened
    bool tooClose = false;
    if(minRange < Scalar(0)) //No hit recorded in DVL operating range
    {
        for(unsigned int i=0; i<4; ++i)
        {
            range[i] = Scalar(-1);
            from[i] = dvlTrans.getOrigin() + dirFactor * dir[i] * channels[3].rangeMin;
            to[i] = dvlTrans.getOrigin();
            btCollisionWorld::ClosestRayResultCallback closest(from[i], to[i]);
            closest.m_collisionFilterGroup = MASK_DYNAMIC;
            closest.m_collisionFilterMask = MASK_STATIC | MASK_DYNAMIC | MASK_ANIMATED_COLLIDING;
            SimulationApp::getApp()->getSimulationManager()->getDynamicsWorld()->rayTest(from[i], to[i], closest);
            
            if(closest.hasHit() && btDot(closest.m_hitNormalWorld, dirFactor * dir[i]) > Scalar(0))
            {
                Vector3 p = from[i].lerp(to[i], closest.m_closestHitFraction);
                range[i] = (p - dvlTrans.getOrigin()).length();
                if(range[i] < minRange || minRange < Scalar(0)) minRange = range[i];
            }
        }
        if(minRange > Scalar(0)) //Detection closer than minimum range of DVL - no water ping possible
            tooClose = true;
    }
    else //Successful bottom ping
    {
        altitude = minRange * btCos(beamAngle);
        v = dvlTrans.getBasis().inverse() * attach->getLinearVelocityInLocalPoint(dvlTrans.getOrigin() - attach->getCGTransform().getOrigin());
        status = 0;
    }
    
    //Get water velocity
    Vector3 wv = V0();
    //Water velocity is reported as 0 if:
    //a) layer thickness = 0
    //b) compressed layer thickness is less than minimum layer thickness
    //ASSUME: Water layer far boundary has to be closer than 80% of altitude.
    if(!tooClose && waterLayer.getX() > Scalar(0) && Scalar(0.8)*altitude > waterLayer.getX() + waterLayer.getY()) //Water layer ping possible
    {
        Ocean* ocn = SimulationApp::getApp()->getSimulationManager()->getOcean();
        if(ocn != nullptr)
        {
            Vector3 zDir = dvlTrans.getBasis().getColumn(2);
            //Layer compressed until minimum layer thinckness is reached.
            Scalar layerSize = btClamped(Scalar(0.8) * altitude - waterLayer.getY(), waterLayer.getX(), waterLayer.getZ()-waterLayer.getY());
            //Sample velocity
            unsigned int n = ceil(layerSize/Scalar(0.1)); //ASSUME: Mesurement resolution = 0.1 m.
            Scalar dist = waterLayer.getY();
            Scalar weight(0);
            for(unsigned int i=0; i<=n; ++i)
            {
                Scalar x = Scalar(i)/Scalar(n);
                Scalar w = btMin(x, 1-x); //f(x) = mu min{x, 1-x} where mu changes the sharpness of the triangle
                Vector3 p = dvlTrans.getOrigin() + zDir * dist;
                wv += w * ocn->GetFluidVelocity(p);
                weight += w;
                dist += layerSize/Scalar(n);
            }
            wv /= weight;
        }
        //Transform to DVL frame and add DVL velocity
        wv = dvlTrans.getBasis().inverse() * wv - v;
        status = status == 3 ? 1 : 2;
    }
    
    //Update noise characteristics
    channels[0].setStdDev(mulNoiseFactor[0] * v.x() + addNoiseStdDev[0]);
    channels[1].setStdDev(mulNoiseFactor[0] * v.y() + addNoiseStdDev[0]);
    channels[2].setStdDev(mulNoiseFactor[0] * v.z() + addNoiseStdDev[0]);
    channels[4].setStdDev(mulNoiseFactor[1] * wv.x() + addNoiseStdDev[1]);
    channels[5].setStdDev(mulNoiseFactor[1] * wv.y() + addNoiseStdDev[1]);
    channels[6].setStdDev(mulNoiseFactor[1] * wv.z() + addNoiseStdDev[1]);
    
    //Save data
    Scalar data[8] = {v.x(), v.y(), v.z(), altitude, wv.x(), wv.y(), wv.z(), Scalar(status)};
    Sample s(8, data);
    AddSampleToHistory(s);
}

std::vector<Renderable> DVL::Render()
{
    std::vector<Renderable> items = LinkSensor::Render();
    if(isRenderable())
    {
        unsigned short status = (unsigned short)trunc(getLastValue(7));
        Renderable item;
        item.type = RenderableType::SENSOR_LINES;
        item.model = glMatrixFromTransform(getSensorFrame());    
        //Bottom ping
        if(status == 0 || status == 2) //Good bottom ping
        {
            //Beams
            Vector3 dir[4];
            for(unsigned int i=0; i<4; ++i)
            {
                Scalar alpha = M_PI_4 + i * M_PI_2;
                dir[i] = Vector3(0,0,1) * btCos(beamAngle) 
                         + (Vector3(1,0,0) * btCos(alpha) + Vector3(0,1,0) * btSin(alpha)) * btSin(beamAngle);
            }
            
            if(!beamPosZ)
            {
                dir[0] = -dir[0];
                dir[1] = -dir[1];
                dir[2] = -dir[2];
                dir[3] = -dir[3];
            }

            if(range[0] > Scalar(0))
            {
                item.points.push_back(glm::vec3(0,0,0));
                item.points.push_back(glm::vec3(dir[0].x()*range[0], dir[0].y()*range[0], dir[0].z()*range[0]));
            }
            
            if(range[1] > Scalar(0))
            {
                item.points.push_back(glm::vec3(0,0,0));
                item.points.push_back(glm::vec3(dir[1].x()*range[1], dir[1].y()*range[1], dir[1].z()*range[1]));
            }
            
            if(range[2] > Scalar(0))
            {
                item.points.push_back(glm::vec3(0,0,0));
                item.points.push_back(glm::vec3(dir[2].x()*range[2], dir[2].y()*range[2], dir[2].z()*range[2]));
            }
            
            if(range[3] > Scalar(0))
            {
                item.points.push_back(glm::vec3(0,0,0));
                item.points.push_back(glm::vec3(dir[3].x()*range[3], dir[3].y()*range[3], dir[3].z()*range[3]));
            }
        }
        //Water ping
        if(status == 1 || status == 2) //Good water ping
        {
            Scalar layerSize = btClamped(Scalar(0.8) * getLastValue(3) - waterLayer.getY(), waterLayer.getX(), waterLayer.getZ()-waterLayer.getY());        
            GLfloat a1 = (GLfloat)waterLayer.getY();
            GLfloat a2 = (GLfloat)(waterLayer.getY() + layerSize);
            GLfloat r1 = a1 * btSin(beamAngle);
            GLfloat r2 = a2 * btSin(beamAngle);
            for(unsigned int i=0; i<4; ++i)
            {
                GLfloat ang1 = (GLfloat)i/2.f * glm::pi<GLfloat>();
                GLfloat ang2 = (GLfloat)(i+1)/2.f * glm::pi<GLfloat>();
                glm::vec3 d1(glm::sin(ang1), glm::cos(ang1), 0.f);
                glm::vec3 d2(glm::sin(ang2), glm::cos(ang2), 0.f);
                item.points.push_back(r1 * d1 + glm::vec3(0.f, 0.f, -a1));
                item.points.push_back(r1 * d2 + glm::vec3(0.f, 0.f, -a1));
                item.points.push_back(r2 * d1 + glm::vec3(0.f, 0.f, -a2));
                item.points.push_back(r2 * d2 + glm::vec3(0.f, 0.f, -a2));
            }
        }
        items.push_back(item);
    }
    return items;
}

void DVL::setRange(const Vector3& velocityMax, Scalar altitudeMin, Scalar altitudeMax)
{
    //Velocity
    channels[0].rangeMin = -btClamped(velocityMax.getX(), Scalar(0), Scalar(BT_LARGE_FLOAT));
    channels[1].rangeMin = -btClamped(velocityMax.getY(), Scalar(0), Scalar(BT_LARGE_FLOAT));
    channels[2].rangeMin = -btClamped(velocityMax.getZ(), Scalar(0), Scalar(BT_LARGE_FLOAT));
    channels[0].rangeMax = btClamped(velocityMax.getX(), Scalar(0), Scalar(BT_LARGE_FLOAT));
    channels[1].rangeMax = btClamped(velocityMax.getY(), Scalar(0), Scalar(BT_LARGE_FLOAT));
    channels[2].rangeMax = btClamped(velocityMax.getZ(), Scalar(0), Scalar(BT_LARGE_FLOAT));
    //Altitude
    channels[3].rangeMin = btClamped(altitudeMin, Scalar(0), Scalar(BT_LARGE_FLOAT));
    channels[3].rangeMax = btClamped(altitudeMax, Scalar(0), Scalar(BT_LARGE_FLOAT));
    //Water velocity
    channels[4].rangeMin = -btClamped(velocityMax.getX(), Scalar(0), Scalar(BT_LARGE_FLOAT));
    channels[5].rangeMin = -btClamped(velocityMax.getY(), Scalar(0), Scalar(BT_LARGE_FLOAT));
    channels[6].rangeMin = -btClamped(velocityMax.getZ(), Scalar(0), Scalar(BT_LARGE_FLOAT));
    channels[4].rangeMax = btClamped(velocityMax.getX(), Scalar(0), Scalar(BT_LARGE_FLOAT));
    channels[5].rangeMax = btClamped(velocityMax.getY(), Scalar(0), Scalar(BT_LARGE_FLOAT));
    channels[6].rangeMax = btClamped(velocityMax.getZ(), Scalar(0), Scalar(BT_LARGE_FLOAT));
}

void DVL::setNoise(Scalar velPercent, Scalar velStdDev, Scalar altitudeStdDev, Scalar waterVelPercent, Scalar waterVelStdDev)
{
    channels[3].setStdDev(btClamped(altitudeStdDev, Scalar(0), Scalar(BT_LARGE_FLOAT)));
    addNoiseStdDev[0] = btClamped(velStdDev, Scalar(0), Scalar(BT_LARGE_FLOAT));
    addNoiseStdDev[1] = btClamped(waterVelStdDev, Scalar(0), Scalar(BT_LARGE_FLOAT));;
    mulNoiseFactor[0] = btClamped(velPercent, Scalar(0), Scalar(100))/Scalar(100);
    mulNoiseFactor[1] = btClamped(waterVelPercent, Scalar(0), Scalar(100))/Scalar(100);
}

void DVL::getRange(Vector3& velocityMax, Scalar& altitudeMin, Scalar& altitudeMax) const
{
    velocityMax.setX(channels[0].rangeMax);
    velocityMax.setY(channels[1].rangeMax);
    velocityMax.setZ(channels[2].rangeMax);
    altitudeMin = channels[3].rangeMin;
    altitudeMax = channels[3].rangeMax;
}

Scalar DVL::getBeamAngle() const
{
    return beamAngle;
}

ScalarSensorType DVL::getScalarSensorType()
{
    return ScalarSensorType::DVL;
}

}
