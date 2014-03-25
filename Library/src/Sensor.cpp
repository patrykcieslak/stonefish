//
//  Sensor.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 1/4/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#include "Sensor.h"

Sensor::Sensor(std::string uniqueName, uint historyLength)
{
    name = uniqueName;
    historyLen = historyLength > 0 ? historyLength : 1;
}

Sensor::~Sensor()
{
    ClearHistory();
}

void Sensor::AddSampleToHistory(const Sample& s)
{
    if(history.size() == historyLen)
        history.pop_front();
        
    history.push_back(std::shared_ptr<Sample>(new Sample(s)));
}

void Sensor::ClearHistory()
{
    history.clear();
}

//return a constant smart pointer to the last sample
const std::shared_ptr<Sample> Sensor::getLastSample()
{
    if(history.size() > 0)
        return history.back();
    else
    {
        ushort dim = getNumOfDimensions();
        btScalar sd[dim];
        std::memset(sd, 0, sizeof(btScalar)*dim);
        return std::shared_ptr<Sample>(new Sample(dim, sd));
    }
}

//return a constant smart pointer to the history
const std::shared_ptr<std::deque<std::shared_ptr<Sample>>> Sensor::getHistory()
{
    return std::shared_ptr<std::deque<std::shared_ptr<Sample>>>(&history);
}

//return a deep copy of the (part of) history
std::shared_ptr<std::vector<std::shared_ptr<Sample>>> Sensor::getHistory(unsigned long nLastSamples)
{
    unsigned long firstSampleId = 0;
    if(nLastSamples > 0 && nLastSamples < history.size())
        firstSampleId = history.size()-nLastSamples;
    
    std::shared_ptr<std::vector<std::shared_ptr<Sample>>> subHistory(new std::vector<std::shared_ptr<Sample>>());
    for(unsigned long i = firstSampleId; i < history.size(); i++)
    {
        std::shared_ptr<Sample> s(new Sample(*history[i].get()));
        subHistory->push_back(s);
    }
    return subHistory;
}

std::string Sensor::getName()
{
    return name;
}
