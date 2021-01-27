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
//  Copyright (c) 2013-2019 Patryk Cieslak. All rights reserved.
//

#include "sensors/ScalarSensor.h"

#include "core/SimulationApp.h"
#include "core/SimulationManager.h"
#include "utils/ScientificFileUtil.h"
#include "sensors/Sample.h"

namespace sf
{

ScalarSensor::ScalarSensor(std::string uniqueName, Scalar frequency, int historyLength) : Sensor(uniqueName, frequency)
{
    historyLen = historyLength;
    history = std::deque<Sample*>(0);
}

ScalarSensor::~ScalarSensor()
{
    ClearHistory();
    channels.clear();
}

Sample ScalarSensor::getLastSample()
{
    if(history.size() > 0)
        return Sample(*history.back());
    else
    {
        unsigned short chs = getNumOfChannels();
        Scalar values[chs];
        memset(values, 0, sizeof(Scalar) * chs);
        return Sample(chs, values);
    }
}

const std::vector<Sample>* ScalarSensor::getHistory()
{
    SDL_LockMutex(updateMutex);
    
    std::vector<Sample>* historyCopy = new std::vector<Sample>();
    for(size_t i=0; i<history.size(); ++i)
        (*historyCopy).push_back(*(history[i]));
    
    SDL_UnlockMutex(updateMutex);
    
    return historyCopy;
}

unsigned short ScalarSensor::getNumOfChannels()
{
    return channels.size();
}

Scalar ScalarSensor::getValue(unsigned long int index, unsigned int channel)
{
    if(index < history.size() && channel < channels.size())
    {
        Sample* s = history[index];
        Scalar v = s->getValue(channel);
        return v;
    }
    
    return Scalar(0);
}

Scalar ScalarSensor::getLastValue(unsigned int channel)
{
    return getValue(history.size() - 1, channel);
}

SensorChannel ScalarSensor::getSensorChannelDescription(unsigned int channel)
{
    if(channel < channels.size())
        return channels[channel];
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
    if(historyLen < 0 && history.size() > 0) //No history
    {
        delete history[0];
        history.pop_front();
    }
    else if(historyLen > 0 && (int)history.size() == historyLen) //Specified history length
    {
        delete history[0];
        history.pop_front();
    }
    //else == 0 --> unlimited history
    
    Sample* sample = new Sample(s);
    
    for(unsigned int i=0; i<sample->getNumOfDimensions(); ++i)
    {
        Scalar* data = sample->getDataPointer();
        
        //Add noise
        if(channels[i].stdDev > Scalar(0))
            data[i] += channels[i].noise(randomGenerator);
    
        //Limit readings
        if(data[i] > channels[i].rangeMax)
            data[i] = channels[i].rangeMax;
        else if(data[i] < channels[i].rangeMin)
            data[i] = channels[i].rangeMin;
    }
    
    //Add to history
    history.push_back(sample);
}

void ScalarSensor::ClearHistory()
{
    for(unsigned int i = 0; i < history.size(); i++)
        delete history[i];
    
    history.clear();
}

void ScalarSensor::SaveMeasurementsToTextFile(const std::string& path, bool includeTime, unsigned int fixedPrecision)
{
    if(history.size() == 0)
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
    fprintf(fp, "#Number of channels: %ld\n", channels.size());
    fprintf(fp, "#Number of samples: %ld\n", history.size());
    if(freq <= Scalar(0.))
        fprintf(fp, "#Frequency: %1.3lf Hz\n", SimulationApp::getApp()->getSimulationManager()->getStepsPerSecond());
    else
        fprintf(fp, "#Frequency: %1.3lf Hz\n", freq);
    fprintf(fp, "#Unit system: SI\n\n");
    
    //Write data header
    if(includeTime)
        fprintf(fp, "#Time\t");
    else
        fprintf(fp, "#");
    
    for(unsigned int i = 0; i < channels.size(); i++)
    {
        fprintf(fp, "%s", channels[i].name.c_str());
        
        if(i < channels.size() - 1)
            fprintf(fp, "\t");
        else
            fprintf(fp, "\n");
    }
    
    //Write data
    std::string format = "%1." + std::to_string(fixedPrecision) + "lf";
    
    for(unsigned int i = 0; i < history.size(); i++)
    {
        Sample* s = history[i];
        
        if(includeTime)
        {
            fprintf(fp, format.c_str(), s->getTimestamp());
            fprintf(fp, "\t");
        }
        
        for(unsigned int h = 0; h < channels.size(); h++)
        {
            Scalar v = s->getValue(h);
            
            fprintf(fp, format.c_str(), v);
            
            if(h < channels.size() - 1)
                fprintf(fp, "\t");
            else
                fprintf(fp, "\n");
        }
    }
    
    fclose(fp);
}

void ScalarSensor::SaveMeasurementsToOctaveFile(const std::string& path, bool includeTime, bool separateChannels)
{
    if(history.size() == 0)
        return;
    
    //build data structure
    ScientificData data("");
    
    if(separateChannels) //channels separated and saved in vecotors
    {
        if(includeTime) //write time vector
        {
            ScientificDataItem* it = new ScientificDataItem();
            it->name = "Time";
            it->type = DATA_VECTOR;
            
            btVectorXu* vector = new btVectorXu((unsigned int)history.size());
            it->value = vector;
            
            for(unsigned int i = 0; i < history.size(); ++i)
            {
                Sample* s = history[i];
                (*vector)[i] = s->getTimestamp();
            }
            
            data.addItem(it);
        }
        
        //write channel vectors
        for(unsigned int i = 0; i < channels.size(); ++i)
        {
            ScientificDataItem* it = new ScientificDataItem();
            it->name = channels[i].name;
            it->type = DATA_VECTOR;
            
            btVectorXu* vector = new btVectorXu((unsigned int)history.size());
            it->value = vector;
            
            for(unsigned int h = 0; h < history.size(); ++h)
            {
                Sample* s = history[h];
                Scalar v = s->getValue(i);
                (*vector)[h] = v;
            }
            
            data.addItem(it);
        }
    }
    else //channels combined in a matrix
    {
        ScientificDataItem* it = new ScientificDataItem();
        it->name = getName();
        it->type = DATA_MATRIX;
        
        btMatrixXu* matrix = new btMatrixXu((unsigned int)history.size(), (unsigned int)channels.size() + (includeTime ? 1 : 0));
        it->value = matrix;
        
        for(unsigned int i = 0; i < history.size(); ++i)
        {
            Sample* s = history[i];
            
            if(includeTime)
                matrix->setElem(i, 0, s->getTimestamp());
            
            for(unsigned int h = 0; h < channels.size(); ++h)
            {
                Scalar v = s->getValue(h);
                matrix->setElem(i, h + (includeTime ? 1 : 0), v);
            }
        }
        
        data.addItem(it);
    }
    
    //save data structure to file
    SaveOctaveData(path, data);
}

}
