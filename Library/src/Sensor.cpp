//
//  Sensor.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 1/4/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#include "Sensor.h"

NameManager Sensor::nameManager;

Sensor::Sensor(std::string uniqueName, unsigned int historyLength)
{
    name = nameManager.AddName(uniqueName);
    historyLen = historyLength > 0 ? historyLength : 1;
    history = std::deque<std::unique_ptr<Sample>>(0);
}

Sensor::~Sensor()
{
    ClearHistory();
    nameManager.RemoveName(name);
}

void Sensor::AddSampleToHistory(const Sample& s)
{
    if(history.size() == historyLen)
        history.pop_front();
        
    history.push_back(std::unique_ptr<Sample>(new Sample(s)));
}

void Sensor::ClearHistory()
{
    history.clear();
}

//return a constant smart pointer to the last sample
const std::shared_ptr<Sample> Sensor::getLastSample()
{
    if(history.size() > 0)
        return std::shared_ptr<Sample>(history.back().get());
    else
        return nullptr;
}

//return a const reference to history
const std::deque<std::unique_ptr<Sample>>& Sensor::getHistory()
{
    return history;
}

std::string Sensor::getName()
{
    return name;
}
