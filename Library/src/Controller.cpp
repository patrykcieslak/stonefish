//
//  Controller.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 1/4/13.
//  Copyright (c) 2013-2017 Patryk Cieslak. All rights reserved.
//

#include "Controller.h"

NameManager Controller::nameManager;

Controller::Controller(std::string uniqueName, btScalar frequency)
{
    name = nameManager.AddName(uniqueName);
    freq = frequency;
    eleapsedTime = btScalar(0.);
    runningTime = btScalar(0.);
    running = false;
}

Controller::~Controller()
{
    nameManager.RemoveName(name);
}

btScalar Controller::getFrequency()
{
    return freq;
}

btScalar Controller::getRunningTime()
{
    return runningTime;
}

std::string Controller::getName()
{
    return name;
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
        runningTime += dt;
        
        if(freq <= btScalar(0.)) //Every simulation tick
        {
            Tick(dt);
        }
        else //Fixed rate
        {
            eleapsedTime += dt;
            btScalar invFreq = btScalar(1.)/freq;
        
            if(eleapsedTime >= invFreq)
            {
                Tick(invFreq);
                eleapsedTime -= invFreq;
            }
        }
    }
}
