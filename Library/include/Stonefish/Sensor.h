//
//  Sensor.h
//  Stonefish
//
//  Created by Patryk Cieslak on 1/4/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Sensor__
#define __Stonefish_Sensor__

#include "UnitSystem.h"
#include "NameManager.h"
#include "Sample.h"

typedef enum {QUANTITY_LENGTH, QUANTITY_ANGLE, QUANTITY_VELOCITY, QUANTITY_ANGULAR_VELOCITY, QUANTITY_ACCELERATION,
              QUANTITY_FORCE, QUANTITY_TORQUE, QUANTITY_CURRENT, QUANTITY_PRESSURE, QUANTITY_UNITLESS, QUANTITY_INVALID} QuantityType;

struct SensorChannel
{
    std::string name;
    QuantityType type;
    
    SensorChannel(std::string name_, QuantityType type_) : name(name_), type(type_) {}
};

//abstract class
class Sensor
{
public:
    Sensor(std::string uniqueName, btScalar frequency = btScalar(-1.), unsigned int historyLength = 0);
    virtual ~Sensor();
    
    virtual void InternalUpdate(btScalar dt) = 0;
    virtual void Reset();
    virtual void Render();
    void Update(btScalar dt);
    void ClearHistory();
    void SaveMeasurementsToFile(const char* path, bool includeTime = true, unsigned int fixedPrecision = 6);
    
    unsigned short getNumOfChannels();
    bool isRenderable();
    void setRenderable(bool render);
    Sample getLastSample();
    const std::deque<Sample*>& getHistory();
    btScalar getValueExternal(unsigned long int index, unsigned int channel);
    btScalar getLastValueExternal(unsigned int channel);
    SensorChannel getSensorChannelDescription(unsigned int channel);
    std::string getName();
    
protected:
    void AddSampleToHistory(const Sample& s);
    std::deque<Sample*> history;
    std::vector<SensorChannel> channels;
    btScalar freq;
    
private:
    std::string name;
    unsigned int historyLen;
    btScalar eleapsedTime;
    bool renderable;
    
    static NameManager nameManager;
};

#endif
