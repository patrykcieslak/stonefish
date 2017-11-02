//
//  Sensor.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 5/6/17.
//  Copyright (c) 2017 Patryk Cieslak. All rights reserved.
//

#include "Sensor.h"
#include "Console.h"
#include "SimulationApp.h"
#include "ScientificFileUtil.h"

NameManager Sensor::nameManager;
std::random_device Sensor::randomDevice;
std::mt19937 Sensor::randomGenerator(randomDevice());

Sensor::Sensor(std::string uniqueName, btScalar frequency)
{
    name = nameManager.AddName(uniqueName);
    freq = frequency;
    eleapsedTime = btScalar(0.);
    renderable = false;
}

Sensor::~Sensor()
{
    nameManager.RemoveName(name);
}

std::string Sensor::getName()
{
    return name;
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
    eleapsedTime = btScalar(0.);
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

std::vector<Renderable> Sensor::Render()
{
    std::vector<Renderable> items(0);
    return items;
}