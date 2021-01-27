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
//  ScalarSensor.h
//  Stonefish
//
//  Created by Patryk Cieślak on 20/11/2018.
//  Copyright (c) 2018-2021 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_ScalarSensor__
#define __Stonefish_ScalarSensor__

#include <deque>
#include "sensors/Sensor.h"

namespace sf
{
    //! An enum defining types of scalar sensors.
    enum class ScalarSensorType {ACC, CURRENT, DVL, COMPASS, FT, GPS, GYRO, IMU, MULTIBEAM, ODOM, PRESSURE, PROFILER, ENCODER, TORQUE, POSE};
    
    //! An enum defining the type of quantity represented by the measurement.
    enum class QuantityType
    {
        UNITLESS,
        LENGTH,
        ANGLE,
        VELOCITY,
        ANGULAR_VELOCITY,
        ACCELERATION,
        FORCE,
        TORQUE,
        CURRENT,
        PRESSURE,
        INVALID
    };
    
    //! A structure desribing a single channel of the sensor.
    struct SensorChannel
    {
        std::string name;
        QuantityType type;
        Scalar stdDev;
        std::normal_distribution<Scalar> noise;
        Scalar rangeMin;
        Scalar rangeMax;
        
        //! A constructor.
        /*!
         \param name_ a name for the channel
         \param type_ a type of quantity represented by channel data
         */
        SensorChannel(std::string name_, QuantityType type_) : name(name_), type(type_), stdDev(0), rangeMin(-BT_LARGE_FLOAT), rangeMax(BT_LARGE_FLOAT) {}
        
        //! A method used to set standard deviation of the measurement
        /*!
         \param sd the value of the standard deviation
         */
        void setStdDev(Scalar sd)
        {
            if(sd > Scalar(0))
            {
                stdDev = sd;
                noise = std::normal_distribution<Scalar>(Scalar(0), stdDev);
            }
        }
    };
    
    class Sample;
    
    //! An abstract class representing a scalar sensor.
    class ScalarSensor : public Sensor
    {
    public:
        //! A constructor.
        /*!
         \param uniqueName a name for the sensor
         \param frequency the sampling frequency of the sensor [Hz] (0 if updated every simulation step)
         \param historyLength defines: -1 -> no history, 0 -> unlimited history, >0 -> history with a specified length
         */
        ScalarSensor(std::string uniqueName, Scalar frequency, int historyLength);
        
        //! A destructor.
        virtual ~ScalarSensor();
        
        //! A method performing internal sensor state update.
        /*!
         \param dt the step time of the simulation [s]
         */
        virtual void InternalUpdate(Scalar dt) = 0;
        
        //! A method returning the type of the sensor.
        virtual SensorType getType() = 0;
        
        //! A method resetting the sensor.
        virtual void Reset();
        
        //! A method clearing the history of measurements.
        void ClearHistory();
        
        //! A method used to save the measurements to a text file.
        /*!
         \param path a path to the output file
         \param includeTime a flag specifying if the timestamp should be written
         \param fixedPrecision number of decimal places to write
         */
        void SaveMeasurementsToTextFile(const std::string& path, bool includeTime = true, unsigned int fixedPrecision = 6);
        
        //! A method used to save the measurements to an Octave file.
        /*!
         \param path a path to the output file
         \param includeTime a flag specifying if the timestamp should be written
         \param separateChannels
         */
        void SaveMeasurementsToOctaveFile(const std::string& path, bool includeTime = true, bool separateChannels = false);
        
        //! A method returning the number of channels of the sensor.
        unsigned short getNumOfChannels();
        
        //! A method returning the last sample.
        Sample getLastSample();
        
        //! A method returing a pointer to a copy of the history of sensor measurements.
        const std::vector<Sample>* getHistory();
        
        //! A method returning the value of the measurement.
        /*!
         \param index the index of the history
         \param channel the index of the channel
         \return value of the measurement
         */
        Scalar getValue(unsigned long int index, unsigned int channel);
        
        //! A method returning the last value of the measurement.
        /*!
         \param channel the index of the channel
         \return last value of the measurement
         */
        Scalar getLastValue(unsigned int channel);
        
        //! A method returning the description of a specified channel.
        /*!
         \param channel the infdex of the channel
         \return a structure discribing the channel
         */
        SensorChannel getSensorChannelDescription(unsigned int channel);
        
        //! A method returning the type of scalar sensor.
        virtual ScalarSensorType getScalarSensorType() = 0;
        
    protected:
        void AddSampleToHistory(const Sample& s);
        std::deque<Sample*> history;
        std::vector<SensorChannel> channels;
        
    private:
        int historyLen;
    };
}
    
#endif
