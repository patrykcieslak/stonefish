//
//  Controller.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 1/4/13.
//  Copyright (c) 2013-2018 Patryk Cieslak. All rights reserved.
//

#include "controllers/Controller.h"

#include "core/SimulationApp.h"
#include "core/SimulationManager.h"

namespace sf
{

Controller::Controller(std::string uniqueName, Scalar frequency)
{
    name = SimulationApp::getApp()->getSimulationManager()->getNameManager()->AddName(uniqueName);
    freq = frequency;
    eleapsedTime = Scalar(0.);
    runningTime = Scalar(0.);
    running = false;
}

Controller::~Controller()
{
    SimulationApp::getApp()->getSimulationManager()->getNameManager()->RemoveName(name);
}

Scalar Controller::getFrequency()
{
    return freq;
}

Scalar Controller::getRunningTime()
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
    eleapsedTime = Scalar(0.);
}

void Controller::Stop()
{
    running = false;
}

void Controller::Update(Scalar dt)
{
    if(running)
    {
        runningTime += dt;
        
        if(freq <= Scalar(0.)) //Every simulation tick
        {
            Tick(dt);
        }
        else //Fixed rate
        {
            eleapsedTime += dt;
            Scalar invFreq = Scalar(1.)/freq;
        
            if(eleapsedTime >= invFreq)
            {
                Tick(invFreq);
                eleapsedTime -= invFreq;
            }
        }
    }
}

}
