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
//  INS.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 21/10/2021.
//  Copyright (c) 2021 Patryk Cieslak. All rights reserved.
//

#include "sensors/scalar/INS.h"

#include "core/SimulationApp.h"
#include "core/SimulationManager.h"
#include "core/NED.h"
#include "entities/MovingEntity.h"
#include "sensors/Sample.h"

namespace sf
{

INS::INS(std::string uniqueName, Scalar frequency, int historyLength) : LinkSensor(uniqueName, frequency, historyLength)
{
    channels.push_back(SensorChannel("North", QuantityType::LENGTH));
    channels.push_back(SensorChannel("East", QuantityType::LENGTH));
    channels.push_back(SensorChannel("Depth", QuantityType::LENGTH));
    channels.push_back(SensorChannel("Altitude", QuantityType::LENGTH));
    channels.push_back(SensorChannel("Latitude", QuantityType::ANGLE));
    channels.push_back(SensorChannel("Longitude", QuantityType::ANGLE));
    channels.push_back(SensorChannel("Body velocity X", QuantityType::LENGTH));
    channels.push_back(SensorChannel("Body velocity Y", QuantityType::LENGTH));
    channels.push_back(SensorChannel("Body velocity Z", QuantityType::LENGTH));
    channels.push_back(SensorChannel("Roll", QuantityType::ANGLE));
    channels.push_back(SensorChannel("Pitch", QuantityType::ANGLE));
    channels.push_back(SensorChannel("Yaw", QuantityType::ANGLE));
    channels.push_back(SensorChannel("Roll rate", QuantityType::ANGULAR_VELOCITY));
    channels.push_back(SensorChannel("Pitch rate", QuantityType::ANGULAR_VELOCITY));
    channels.push_back(SensorChannel("Yaw rate", QuantityType::ANGULAR_VELOCITY));
    channels.push_back(SensorChannel("Body acceleration X", QuantityType::ACCELERATION));
    channels.push_back(SensorChannel("Body acceleration Y", QuantityType::ACCELERATION));
    channels.push_back(SensorChannel("Body acceleration Z", QuantityType::ACCELERATION));
    
    gpsName = "";
    dvlName = "";
    pressName = "";
    imuNoise = false;
    out = I4();
}

void INS::Reset()
{
    ScalarSensor::Reset();
    ned = V0();
    velocity = V0();
    altitude = 0.0;
    Scalar height;
    SimulationApp::getApp()->getSimulationManager()->getNED()->Ned2Geodetic(0.0, 0.0, 0.0, latitude, longitude, height);
}

void INS::InternalUpdate(Scalar dt)
{
    Scalar now = SimulationApp::getApp()->getSimulationManager()->getSimulationTime();
    
    //--- internal sensors
    //get sensor frame in world
    Transform imuTrans = getSensorFrame();
    Vector3 R = imuTrans.getOrigin() - attach->getCGTransform().getOrigin();
    //get angular velocity
    Vector3 av = imuTrans.getBasis().inverse() * attach->getAngularVelocity();
    //get acceleration
    Vector3 acc = imuTrans.getBasis().inverse() * (
                   attach->getLinearAcceleration() 
                   + attach->getAngularAcceleration().cross(R)
                   + attach->getAngularVelocity().cross(attach->getAngularVelocity().cross(R))
                ); // NO GRAVITY
    //get angular acceleration
    Vector3 aacc = imuTrans.getBasis().inverse() * attach->getAngularAcceleration();

    //noise
    if(imuNoise)
    {
        av.setX(av.x() + avNoiseX(randomGenerator));
        av.setY(av.y() + avNoiseY(randomGenerator));
        av.setZ(av.z() + avNoiseZ(randomGenerator));
        acc.setX(acc.x() + accNoiseX(randomGenerator));
        acc.setY(acc.y() + accNoiseY(randomGenerator));
        acc.setZ(acc.z() + accNoiseZ(randomGenerator));
    }

    //Predict (implicit Euler)
    velocity += acc * dt; //In body frame (accumulated velocity)
    
    //--- external sensors
    DVL* dvl;
    if(dvlName != "" && (dvl = (DVL*)SimulationApp::getApp()->getSimulationManager()->getSensor(dvlName)) != nullptr) //Correct velocities
    {
        Sample s = dvl->getLastSample();
        Scalar ts = s.getTimestamp();
        if(ts >= 0.0 && now-ts <= dt) //Is DVL valid?
        {
            Transform dvlTrans = dvl->getSensorFrame();
            Vector3 avv = imuTrans.getBasis() * av; // Includes noise
            velocity = imuTrans.getBasis().inverse()* (
                       dvlTrans.getBasis() * Vector3(s.getValue(0), s.getValue(1), s.getValue(2)) // Pure linear velocity component
                       + avv.cross(imuTrans.getOrigin() - dvlTrans.getOrigin()) // Angular velocity component
                       ); 
            altitude = s.getValue(3);
        }
    }

    Vector3 dp = velocity * dt; //In body frame
    ned += imuTrans.getBasis() * dp; //In NED frame
    
    GPS* gps;
    if(gpsName != "" && (gps = (GPS*)SimulationApp::getApp()->getSimulationManager()->getSensor(gpsName)) != nullptr) //Correct global position
    {
        Sample s = gps->getLastSample();
        Scalar ts = s.getTimestamp();
        if(ts >= 0.0 && now-ts <= dt && s.getValue(0) <= Scalar(90) && s.getValue(1) <= Scalar(180)) //Is GPS valid?
        {
            Vector3 trans = imuTrans.getOrigin() - gps->getSensorFrame().getOrigin();
            ned.setX(trans.x() + s.getValue(2));
            ned.setY(trans.y() + s.getValue(3));
        }
    }

    Pressure* press;
    if(pressName != "" && (press = (Pressure*)SimulationApp::getApp()->getSimulationManager()->getSensor(pressName)) != nullptr) //Correct depth
    {
        Sample s = press->getLastSample();
        Scalar ts = s.getTimestamp();
        if(ts >= 0.0 && now-ts <= dt) //Is pressure valid?
        {
            Ocean* liq = SimulationApp::getApp()->getSimulationManager()->getOcean();
            if(liq != NULL)
            {
                Scalar rho = liq->getLiquid().density;
                Scalar g = SimulationApp::getApp()->getSimulationManager()->getGravity().z();
                Scalar depth = s.getValue(0)/(rho*g);
                ned.setZ(depth + imuTrans.getOrigin().z() - press->getSensorFrame().getOrigin().z() ); //Depth at INS frame
            }
        }
    }

    //transformed output
    Vector3 nedo = ned + (imuTrans.getBasis() * out.getOrigin());
    Vector3 velo = out.getBasis().inverse() * (velocity + av.cross(out.getOrigin()));
    Vector3 avo = out.getBasis().inverse() * av;
    Vector3 acco = out.getBasis().inverse() * (acc + aacc.cross(out.getOrigin()) + av.cross(av.cross(out.getOrigin()))); 
    
    //compute geodetic position
    Scalar height;
    SimulationApp::getApp()->getSimulationManager()->getNED()->Ned2Geodetic(nedo.x(), nedo.y(), nedo.z(), latitude, longitude, height);
    
    //get angles
    Scalar yaw, pitch, roll;
    (imuTrans * out).getBasis().getEulerYPR(yaw, pitch, roll);

    //record sample
    Scalar values[18] = {nedo.x(), nedo.y(), nedo.z(), altitude, latitude, longitude,
                         velo.x(), velo.y(), velo.z(), roll, pitch, yaw, 
                         avo.x(), avo.y(), avo.z(), acco.x(), acco.y(), acco.z()};
    Sample s(18, values);
    AddSampleToHistory(s); //Adds noise.....:(
}

void INS::ConnectGPS(const std::string& name)
{
    gpsName = name;
}

void INS::ConnectDVL(const std::string& name)
{
    dvlName = name;
}

void INS::ConnectPressure(const std::string& name)
{
    pressName = name;
}

void INS::setOutputFrame(const Transform& T)
{
    out = T;
}

void INS::setRange(Vector3 angularVelocityMax, Vector3 linearAccelerationMax)
{
    channels[12].rangeMin = -btClamped(angularVelocityMax.x(), Scalar(0), Scalar(BT_LARGE_FLOAT));
    channels[13].rangeMin = -btClamped(angularVelocityMax.y(), Scalar(0), Scalar(BT_LARGE_FLOAT));
    channels[14].rangeMin = -btClamped(angularVelocityMax.z(), Scalar(0), Scalar(BT_LARGE_FLOAT));
    channels[12].rangeMax = btClamped(angularVelocityMax.x(), Scalar(0), Scalar(BT_LARGE_FLOAT));
    channels[13].rangeMax = btClamped(angularVelocityMax.y(), Scalar(0), Scalar(BT_LARGE_FLOAT));
    channels[14].rangeMax = btClamped(angularVelocityMax.z(), Scalar(0), Scalar(BT_LARGE_FLOAT));
    channels[15].rangeMin = -btClamped(linearAccelerationMax.x(), Scalar(0), Scalar(BT_LARGE_FLOAT));
    channels[16].rangeMin = -btClamped(linearAccelerationMax.y(), Scalar(0), Scalar(BT_LARGE_FLOAT));
    channels[17].rangeMin = -btClamped(linearAccelerationMax.z(), Scalar(0), Scalar(BT_LARGE_FLOAT));
    channels[15].rangeMax = btClamped(linearAccelerationMax.x(), Scalar(0), Scalar(BT_LARGE_FLOAT));
    channels[16].rangeMax = btClamped(linearAccelerationMax.y(), Scalar(0), Scalar(BT_LARGE_FLOAT));
    channels[17].rangeMax = btClamped(linearAccelerationMax.z(), Scalar(0), Scalar(BT_LARGE_FLOAT));
}
    
void INS::setNoise(Vector3 angularVelocityStdDev, Vector3 linearAccelerationStdDev)
{
    avNoiseX = std::normal_distribution<Scalar>(Scalar(0), btClamped(angularVelocityStdDev.x(), Scalar(0), BT_LARGE_FLOAT));
    avNoiseY = std::normal_distribution<Scalar>(Scalar(0), btClamped(angularVelocityStdDev.y(), Scalar(0), BT_LARGE_FLOAT));
    avNoiseZ = std::normal_distribution<Scalar>(Scalar(0), btClamped(angularVelocityStdDev.z(), Scalar(0), BT_LARGE_FLOAT));
    
    accNoiseX = std::normal_distribution<Scalar>(Scalar(0), btClamped(linearAccelerationStdDev.x(), Scalar(0), BT_LARGE_FLOAT));
    accNoiseY = std::normal_distribution<Scalar>(Scalar(0), btClamped(linearAccelerationStdDev.y(), Scalar(0), BT_LARGE_FLOAT));
    accNoiseZ = std::normal_distribution<Scalar>(Scalar(0), btClamped(linearAccelerationStdDev.z(), Scalar(0), BT_LARGE_FLOAT));

    imuNoise = true;
}

ScalarSensorType INS::getScalarSensorType()
{
    return ScalarSensorType::INS;
}

std::vector<Renderable> INS::Render()
{
    std::vector<Renderable> items = LinkSensor::Render();
    if(isRenderable())
    {
        Renderable item;
        item.type = RenderableType::SENSOR_CS;
        item.model = glMatrixFromTransform(getSensorFrame() * out);
        items.push_back(item);

        item.type = RenderableType::SENSOR_LINES;
        item.model = glMatrixFromTransform(getSensorFrame());
        item.points.push_back(glm::vec3(0.f));
        item.points.push_back(glVectorFromVector(out.getOrigin()));
        items.push_back(item);
    }
    return items;
}

}
