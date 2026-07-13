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

ScalarSensor::ScalarSensor(const std::string& uniqueName, Scalar frequency, long int historyLength) 
    : Sensor(uniqueName, frequency), sampleCount_(0), historyLen_(historyLength)
{
}

const Sample& ScalarSensor::getLastSample() const
{
    if(history_.size() > 0)
        return *history_.back();
    else
        throw std::runtime_error("Sensor '" + getName() + "' has no measurement history");
}

std::unique_ptr<std::vector<Sample>> ScalarSensor::getHistory()
{
    SDL_LockMutex(updateMutex_);
    
    std::unique_ptr<std::vector<Sample>> historyCopy = std::make_unique<std::vector<Sample>>();
    for(size_t i=0; i<history_.size(); ++i)
        historyCopy->push_back(*history_[i]);
    
    SDL_UnlockMutex(updateMutex_);
    
    return historyCopy;
}

size_t ScalarSensor::getNumOfChannels() const
{
    return channels_.size();
}

Scalar ScalarSensor::getValue(size_t index, size_t channel) const
{
    if(index < history_.size() && channel < channels_.size())
        return history_[index]->getValue(channel);
    else
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

void ScalarSensor::AddSampleToHistory(std::unique_ptr<Sample> sample)
{
    if(historyLen_ < 0 && history_.size() > 0) //No history
    {
        history_.pop_front();
    }
    else if(historyLen_ > 0 && (int)history_.size() == historyLen_) //Specified history length
    {
        history_.pop_front();
    }
    //else == 0 --> unlimited history
    
    sample->setId(sampleCount_);
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
    history_.push_back(std::move(sample));
}

void ScalarSensor::ClearHistory()
{
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
        if(includeTime)
        {
            fprintf(fp, format.c_str(), history_[i]->getTimestamp());
            fprintf(fp, "\t");
        }
        
        for(size_t h = 0; h < channels_.size(); h++)
        {
            Scalar v = history_[i]->getValue(h);
            
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
