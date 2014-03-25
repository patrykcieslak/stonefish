//
//  Sensor.h
//  Stonefish
//
//  Created by Patryk Cieslak on 1/4/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Sensor__
#define __Stonefish_Sensor__

#include "common.h"
#include "Sample.h"

typedef enum {X_AXIS, Y_AXIS, Z_AXIS} AxisType;

//abstract class
class Sensor
{
public:
    Sensor(std::string uniqueName, uint historyLength);
    virtual ~Sensor();
    
    virtual void Update(btScalar dt) = 0;
    virtual ushort getNumOfDimensions() = 0;
    
    void ClearHistory();
    const std::shared_ptr<Sample> getLastSample();
    const std::shared_ptr<std::deque<std::shared_ptr<Sample>>> getHistory();
    std::shared_ptr<std::vector<std::shared_ptr<Sample>>> getHistory(unsigned long nLastSamples = 0);
    std::string getName();
    
protected:
    void AddSampleToHistory(const Sample& s);
    
private:
    std::string name;
    uint historyLen;
    std::deque<std::shared_ptr<Sample>> history;
};

#endif
