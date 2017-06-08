//
//  SimpleSensor.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 1/4/13.
//  Copyright (c) 2013-2017 Patryk Cieslak. All rights reserved.
//

#include "SimpleSensor.h"
#include "Console.h"
#include "SimulationApp.h"
#include "ScientificFileUtil.h"

SimpleSensor::SimpleSensor(std::string uniqueName, btScalar frequency, unsigned int historyLength) : Sensor(uniqueName, frequency)
{
    historyLen = historyLength;
    history = std::deque<Sample*>(0);
}

SimpleSensor::~SimpleSensor()
{
    ClearHistory();
    channels.clear();
}

SensorType SimpleSensor::getType()
{
	return SENSOR_SIMPLE;
}

Sample SimpleSensor::getLastSample()
{
    if(history.size() > 0)
        return Sample(*history.back());
    else
    {
        unsigned short chs = getNumOfChannels();
        btScalar values[chs];
        memset(values, 0, sizeof(btScalar) * chs);
        return Sample(chs, values);
    }
}

const std::deque<Sample*>& SimpleSensor::getHistory()
{
    return history;
}

unsigned short SimpleSensor::getNumOfChannels()
{
    return channels.size();
}

btScalar SimpleSensor::getValueExternal(unsigned long int index, unsigned int channel)
{
    if(index < history.size() && channel < channels.size())
    {
        Sample* s = history[index];
        btScalar v = s->getValue(channel);
        
        switch(channels[channel].type)
        {
            case QUANTITY_LENGTH:
                return UnitSystem::GetLength(v);
                
            case QUANTITY_ANGLE:
                return UnitSystem::GetAngle(v);
                
            case QUANTITY_VELOCITY:
                return UnitSystem::GetVelocity(v);
                
            case QUANTITY_ANGULAR_VELOCITY:
                return UnitSystem::GetAngularVelocity(v);
                
            case QUANTITY_ACCELERATION:
                return UnitSystem::GetAcceleration(v);
                
            case QUANTITY_FORCE:
                return UnitSystem::GetForce(v);
                
            case QUANTITY_TORQUE:
                return UnitSystem::GetTorque(v);
                
            case QUANTITY_PRESSURE:
                return UnitSystem::GetPressure(v);
                
            case QUANTITY_CURRENT:
            case QUANTITY_UNITLESS:
            case QUANTITY_INVALID:
                return v;
        }
    }
    else
        return btScalar(0.);

}

btScalar SimpleSensor::getLastValueExternal(unsigned int channel)
{
    return getValueExternal(history.size() - 1, channel);
}

SensorChannel SimpleSensor::getSensorChannelDescription(unsigned int channel)
{
    if(channel < channels.size())
        return channels[channel];
    else
        return SensorChannel("Invalid", QUANTITY_INVALID);
}

void SimpleSensor::Reset()
{
	ClearHistory();
    Sensor::Reset();
}

void SimpleSensor::AddSampleToHistory(const Sample& s)
{
    if(historyLen > 0 && history.size() == historyLen) // 0 means unlimited history
    {
        delete history[0];
        history.pop_front();
    }
    
    history.push_back(new Sample(s));
}

void SimpleSensor::ClearHistory()
{
    for(int i = 0; i < history.size(); i++)
        delete history[i];
    
    history.clear();
}

void SimpleSensor::SaveMeasurementsToTextFile(const char* path, bool includeTime, unsigned int fixedPrecision)
{
    if(history.size() == 0)
        return;
    
    cInfo("Saving %s measurements to: %s", getName().c_str(), path);
    
    FILE* fp = fopen(path, "wt");
    if(fp == NULL)
    {
        cError("File could not be opened!");
        return;
    }
    
    //Write header
    fprintf(fp, "#Measurements from %s\n", getName().c_str());
    fprintf(fp, "#Number of channels: %ld\n", channels.size());
    fprintf(fp, "#Number of samples: %ld\n", history.size());
    if(freq <= btScalar(0.))
        fprintf(fp, "#Frequency: %1.3lf Hz\n", SimulationApp::getApp()->getSimulationManager()->getStepsPerSecond());
    else
        fprintf(fp, "#Frequency: %1.3lf Hz\n", freq);
    fprintf(fp, "#Unit system: %s\n\n", UnitSystem::GetDescription().c_str());
    
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
            btScalar v = s->getValue(h);
            
            switch(channels[h].type)
            {
                case QUANTITY_LENGTH:
                    v = UnitSystem::GetLength(v);
                    break;
                    
                case QUANTITY_ANGLE:
                    v = UnitSystem::GetAngle(v);
                    break;
                    
                case QUANTITY_VELOCITY:
                    v = UnitSystem::GetVelocity(v);
                    break;
                    
                case QUANTITY_ANGULAR_VELOCITY:
                    v = UnitSystem::GetAngularVelocity(v);
                    break;
                    
                case QUANTITY_ACCELERATION:
                    v = UnitSystem::GetAcceleration(v);
                    break;
                    
                case QUANTITY_FORCE:
                    v = UnitSystem::GetForce(v);
                    break;
                    
                case QUANTITY_TORQUE:
                    v = UnitSystem::GetTorque(v);
                    break;
                    
                case QUANTITY_PRESSURE:
                    v =  UnitSystem::GetPressure(v);
                    break;
                    
                case QUANTITY_CURRENT:
                case QUANTITY_UNITLESS:
                case QUANTITY_INVALID:
                    break;
            }
            
            fprintf(fp, format.c_str(), v);
            
            if(h < channels.size() - 1)
                fprintf(fp, "\t");
            else
                fprintf(fp, "\n");
        }
    }
    
    fclose(fp);
}

void SimpleSensor::SaveMeasurementsToOctaveFile(const char* path, bool includeTime, bool separateChannels)
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
                btScalar v = s->getValue(i);
                
                switch(channels[i].type)
                {
                    case QUANTITY_LENGTH:
                        v = UnitSystem::GetLength(v);
                        break;
                        
                    case QUANTITY_ANGLE:
                        v = UnitSystem::GetAngle(v);
                        break;
                        
                    case QUANTITY_VELOCITY:
                        v = UnitSystem::GetVelocity(v);
                        break;
                        
                    case QUANTITY_ANGULAR_VELOCITY:
                        v = UnitSystem::GetAngularVelocity(v);
                        break;
                        
                    case QUANTITY_ACCELERATION:
                        v = UnitSystem::GetAcceleration(v);
                        break;
                        
                    case QUANTITY_FORCE:
                        v = UnitSystem::GetForce(v);
                        break;
                        
                    case QUANTITY_TORQUE:
                        v = UnitSystem::GetTorque(v);
                        break;
                        
                    case QUANTITY_PRESSURE:
                        v =  UnitSystem::GetPressure(v);
                        break;
                        
                    case QUANTITY_CURRENT:
                    case QUANTITY_UNITLESS:
                    case QUANTITY_INVALID:
                        break;
                }
                
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
                btScalar v = s->getValue(h);
                
                switch(channels[h].type)
                {
                    case QUANTITY_LENGTH:
                        v = UnitSystem::GetLength(v);
                        break;
                        
                    case QUANTITY_ANGLE:
                        v = UnitSystem::GetAngle(v);
                        break;
                        
                    case QUANTITY_VELOCITY:
                        v = UnitSystem::GetVelocity(v);
                        break;
                        
                    case QUANTITY_ANGULAR_VELOCITY:
                        v = UnitSystem::GetAngularVelocity(v);
                        break;
                        
                    case QUANTITY_ACCELERATION:
                        v = UnitSystem::GetAcceleration(v);
                        break;
                        
                    case QUANTITY_FORCE:
                        v = UnitSystem::GetForce(v);
                        break;
                        
                    case QUANTITY_TORQUE:
                        v = UnitSystem::GetTorque(v);
                        break;
                        
                    case QUANTITY_PRESSURE:
                        v =  UnitSystem::GetPressure(v);
                        break;
                        
                    case QUANTITY_CURRENT:
                    case QUANTITY_UNITLESS:
                    case QUANTITY_INVALID:
                        break;
                }
                
                matrix->setElem(i, h + (includeTime ? 1 : 0), v);
            }
        }
        
        data.addItem(it);
    }
    
    //save data structure to file
    SaveOctaveData(path, data);
}

void SimpleSensor::Render()
{
}