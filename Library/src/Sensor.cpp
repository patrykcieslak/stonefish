//
//  Sensor.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 1/4/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#include "Sensor.h"
#include "Console.h"
#include "SimulationApp.h"

NameManager Sensor::nameManager;

#pragma mark Constructors
Sensor::Sensor(std::string uniqueName, btScalar frequency, unsigned int historyLength)
{
    name = nameManager.AddName(uniqueName);
    historyLen = historyLength;
    history = std::deque<Sample*>(0);
    freq = frequency;
    eleapsedTime = btScalar(0.);
    renderable = false;
}

#pragma mark - Destructor
Sensor::~Sensor()
{
    ClearHistory();
    nameManager.RemoveName(name);
    channels.clear();
}

#pragma mark - Accessors
Sample Sensor::getLastSample()
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

const std::deque<Sample*>& Sensor::getHistory()
{
    return history;
}

std::string Sensor::getName()
{
    return name;
}

unsigned short Sensor::getNumOfChannels()
{
    return channels.size();
}

btScalar Sensor::getValueExternal(unsigned long int index, unsigned int channel)
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

btScalar Sensor::getLastValueExternal(unsigned int channel)
{
    return getValueExternal(history.size() - 1, channel);
}

SensorChannel Sensor::getSensorChannelDescription(unsigned int channel)
{
    if(channel < channels.size())
        return channels[channel];
    else
        return SensorChannel("Invalid", QUANTITY_INVALID);
}

void Sensor::setRenderable(bool render)
{
    renderable = render;
}

bool Sensor::isRenderable()
{
    return renderable;
}

#pragma mark - Methods
void Sensor::Reset()
{
    eleapsedTime = btScalar(0.);
    ClearHistory();
    InternalUpdate(1.); //time delta should not affect initial measurement!!!
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

void Sensor::SaveMeasurementsToFile(const char* path, bool includeTime, unsigned int fixedPrecision)
{
    if(history.size() > 0)
    {
        FILE* fp = fopen(path, "wt");
        
        //Write header
        fprintf(fp, "#Measurements from %s\n", name.c_str());
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
        
        cInfo("Saved %s measurements to: %s\n", name.c_str(), path);
    }
}

void Sensor::Render()
{
}