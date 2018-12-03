//
//  Sensor.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 5/6/17.
//  Copyright (c) 2017-2018 Patryk Cieslak. All rights reserved.
//

#include "sensors/Sensor.h"

#include "core/SimulationApp.h"
#include "core/SimulationManager.h"
#include "core/Console.h"
#include "graphics/OpenGLPipeline.h"
#include "utils/ScientificFileUtil.h"

namespace sf
{

std::random_device Sensor::randomDevice;
std::mt19937 Sensor::randomGenerator(randomDevice());

Sensor::Sensor(std::string uniqueName, Scalar frequency)
{
    name = SimulationApp::getApp()->getSimulationManager()->getNameManager()->AddName(uniqueName);
    freq = frequency == Scalar(0) ? Scalar(1) : frequency;
    eleapsedTime = Scalar(0);
    renderable = false;
    newDataAvailable = false;
}

Sensor::~Sensor()
{
	SimulationApp::getApp()->getSimulationManager()->getNameManager()->RemoveName(name);
}

std::string Sensor::getName()
{
    return name;
}

void Sensor::MarkDataOld()
{
    newDataAvailable = false;
}

void Sensor::setUpdateFrequency(Scalar f)
{
    freq = f == Scalar(0) ? Scalar(1) : f;
}

bool Sensor::isNewDataAvailable()
{
    return newDataAvailable;
}

void Sensor::setRenderable(bool render)
{
    renderable = render;
}

bool Sensor::isRenderable()
{
    return renderable;
}

void Sensor::Reset()
{
    eleapsedTime = Scalar(0.);
    InternalUpdate(1.); //time delta should not affect initial measurement!!!
}

void Sensor::Update(Scalar dt)
{
    if(freq <= Scalar(0.)) // Every simulation tick
    {
        InternalUpdate(dt);
        newDataAvailable = true;
    }
    else //Fixed rate
    {
        eleapsedTime += dt;
        Scalar invFreq = Scalar(1.)/freq;
        
        if(eleapsedTime >= invFreq)
        {
            InternalUpdate(invFreq);
            eleapsedTime -= invFreq;
            newDataAvailable = true;
        }
    }
}

std::vector<Renderable> Sensor::Render()
{
    std::vector<Renderable> items(0);
    return items;
}
    
}
