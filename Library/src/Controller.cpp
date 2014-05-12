//
//  Controller.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 1/4/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#include "Controller.h"

NameManager Controller::nameManager;

Controller::Controller(std::string uniqueName, btScalar frequency)
{
    name = nameManager.AddName(uniqueName);
    freq = frequency;
    running = false;
}

Controller::~Controller()
{
    nameManager.RemoveName(name);
}

void Controller::Start()
{
    running = true;
    eleapsedTime = btScalar(0.);
}

void Controller::Stop()
{
    running = false;
}

void Controller::Update(btScalar dt)
{
    if(running)
    {
        eleapsedTime += dt;
        btScalar invFreq = btScalar(1.)/freq;
        
        if(eleapsedTime >= invFreq)
        {
            Tick();
            eleapsedTime -= invFreq;
        }
    }
}

std::string Controller::getName()
{
    return name;
}

btScalar Controller::getFrequency()
{
    return freq;
}

