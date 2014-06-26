//
//  Sensor.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 1/4/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#include "Sensor.h"

NameManager Sensor::nameManager;

Sensor::Sensor(std::string uniqueName, btScalar frequency, unsigned int historyLength)
{
    name = nameManager.AddName(uniqueName);
    historyLen = historyLength;
    history = std::deque<Sample*>(0);
    freq = frequency;
    eleapsedTime = btScalar(0.);
    renderable = false;
}

Sensor::~Sensor()
{
    ClearHistory();
    nameManager.RemoveName(name);
}

void Sensor::Reset()
{
    eleapsedTime = btScalar(0.);
}

void Sensor::Update(btScalar dt)
{
    if(freq <= btScalar(0.)) // Every simulation tick
    {
        InternalUpdate(dt);
    }
    else //Fixed rate
    {
        eleapsedTime += dt;
        btScalar invFreq = btScalar(1.)/freq;
    
        if(eleapsedTime >= invFreq)
        {
            InternalUpdate(invFreq);
            eleapsedTime -= invFreq;
        }
    }
}

void Sensor::AddSampleToHistory(const Sample& s)
{
    if(historyLen > 0 && history.size() == historyLen) // 0 means unlimited history
    {
        delete history[0];
        history.pop_front();
    }
    
    history.push_back(new Sample(s));
}

void Sensor::ClearHistory()
{
    for(int i = 0; i < history.size(); i++)
        delete history[i];
    
    history.clear();
}

//return a constant smart pointer to the last sample
Sample Sensor::getLastSample()
{
    if(history.size() > 0)
        return Sample(*history.back());
    else
    {
        unsigned short dim = getNumOfDimensions();
        btScalar values[dim];
        memset(values, 0, sizeof(btScalar) * dim);
        return Sample(dim, values);
    }
}

//return a const reference to history
const std::deque<Sample*>& Sensor::getHistory()
{
    return history;
}

std::string Sensor::getName()
{
    return name;
}

void Sensor::Render()
{
}

void Sensor::setRenderable(bool render)
{
    renderable = render;
}

bool Sensor::isRenderable()
{
    return renderable;
}
