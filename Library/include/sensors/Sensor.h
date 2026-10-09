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
//  Copyright (c) 2013-2026 Patryk Cieslak. All rights reserved.
//

#pragma once

#include <random>
#include <SDL2/SDL_mutex.h>
#include "StonefishCommon.h"
#include "core/ConstructInfo.h"

namespace sf
{
    //! An enum defining types of sensors.
    enum class SensorType {JOINT, LINK, VISION, OTHER};
    
    //! Required structures.
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
        Sensor(const std::string& uniqueName, Scalar frequency);
        
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
        bool IsNewDataAvailable() const;
        
        //! A method to set the sampling rate of the sensor.
        /*!
         \param f the sampling frequency of the sensor [Hz]
         */
        void SetUpdateFrequency(Scalar f);

        //! A method returning the sensor's name.
        const std::string& GetName() const;

        //! A method returning the sampling rate of the sensor.
        Scalar GetUpdateFrequency() const;
        
        //! A method informing if the sensor is enabled.
        bool IsEnabled() const;

        //! A method informing if the sensor is renderable.
        bool IsRenderable() const;
        
        //! A method to set if the sensor is enabled.
        void SetEnabled(bool en);

        //! A method to set if the sensor is renderable.
        void SetRenderable(bool render);

        //! A method to set the visual representation of the sensor.
        void SetVisual(const std::string& meshFilename, Scalar scale, const std::string& look);
                
        //! A method performing an internal update of the sensor state.
        /*!
         \param dt the sampling time of the simulation [s]
         */
        virtual void InternalUpdate(Scalar dt) = 0;
        
        //! A method returning the type of the sensor.
        virtual SensorType GetType() const = 0;

        //! A method returning the sensor measurement frame.
        virtual Transform GetSensorFrame() const = 0;

        //! A method returning the velocity of the sensor measurement frame.
        /*!
         \param linear output of the linear velocity of the sensor measurement frame [m/s]
         \param angular output of the angular velocity of the sensor measurement frame [rad/s]
         */
        virtual void GetSensorVelocity(Vector3& linear, Vector3& angular) const = 0;

        //! A deleter method required for the plugin architecture.
        static void DefaultDeleter(Sensor* s);
        
    protected:
        Scalar freq_;
        SDL_mutex* updateMutex_;
        
        static std::random_device randomDevice;
        static std::mt19937 randomGenerator;
        
    private:
        std::string name_;
        Scalar eleapsedTime_;
        bool newDataAvailable_;
        bool renderable_;
        bool enabled_;
        int lookId_;
        int graObjectId_;
    };

    using SensorDeleter = void(*)(Sensor*);
    typedef Sensor* (*CreateSensorFunc)(const char*, Scalar);
    typedef void (*DestroySensorFunc)(Sensor*);
}
