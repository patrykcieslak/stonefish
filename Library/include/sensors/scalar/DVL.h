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
//  DVL.h
//  Stonefish
//
//  Created by Patryk Cieslak on 30/10/2017.
//  Copyright (c) 2017-2021 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_DVL__
#define __Stonefish_DVL__

#include "sensors/scalar/LinkSensor.h"

namespace sf
{
    //! A class representing a Dopple velocity log (DVL) sensor with four beams.
    class DVL : public LinkSensor
    {
    public:
        //! A constructor.
        /*!
         \param uniqueName a name for the sensor
         \param beamAngleDeg the angle between the beams and the vertical axis [deg]
         \param beamPositiveZ a flag specifying if beams are pointing in the same direction as the z axis of the sensor
         \param frequency the sampling frequency of the sensor [Hz] (-1 if updated every simulation step)
         \param historyLength defines: -1 -> no history, 0 -> unlimited history, >0 -> history with a specified length
         */
        DVL(std::string uniqueName, Scalar beamAngleDeg, bool beamPositiveZ, Scalar frequency = Scalar(-1), int historyLength = -1);
        
        //! A method performing internal sensor state update.
        /*!
         \param dt the step time of the simulation [s]
         */
        void InternalUpdate(Scalar dt);
        
        //! A method used to set the range of the sensor.
        /*!
         \param velocityMax the maximum measured linear velocity [m s^-1]
         \param altitudeMin the minimum measured altitude (blanking) [m]
         \param altitudeMax the maximum measured altitude [m]
         */
        void setRange(const Vector3& velocityMax, Scalar altitudeMin, Scalar altitudeMax);
        
        //! A method used to set the water mass ping parameters.
        /*!
         \param minSize the minimum thickness of the water layer [m]
         \param nearBoundary the distance from the sensor to the near layer boundary [m]
         \param farBoundary the distance from the sensor to the far layer boundary [m]
         */
        void setWaterLayer(Scalar minSize, Scalar nearBoundary, Scalar farBoundary);

        //! A method used to set the noise characteristics of the sensor.
        /*!
         \param velPercent multiplicative bottom velocity mesurement noise factor as percent of velocity
         \param velStdDev standard deviation of the bottom velocity measurement noise
         \param altitudeStdDev standard deviation of the altitude measurement noise
         \param waterVelPercent multiplicative water velocity mesurement noise factor as percent of water velocity
         \param waterVelStdDev standard deviation of the water velocity measurement noise
         */
        void setNoise(Scalar velPercent, Scalar velStdDev, Scalar altitudeStdDev, Scalar waterVelPercent, Scalar waterVelStdDev);
        
        //! A method that returns the measurement ranges of the DVL.
        /*!
         \param velocityMax the output variable to store the maximum measured linear velocity [m s^-1]
         \param altitudeMin the output variable to store the minimum measured altitude (blanking) [m]
         \param altitudeMax the output variable to store the maximum measured altitude [m]
         */
        void getRange(Vector3& velocityMax, Scalar& altitudeMin, Scalar& altitudeMax) const;

        //! A method that return the angle between the beams and the vertical axis.
        Scalar getBeamAngle() const;

        //! A method rendering the sensor representation.
        std::vector<Renderable> Render();
        
        //! A method returning the type of the scalar sensor.
        ScalarSensorType getScalarSensorType();
        
    private:
        Scalar beamAngle;
        bool beamPosZ;
        Scalar range[4];
        Vector3 waterLayer;
        Scalar addNoiseStdDev[2]; //Additive noise
        Scalar mulNoiseFactor[2]; //Noise dependent on distance
    };
}

#endif
