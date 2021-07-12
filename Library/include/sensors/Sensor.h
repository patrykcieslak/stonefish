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
//  Sensor.h
//  Stonefish
//
//  Created by Patryk Cieslak on 1/4/13.
//  Copyright (c) 2013-2021 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Sensor__
#define __Stonefish_Sensor__

#include <random>
#include <SDL2/SDL_mutex.h>
#include "StonefishCommon.h"

namespace sf
{
    //! An enum defining types of sensors.
    enum class SensorType {JOINT, LINK, VISION, OTHER};
    
    struct Renderable;
    
    //! An abstract class representing a sensor.
    class Sensor
    {
    public:
        //! A constructor.
        /*!
         \param uniqueName a name for the sensor
         \param frequency the sampling frequency of the sensor [Hz] (0 if updated every simulation step)
         */
        Sensor(std::string uniqueName, Scalar frequency);
        
        //! A destructor.
        virtual ~Sensor();
        
        //! A method that resets the sensor.
        virtual void Reset();
        
        //! A method implementing the rendering of the sensor.
        virtual std::vector<Renderable> Render();
        
        //! A method that updates the sensor readings.
        /*!
         \param dt a time step of the simulation [s]
         */
        void Update(Scalar dt);
        
        //! A method used to mark data as old.
        void MarkDataOld();
        
        //! A method to check if new data is available.
        bool isNewDataAvailable();
        
        //! A method to set the sampling rate of the sensor.
        /*!
         \param f the sampling frequency of the sensor [Hz]
         */
        void setUpdateFrequency(Scalar f);

        //! A method returning the sampling rate of the sensor.
        Scalar getUpdateFrequency();
        
        //! A method informing if the sensor is renderable.
        bool isRenderable();
        
        //! A method to set if the sensor is renderable.
        void setRenderable(bool render);

        //! A method to set the visual representation of the sensor.
        void setVisual(const std::string& meshFilename, Scalar scale, const std::string& look);
        
        //! A method returning the sensor name.
        std::string getName();
        
        //! A method performing an internal update of the sensor state.
        /*!
         \param dt the sampling time of the simulation [s]
         */
        virtual void InternalUpdate(Scalar dt) = 0;
        
        //! A method returning the type of the sensor.
        virtual SensorType getType() = 0;

        //! A method returning the sensor measurement frame.
        virtual Transform getSensorFrame() const = 0;
        
    protected:
        Scalar freq;
        SDL_mutex* updateMutex;
        
        static std::random_device randomDevice;
        static std::mt19937 randomGenerator;
        
    private:
        std::string name;
        Scalar eleapsedTime;
        bool newDataAvailable;
        bool renderable;
        int lookId;
        int graObjectId;
    };
}

#endif
