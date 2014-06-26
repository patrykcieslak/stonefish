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

//abstract class
class Sensor
{
public:
    Sensor(std::string uniqueName, btScalar frequency = btScalar(-1.), unsigned int historyLength = 0);
    virtual ~Sensor();
    
    virtual void InternalUpdate(btScalar dt) = 0;
    virtual unsigned short getNumOfDimensions() = 0;
    virtual void Reset();
    virtual void Render();
    
    void Update(btScalar dt);
    void ClearHistory();
    bool isRenderable();
    void setRenderable(bool render);
    Sample getLastSample();
    const std::deque<Sample*>& getHistory();
    std::string getName();
    
protected:
    void AddSampleToHistory(const Sample& s);
    std::deque<Sample*> history;
    btScalar freq;
    
private:
    std::string name;
    unsigned int historyLen;
    btScalar eleapsedTime;
    bool renderable;
    
    static NameManager nameManager;
};

#endif
