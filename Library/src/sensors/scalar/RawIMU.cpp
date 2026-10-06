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
//  RawIMU.cpp
//  Stonefish
//
//  Created by Roger Pi on 02/10/2026
//  Copyright (c) 2026 Patryk Cieslak. All rights reserved.
//

#include "sensors/scalar/RawIMU.h"

#include "entities/MovingEntity.h"
#include "sensors/Sample.h"
#include "core/SimulationApp.h"
#include "core/SimulationManager.h"

namespace sf
{

RawIMU::RawIMU(std::string uniqueName, Scalar frequency, int historyLength) : LinkSensor(uniqueName, frequency, historyLength)
{
    channels.push_back(SensorChannel("Angular velocity X", QuantityType::ANGULAR_VELOCITY));
    channels.push_back(SensorChannel("Angular velocity Y", QuantityType::ANGULAR_VELOCITY));
    channels.push_back(SensorChannel("Angular velocity Z", QuantityType::ANGULAR_VELOCITY));
    channels.push_back(SensorChannel("Linear acceleration X", QuantityType::ACCELERATION));
    channels.push_back(SensorChannel("Linear acceleration Y", QuantityType::ACCELERATION));
    channels.push_back(SensorChannel("Linear acceleration Z", QuantityType::ACCELERATION));

    earthRotation = true;
    initialUpdate = false;
    setGyroBias(V0());
    setAccBias(V0());
}

void RawIMU::InternalUpdate(Scalar dt)
{
    // Sensor::Reset() performs the first measurement with a dummy time step,
    // which must not propagate the time-varying biases.
    if(initialUpdate)
        initialUpdate = false;
    else
    {
        PropagateBias(gyroBias, dt);
        PropagateBias(accBias, dt);
    }

    //get sensor frame in world
    Transform imuTrans = getSensorFrame();
    Matrix3 toImuFrame = imuTrans.getBasis().inverse();
    Vector3 R = imuTrans.getOrigin() - attach->getCGTransform().getOrigin();
    Vector3 omega = attach->getAngularVelocity();

    //angular velocity (relative to NED) and specific force
    Vector3 av = omega;
    Vector3 f = attach->getLinearAcceleration()
                 + attach->getAngularAcceleration().cross(R)
                 + omega.cross(omega.cross(R))
                 - SimulationApp::getApp()->getSimulationManager()->getGravity(); // Negative to get readings like in actual sensor

    //Earth rotation: gyros sense the Earth rate, accelerometers sense the Coriolis acceleration.
    //Gravity is plumb-bob gravity, so it already contains the centripetal term.
    if(earthRotation)
    {
        Vector3 earthRate = SimulationApp::getApp()->getSimulationManager()->getEarthRate();
        Vector3 v = attach->getLinearVelocity() + omega.cross(R);
        av += earthRate;
        f += Scalar(2) * earthRate.cross(v);
    }

    //transform to sensor frame and add biases
    av = toImuFrame * av + gyroBias.total();
    f = toImuFrame * f + accBias.total();

    //record sample (white noise is added in AddSampleToHistory)
    Sample s{std::vector<Scalar>({av.x(), av.y(), av.z(), f.x(), f.y(), f.z()})};
    AddSampleToHistory(s);
}

void RawIMU::Reset()
{
    ResetBias(gyroBias);
    ResetBias(accBias);
    initialUpdate = true;
    ScalarSensor::Reset();
}

void RawIMU::ResetBias(BiasModel& bias)
{
    for(int i=0; i<3; ++i)
    {
        Scalar sd = bias.turnOnStdDev[i];
        bias.turnOn[i] = sd > Scalar(0) ? std::normal_distribution<Scalar>(Scalar(0), sd)(randomGenerator) : Scalar(0);
    }
    bias.dynamic = V0();
}

void RawIMU::PropagateBias(BiasModel& bias, Scalar dt)
{
    if(dt <= Scalar(0))
        return;

    std::normal_distribution<Scalar> n(Scalar(0), Scalar(1));
    for(int i=0; i<3; ++i)
    {
        Scalar sigma = bias.instability[i];
        if(sigma <= Scalar(0))
            continue;

        Scalar tau = bias.tau[i];
        if(tau > Scalar(0)) //First-order Gauss-Markov (exact discretization)
        {
            Scalar phi = btExp(-dt/tau);
            bias.dynamic[i] = phi * bias.dynamic[i] + sigma * btSqrt(Scalar(1) - phi*phi) * n(randomGenerator);
        }
        else //Random walk
            bias.dynamic[i] += sigma * btSqrt(dt) * n(randomGenerator);
    }
}

Vector3 RawIMU::BiasModel::total() const
{
    return constant + turnOn + dynamic;
}

void RawIMU::setRange(Vector3 angularVelocityMax, Vector3 linearAccelerationMax)
{
    for(int i=0; i<3; ++i)
    {
        Scalar av = btClamped(angularVelocityMax[i], Scalar(0), Scalar(BT_LARGE_FLOAT));
        Scalar la = btClamped(linearAccelerationMax[i], Scalar(0), Scalar(BT_LARGE_FLOAT));
        channels[i].rangeMin = -av;
        channels[i].rangeMax = av;
        channels[i+3].rangeMin = -la;
        channels[i+3].rangeMax = la;
    }
}

void RawIMU::setNoise(Vector3 angularVelocityStdDev, Vector3 linearAccelerationStdDev)
{
    for(int i=0; i<3; ++i)
    {
        channels[i].setStdDev(btClamped(angularVelocityStdDev[i], Scalar(0), Scalar(BT_LARGE_FLOAT)));
        channels[i+3].setStdDev(btClamped(linearAccelerationStdDev[i], Scalar(0), Scalar(BT_LARGE_FLOAT)));
    }
}

void RawIMU::setEarthRotation(bool enabled)
{
    earthRotation = enabled;
}

void RawIMU::setGyroBias(Vector3 constant, Vector3 turnOnStdDev, Vector3 instability, Vector3 tau)
{
    gyroBias.constant = constant;
    gyroBias.turnOnStdDev = turnOnStdDev;
    gyroBias.instability = instability;
    gyroBias.tau = tau;
    ResetBias(gyroBias);
}

void RawIMU::setAccBias(Vector3 constant, Vector3 turnOnStdDev, Vector3 instability, Vector3 tau)
{
    accBias.constant = constant;
    accBias.turnOnStdDev = turnOnStdDev;
    accBias.instability = instability;
    accBias.tau = tau;
    ResetBias(accBias);
}

Vector3 RawIMU::getGyroBias() const
{
    return gyroBias.total();
}

Vector3 RawIMU::getAccBias() const
{
    return accBias.total();
}

bool RawIMU::isEarthRotationEnabled() const
{
    return earthRotation;
}

ScalarSensorType RawIMU::getScalarSensorType() const
{
    return ScalarSensorType::RAW_IMU;
}

}
