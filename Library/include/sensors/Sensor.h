//
//  Sensor.h
//  Stonefish
//
//  Created by Patryk Cieslak on 1/4/13.
//  Copyright (c) 2013-2019 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Sensor__
#define __Stonefish_Sensor__

#include <random>
#include "StonefishCommon.h"

namespace sf
{
    //! An enum defining types of sensors.
    typedef enum {SENSOR_JOINT = 0, SENSOR_LINK, SENSOR_VISION, SENSOR_OTHER} SensorType;
    
    struct Renderable;
    
    //! An abstract class representing a sensor.
    class Sensor
    {
    public:
        //! A constructor.
        /*!
         \param uniqueName a name for the sensor
         \param frequency the sampling frequency of the sensor [Hz] (-1 if updated every simulation step)
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
        
        //! A method informing if the sensor is renderable.
        bool isRenderable();
        
        //! A method to set if the sensor is renderable.
        void setRenderable(bool render);
        
        //! A method returning the sensor name.
        std::string getName();
        
        //! A method performing an internal update of the sensor state.
        /*!
         \param dt the sampling time of the simulation [s]
         */
        virtual void InternalUpdate(Scalar dt) = 0;
        
        //! A method returning the type of the sensor.
        virtual SensorType getType() = 0;
        
    protected:
        Scalar freq;
        
        static std::random_device randomDevice;
        static std::mt19937 randomGenerator;
        
    private:
        std::string name;
        Scalar eleapsedTime;
        bool renderable;
        bool newDataAvailable;
    };
}

#endif
