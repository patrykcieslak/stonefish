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
//  ScalarSensor.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 1/4/13.
//  Copyright (c) 2013-2026 Patryk Cieslak. All rights reserved.
//

#include "sensors/ScalarSensor.h"

#include "core/SimulationApp.h"
#include "core/SimulationManager.h"
#include "sensors/Sample.h"

namespace sf
{

ScalarSensor::ScalarSensor(std::string uniqueName, Scalar frequency, int historyLength) : Sensor(uniqueName, frequency)
{
    historyLen_ = historyLength;
    history_ = std::deque<Sample*>(0);
    sampleCount_ = 0;
}

ScalarSensor::~ScalarSensor()
{
    ClearHistory();
    channels_.clear();
}

Sample ScalarSensor::getLastSample() const
{
    if(history_.size() > 0)
        return Sample(*history_.back());
    else
        return Sample(std::vector<Scalar>(getNumOfChannels(), Scalar(0)), true);
}

const std::vector<Sample>* ScalarSensor::getHistory()
{
    SDL_LockMutex(updateMutex_);
    
    std::vector<Sample>* historyCopy = new std::vector<Sample>();
    for(size_t i=0; i<history_.size(); ++i)
        (*historyCopy).push_back(*(history_[i]));
    
    SDL_UnlockMutex(updateMutex_);
    
    return historyCopy;
}

unsigned short ScalarSensor::getNumOfChannels() const
{
    return channels_.size();
}

Scalar ScalarSensor::getValue(size_t index, size_t channel) const
{
    if(index < history_.size() && channel < channels_.size())
    {
        Sample* s = history_[index];
        Scalar v = s->getValue(channel);
        return v;
    }
    
    return Scalar(0);
}

Scalar ScalarSensor::getLastValue(size_t channel) const
{
    return getValue(history_.size() - 1, channel);
}

SensorChannel ScalarSensor::getSensorChannelDescription(size_t channel) const
{
    if(channel < channels_.size())
        return channels_[channel];
    else
        return SensorChannel("Invalid", QuantityType::INVALID);
}

void ScalarSensor::Reset()
{
    ClearHistory();
    Sensor::Reset();
}

void ScalarSensor::AddSampleToHistory(const Sample& s)
{
    if(historyLen_ < 0 && history_.size() > 0) //No history
    {
        delete history_[0];
        history_.pop_front();
    }
    else if(historyLen_ > 0 && (int)history_.size() == historyLen_) //Specified history length
    {
        delete history_[0];
        history_.pop_front();
    }
    //else == 0 --> unlimited history
    
    Sample* sample = new Sample(s, sampleCount_);
    ++sampleCount_;
    
    for(size_t i=0; i<sample->getNumOfDimensions(); ++i)
    {
        Scalar* data = sample->getDataPointer();
        
        //Add noise
        if(channels_[i].stdDev > Scalar(0) && data[i] < channels_[i].rangeMax && data[i] > channels_[i].rangeMin)
            data[i] += channels_[i].noise(randomGenerator);
    
        //Limit readings
        if(data[i] > channels_[i].rangeMax)
            data[i] = channels_[i].rangeMax;
        else if(data[i] < channels_[i].rangeMin)
            data[i] = channels_[i].rangeMin;
    }
    
    //Add to history
    history_.push_back(sample);
}

void ScalarSensor::ClearHistory()
{
    for(size_t i = 0; i < history_.size(); i++)
        delete history_[i];
    
    history_.clear();
}

void ScalarSensor::SaveMeasurementsToTextFile(const std::string& path, bool includeTime, size_t fixedPrecision)
{
    if(history_.size() == 0)
        return;
    
    cInfo("Saving %s measurements to: %s", getName().c_str(), path.c_str());
    
    FILE* fp = fopen(path.c_str(), "wt");
    if(fp == NULL)
    {
        cError("File could not be opened!");
        return;
    }
    
    //Write header
    fprintf(fp, "#Measurements from %s\n", getName().c_str());
    fprintf(fp, "#Number of channels: %ld\n", channels_.size());
    fprintf(fp, "#Number of samples: %ld\n", history_.size());
    if(freq_ <= Scalar(0.))
        fprintf(fp, "#Frequency: %1.3lf Hz\n", SimulationApp::getApp()->getSimulationManager()->getStepsPerSecond());
    else
        fprintf(fp, "#Frequency: %1.3lf Hz\n", freq_);
    fprintf(fp, "#Unit system: SI\n\n");
    
    //Write data header
    if(includeTime)
        fprintf(fp, "#Time\t");
    else
        fprintf(fp, "#");
    
    for(size_t i = 0; i < channels_.size(); i++)
    {
        fprintf(fp, "%s", channels_[i].name.c_str());
        
        if(i < channels_.size() - 1)
            fprintf(fp, "\t");
        else
            fprintf(fp, "\n");
    }
    
    //Write data
    std::string format = "%1." + std::to_string(fixedPrecision) + "lf";
    
    for(size_t i = 0; i < history_.size(); i++)
    {
        Sample* s = history_[i];
        
        if(includeTime)
        {
            fprintf(fp, format.c_str(), s->getTimestamp());
            fprintf(fp, "\t");
        }
        
        for(size_t h = 0; h < channels_.size(); h++)
        {
            Scalar v = s->getValue(h);
            
            fprintf(fp, format.c_str(), v);
            
            if(h < channels_.size() - 1)
                fprintf(fp, "\t");
            else
                fprintf(fp, "\n");
        }
    }
    
    fclose(fp);
}

}
