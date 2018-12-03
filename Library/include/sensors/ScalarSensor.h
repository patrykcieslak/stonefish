//
//  ScalarSensor.h
//  Stonefish
//
//  Created by Patryk Cieślak on 20/11/2018.
//  Copyright (c) 2018 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_ScalarSensor__
#define __Stonefish_ScalarSensor__

#include <deque>
#include "sensors/Sensor.h"

namespace sf
{
    //!
    typedef enum
    {
        QUANTITY_LENGTH = 0,
        QUANTITY_ANGLE,
        QUANTITY_VELOCITY,
        QUANTITY_ANGULAR_VELOCITY,
        QUANTITY_ACCELERATION,
        QUANTITY_FORCE,
        QUANTITY_TORQUE,
        QUANTITY_CURRENT,
        QUANTITY_PRESSURE,
        QUANTITY_UNITLESS,
        QUANTITY_INVALID
    }
    QuantityType;
    
    //!
    struct SensorChannel
    {
        std::string name;
        QuantityType type;
        Scalar stdDev;
        std::normal_distribution<Scalar> noise;
        Scalar rangeMin;
        Scalar rangeMax;
        
        SensorChannel(std::string name_, QuantityType type_) : name(name_), type(type_), stdDev(0), rangeMin(-BT_LARGE_FLOAT), rangeMax(BT_LARGE_FLOAT)
        {
        }
        
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
    
    //!
    class ScalarSensor : public Sensor
    {
    public:
        //HistoryLength: -1 --> no history, 0 --> unlimited history, >0 --> history with specified number of simulation steps
        ScalarSensor(std::string uniqueName, Scalar frequency, int historyLength);
        virtual ~ScalarSensor();
        
        virtual void InternalUpdate(Scalar dt) = 0;
        virtual SensorType getType() = 0;
        
        virtual void Reset();
        void ClearHistory();
        void SaveMeasurementsToTextFile(const char* path, bool includeTime = true, unsigned int fixedPrecision = 6);
        void SaveMeasurementsToOctaveFile(const char* path, bool includeTime = true, bool separateChannels = false);
        
        unsigned short getNumOfChannels();
        Sample getLastSample();
        const std::deque<Sample*>& getHistory();
        Scalar getValueExternal(unsigned long int index, unsigned int channel);
        Scalar getLastValueExternal(unsigned int channel);
        SensorChannel getSensorChannelDescription(unsigned int channel);
        
    protected:
        void AddSampleToHistory(const Sample& s);
        std::deque<Sample*> history;
        std::vector<SensorChannel> channels;
        
    private:
        int historyLen;
    };
}
    
#endif
