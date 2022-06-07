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
//  INS.h
//  Stonefish
//
//  Created by Patryk Cieslak on 21/10/2021.
//  Copyright (c) 2021 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_INS__
#define __Stonefish_INS__

#include "sensors/scalar/LinkSensor.h"
#include "sensors/scalar/GPS.h"
#include "sensors/scalar/Pressure.h"
#include "sensors/scalar/DVL.h"

namespace sf
{
    //! A class representing an inertial navigation system (INS).
    class INS : public LinkSensor
    {
    public:
        //! A constructor.
        /*!
         \param uniqueName a name for the sensor
         \param frequency the sampling frequency of the sensor [Hz] (-1 if updated every simulation step)
         \param historyLength defines: -1 -> no history, 0 -> unlimited history, >0 -> history with a specified length
         */
        INS(std::string uniqueName, Scalar frequency = Scalar(-1), int historyLength = -1);
    
        //! A method performing internal sensor state update.
        /*!
         \param dt the step time of the simulation [s]
         */
        void InternalUpdate(Scalar dt);

        //! A method that resets the sensor.
        void Reset();

        //! A method used to connect a GPS to the INS.
        /*!
         \param name a unique name identifying the sensor
         */
        void ConnectGPS(const std::string& name);

        //! A method used to connect a pressure sensor to the INS.
        /*!
         \param name a unique name identifying the sensor
         */
        void ConnectPressure(const std::string& name);

        //! A method used to connect a DVL to the INS.
        /*!
         \param name a unique name identifying the sensor
         */
        void ConnectDVL(const std::string& name);

        //! A method used to set the output frame (in device frame).
        /*!
         \param T a transformation from the device origin to the output frame
         */
        void setOutputFrame(const Transform& T);

        //! A method used to set the range of the sensor.
        /*!
         \param angularVelocityMax the maximum measured angular velocity for each axis [rad s^-1]
         \param linearAccelerationMax the maximum measured linear acceleration for each axis [m s^-2]
         */
        void setRange(Vector3 angularVelocityMax, Vector3 linearAccelerationMax);
        
        //! A method used to set the noise characteristics of the sensor.
        /*!
         \param angularVelocityStdDev standard deviation of the angular velocity measurement noise for each axis
         \param linearAccelerationStdDev the standard deviation of the acceleration measurement for each axis
         */
        void setNoise(Vector3 angularVelocityStdDev, Vector3 linearAccelerationStdDev);
        
        //! A method rendering the sensor representation.
        std::vector<Renderable> Render();

        //! A method returning the type of the scalar sensor.
        ScalarSensorType getScalarSensorType();

        private:
            Scalar latitude, longitude, altitude;
            Vector3 ned;
            Vector3 velocity;
            Transform out;
            std::string gpsName;
            std::string dvlName;
            std::string pressName;
            std::normal_distribution<Scalar> accNoiseX;
            std::normal_distribution<Scalar> accNoiseY;
            std::normal_distribution<Scalar> accNoiseZ;
            std::normal_distribution<Scalar> avNoiseX;
            std::normal_distribution<Scalar> avNoiseY;
            std::normal_distribution<Scalar> avNoiseZ;
            bool imuNoise;
    };
}

#endif
