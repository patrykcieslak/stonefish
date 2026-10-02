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
//  RawIMU.h
//  Stonefish
//
//  Created by Roger Pi on 02/10/2026
//  Copyright (c) 2026 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_RawIMU__
#define __Stonefish_RawIMU__

#include "sensors/scalar/LinkSensor.h"

namespace sf
{
    //! A class representing a raw (strapdown) inertial measurement unit.
    /*!
     The sensor outputs only angular velocity and specific force, like a real strapdown IMU.
     Optionally, the measurements include the Earth rotation rate (gyroscopes) and the Coriolis
     acceleration (accelerometers), computed at the latitude of the NED origin. Each triad can have
     a constant bias, a random turn-on bias drawn at every reset, and a time-varying bias modelled as
     a first-order Gauss-Markov process (or a random walk if the correlation time is not positive).
     */
    class RawIMU : public LinkSensor
    {
    public:
        //! A constructor.
        /*!
         \param uniqueName a name for the sensor
         \param frequency the sampling frequency of the sensor [Hz] (-1 if updated every simulation step)
         \param historyLength defines: -1 -> no history, 0 -> unlimited history, >0 -> history with a specified length
         */
        RawIMU(std::string uniqueName, Scalar frequency = Scalar(-1), int historyLength = -1);

        //! A method performing internal sensor state update.
        /*!
         \param dt the step time of the simulation [s]
         */
        void InternalUpdate(Scalar dt) override;

        //! A method that resets the sensor (redraws turn-on biases and clears time-varying biases).
        void Reset() override;

        //! A method used to set the range of the sensor.
        /*!
         \param angularVelocityMax the maximum measured angular velocity for each axis [rad s^-1]
         \param linearAccelerationMax the maximum measured linear acceleration for each axis [m s^-2]
         */
        void setRange(Vector3 angularVelocityMax, Vector3 linearAccelerationMax);

        //! A method used to set the white noise of the sensor.
        /*!
         \param angularVelocityStdDev standard deviation of the angular velocity noise for each axis [rad s^-1]
         \param linearAccelerationStdDev standard deviation of the linear acceleration noise for each axis [m s^-2]
         */
        void setNoise(Vector3 angularVelocityStdDev, Vector3 linearAccelerationStdDev);

        //! A method used to enable or disable the Earth rotation effects (Earth rate and Coriolis).
        /*!
         \param enabled a flag indicating if the Earth rotation should be sensed
         */
        void setEarthRotation(bool enabled);

        //! A method used to set the bias model of the gyroscopes.
        /*!
         \param constant constant bias for each axis [rad s^-1]
         \param turnOnStdDev standard deviation of the random turn-on bias, drawn at every reset [rad s^-1]
         \param instability standard deviation of the Gauss-Markov bias (or random walk intensity [rad s^-1 s^-1/2] if tau <= 0)
         \param tau correlation time of the Gauss-Markov bias for each axis [s] (<= 0 -> random walk)
         */
        void setGyroBias(Vector3 constant, Vector3 turnOnStdDev = Vector3(0,0,0),
                         Vector3 instability = Vector3(0,0,0), Vector3 tau = Vector3(0,0,0));

        //! A method used to set the bias model of the accelerometers.
        /*!
         \param constant constant bias for each axis [m s^-2]
         \param turnOnStdDev standard deviation of the random turn-on bias, drawn at every reset [m s^-2]
         \param instability standard deviation of the Gauss-Markov bias (or random walk intensity [m s^-2 s^-1/2] if tau <= 0)
         \param tau correlation time of the Gauss-Markov bias for each axis [s] (<= 0 -> random walk)
         */
        void setAccBias(Vector3 constant, Vector3 turnOnStdDev = Vector3(0,0,0),
                        Vector3 instability = Vector3(0,0,0), Vector3 tau = Vector3(0,0,0));

        //! A method returning the current total gyroscope bias (ground truth) [rad s^-1].
        Vector3 getGyroBias() const;

        //! A method returning the current total accelerometer bias (ground truth) [m s^-2].
        Vector3 getAccBias() const;

        //! A method informing if the Earth rotation is sensed.
        bool isEarthRotationEnabled() const;

        //! A method returning the type of the scalar sensor.
        ScalarSensorType getScalarSensorType() const override;

    private:
        struct BiasModel
        {
            Vector3 constant;
            Vector3 turnOnStdDev;
            Vector3 instability;
            Vector3 tau;
            Vector3 turnOn;
            Vector3 dynamic;

            Vector3 total() const;
        };

        void ResetBias(BiasModel& bias);
        void PropagateBias(BiasModel& bias, Scalar dt);

        BiasModel gyroBias;
        BiasModel accBias;
        bool earthRotation;
        bool initialUpdate;
    };
}

#endif
