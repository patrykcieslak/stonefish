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

typedef enum {X_AXIS, Y_AXIS, Z_AXIS} AxisType;

//abstract class
class Sensor
{
public:
    Sensor(std::string uniqueName, unsigned int historyLength);
    virtual ~Sensor();
    
    virtual void Reset() = 0;
    virtual void Update(btScalar dt) = 0;
    virtual unsigned short getNumOfDimensions() = 0;
    
    void ClearHistory();
    Sample getLastSample();
    const std::deque<Sample*>& getHistory();
    std::string getName();
    
protected:
    void AddSampleToHistory(const Sample& s);
    
private:
    std::string name;
    unsigned int historyLen;
    std::deque<Sample*> history;
    
    static NameManager nameManager;
};

#endif
