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
//  Copyright (c) 2021-2025 Patryk Cieslak. All rights reserved.
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
    channels_.push_back(SensorChannel("North", QuantityType::LENGTH));
    channels_.push_back(SensorChannel("East", QuantityType::LENGTH));
    channels_.push_back(SensorChannel("Depth", QuantityType::LENGTH));
    channels_.push_back(SensorChannel("Altitude", QuantityType::LENGTH));
    channels_.push_back(SensorChannel("Latitude", QuantityType::ANGLE));
    channels_.push_back(SensorChannel("Longitude", QuantityType::ANGLE));
    channels_.push_back(SensorChannel("Body velocity X", QuantityType::LENGTH));
    channels_.push_back(SensorChannel("Body velocity Y", QuantityType::LENGTH));
    channels_.push_back(SensorChannel("Body velocity Z", QuantityType::LENGTH));
    channels_.push_back(SensorChannel("Roll", QuantityType::ANGLE));
    channels_.push_back(SensorChannel("Pitch", QuantityType::ANGLE));
    channels_.push_back(SensorChannel("Yaw", QuantityType::ANGLE));
    channels_.push_back(SensorChannel("Roll rate", QuantityType::ANGULAR_VELOCITY));
    channels_.push_back(SensorChannel("Pitch rate", QuantityType::ANGULAR_VELOCITY));
    channels_.push_back(SensorChannel("Yaw rate", QuantityType::ANGULAR_VELOCITY));
    channels_.push_back(SensorChannel("Body acceleration X", QuantityType::ACCELERATION));
    channels_.push_back(SensorChannel("Body acceleration Y", QuantityType::ACCELERATION));
    channels_.push_back(SensorChannel("Body acceleration Z", QuantityType::ACCELERATION));
    
    gpsName_ = "";
    dvlName_ = "";
    pressName_ = "";
    imuNoise_ = false;
    out_ = I4();
}

void INS::Reset()
{
    ScalarSensor::Reset();
    ned_ = V0();
    velocity_ = V0();
    altitude_ = 0.0;
    Scalar height;
    SimulationApp::getApp()->getSimulationManager()->getNED()->Ned2Geodetic(0.0, 0.0, 0.0, latitude_, longitude_, height);
}

void INS::InternalUpdate(Scalar dt)
{
    Scalar now = SimulationApp::getApp()->getSimulationManager()->getSimulationTime(true);
    
    //--- internal sensors
    //get sensor frame in world
    Transform imuTrans = getSensorFrame();
    Vector3 R = imuTrans.getOrigin() - attach_->getCGTransform().getOrigin();
    //get angular velocity
    Vector3 av = imuTrans.getBasis().inverse() * attach_->getAngularVelocity();
    //get acceleration
    Vector3 acc = imuTrans.getBasis().inverse() * (
                   attach_->getLinearAcceleration() 
                   + attach_->getAngularAcceleration().cross(R)
                   + attach_->getAngularVelocity().cross(attach_->getAngularVelocity().cross(R))
                ); // NO GRAVITY
    //get angular acceleration
    Vector3 aacc = imuTrans.getBasis().inverse() * attach_->getAngularAcceleration();

    //noise
    if(imuNoise_)
    {
        av.setX(av.x() + avNoiseX_(randomGenerator));
        av.setY(av.y() + avNoiseY_(randomGenerator));
        av.setZ(av.z() + avNoiseZ_(randomGenerator));
        acc.setX(acc.x() + accNoiseX_(randomGenerator));
        acc.setY(acc.y() + accNoiseY_(randomGenerator));
        acc.setZ(acc.z() + accNoiseZ_(randomGenerator));
    }

    //Predict (implicit Euler)
    velocity_ += acc * dt; //In body frame (accumulated velocity)
    
    //--- external sensors
    DVL* dvl;
    if(dvlName_ != "" && (dvl = (DVL*)SimulationApp::getApp()->getSimulationManager()->getSensor(dvlName_)) != nullptr) //Correct velocities
    {
        Sample s = dvl->getLastSample();
        Scalar ts = s.getTimestamp();
        if(ts >= 0.0 && now-ts <= dt) //Is DVL valid?
        {
            Transform dvlTrans = dvl->getSensorFrame();
            Vector3 avv = imuTrans.getBasis() * av; // Includes noise
            velocity_ = imuTrans.getBasis().inverse()* (
                       dvlTrans.getBasis() * Vector3(s.getValue(0), s.getValue(1), s.getValue(2)) // Pure linear velocity component
                       + avv.cross(imuTrans.getOrigin() - dvlTrans.getOrigin()) // Angular velocity component
                       ); 
            altitude_ = s.getValue(3);
        }
    }

    Vector3 dp = velocity_ * dt; //In body frame
    ned_ += imuTrans.getBasis() * dp; //In NED frame
    
    GPS* gps;
    if(gpsName_ != "" && (gps = (GPS*)SimulationApp::getApp()->getSimulationManager()->getSensor(gpsName_)) != nullptr) //Correct global position
    {
        Sample s = gps->getLastSample();
        Scalar ts = s.getTimestamp();
        if(ts >= 0.0 && now-ts <= dt && s.getValue(0) <= Scalar(90) && s.getValue(1) <= Scalar(180)) //Is GPS valid?
        {
            Vector3 trans = imuTrans.getOrigin() - gps->getSensorFrame().getOrigin();
            ned_.setX(trans.x() + s.getValue(2));
            ned_.setY(trans.y() + s.getValue(3));
        }
    }

    Pressure* press;
    if(pressName_ != "" && (press = (Pressure*)SimulationApp::getApp()->getSimulationManager()->getSensor(pressName_)) != nullptr) //Correct depth
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
                ned_.setZ(depth + imuTrans.getOrigin().z() - press->getSensorFrame().getOrigin().z() ); //Depth at INS frame
            }
        }
    }

    //transformed output
    Vector3 nedo = ned_ + (imuTrans.getBasis() * out_.getOrigin());
    Vector3 velo = out_.getBasis().inverse() * (velocity_ + av.cross(out_.getOrigin()));
    Vector3 avo = out_.getBasis().inverse() * av;
    Vector3 acco = out_.getBasis().inverse() * (acc + aacc.cross(out_.getOrigin()) + av.cross(av.cross(out_.getOrigin()))); 
    
    //compute geodetic position
    Scalar height;
    SimulationApp::getApp()->getSimulationManager()->getNED()->Ned2Geodetic(nedo.x(), nedo.y(), nedo.z(), latitude_, longitude_, height);
    
    //get angles
    Scalar yaw, pitch, roll;
    (imuTrans * out_).getBasis().getEulerYPR(yaw, pitch, roll);

    //record sample
    Sample s{std::vector<Scalar>(
        {nedo.x(), nedo.y(), nedo.z(), altitude_, latitude_, longitude_,
         velo.x(), velo.y(), velo.z(), roll, pitch, yaw, 
         avo.x(), avo.y(), avo.z(), acco.x(), acco.y(), acco.z()}
        )};
    AddSampleToHistory(s); //Adds noise.....:(
}

void INS::ConnectGPS(const std::string& name)
{
    gpsName_ = name;
}

void INS::ConnectDVL(const std::string& name)
{
    dvlName_ = name;
}

void INS::ConnectPressure(const std::string& name)
{
    pressName_ = name;
}

void INS::setOutputFrame(const Transform& T)
{
    out_ = T;
}

void INS::setRange(Vector3 angularVelocityMax, Vector3 linearAccelerationMax)
{
    channels_[12].rangeMin = -btClamped(angularVelocityMax.x(), Scalar(0), Scalar(BT_LARGE_FLOAT));
    channels_[13].rangeMin = -btClamped(angularVelocityMax.y(), Scalar(0), Scalar(BT_LARGE_FLOAT));
    channels_[14].rangeMin = -btClamped(angularVelocityMax.z(), Scalar(0), Scalar(BT_LARGE_FLOAT));
    channels_[12].rangeMax = btClamped(angularVelocityMax.x(), Scalar(0), Scalar(BT_LARGE_FLOAT));
    channels_[13].rangeMax = btClamped(angularVelocityMax.y(), Scalar(0), Scalar(BT_LARGE_FLOAT));
    channels_[14].rangeMax = btClamped(angularVelocityMax.z(), Scalar(0), Scalar(BT_LARGE_FLOAT));
    channels_[15].rangeMin = -btClamped(linearAccelerationMax.x(), Scalar(0), Scalar(BT_LARGE_FLOAT));
    channels_[16].rangeMin = -btClamped(linearAccelerationMax.y(), Scalar(0), Scalar(BT_LARGE_FLOAT));
    channels_[17].rangeMin = -btClamped(linearAccelerationMax.z(), Scalar(0), Scalar(BT_LARGE_FLOAT));
    channels_[15].rangeMax = btClamped(linearAccelerationMax.x(), Scalar(0), Scalar(BT_LARGE_FLOAT));
    channels_[16].rangeMax = btClamped(linearAccelerationMax.y(), Scalar(0), Scalar(BT_LARGE_FLOAT));
    channels_[17].rangeMax = btClamped(linearAccelerationMax.z(), Scalar(0), Scalar(BT_LARGE_FLOAT));
}
    
void INS::setNoise(Vector3 angularVelocityStdDev, Vector3 linearAccelerationStdDev)
{
    avNoiseX_ = std::normal_distribution<Scalar>(Scalar(0), btClamped(angularVelocityStdDev.x(), Scalar(0), BT_LARGE_FLOAT));
    avNoiseY_ = std::normal_distribution<Scalar>(Scalar(0), btClamped(angularVelocityStdDev.y(), Scalar(0), BT_LARGE_FLOAT));
    avNoiseZ_ = std::normal_distribution<Scalar>(Scalar(0), btClamped(angularVelocityStdDev.z(), Scalar(0), BT_LARGE_FLOAT));
    
    accNoiseX_ = std::normal_distribution<Scalar>(Scalar(0), btClamped(linearAccelerationStdDev.x(), Scalar(0), BT_LARGE_FLOAT));
    accNoiseY_ = std::normal_distribution<Scalar>(Scalar(0), btClamped(linearAccelerationStdDev.y(), Scalar(0), BT_LARGE_FLOAT));
    accNoiseZ_ = std::normal_distribution<Scalar>(Scalar(0), btClamped(linearAccelerationStdDev.z(), Scalar(0), BT_LARGE_FLOAT));

    imuNoise_ = true;
}

ScalarSensorType INS::getScalarSensorType() const
{
    return ScalarSensorType::INS;
}

std::vector<Renderable> INS::Render()
{
    std::vector<Renderable> items = LinkSensor::Render();
    if(isRenderable())
    {
        Renderable item1;
        item1.type = RenderableType::SENSOR_CS;
        item1.model = glMatrixFromTransform(getSensorFrame() * out_);
        items.push_back(item1);

        Renderable item2;
        item2.type = RenderableType::SENSOR_LINES;
        item2.model = glMatrixFromTransform(getSensorFrame());
        item2.data = std::make_shared<std::vector<glm::vec3>>();
        auto points = item2.getDataAsPoints();
        points->push_back(glm::vec3(0.f));
        points->push_back(glVectorFromVector(out_.getOrigin()));
        items.push_back(item2);
    }
    return items;
}

}
