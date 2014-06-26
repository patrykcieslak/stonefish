//
//  Controller.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 1/4/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#include "Controller.h"

NameManager Controller::nameManager;

#pragma mark Constructors
Controller::Controller(std::string uniqueName, btScalar frequency)
{
    name = nameManager.AddName(uniqueName);
    freq = frequency;
    running = false;
}

#pragma mark - Destructor
Controller::~Controller()
{
    nameManager.RemoveName(name);
}

#pragma mark - Accessors
std::string Controller::getName()
{
    return name;
}

btScalar Controller::getFrequency()
{
    return freq;
}

#pragma mark - Methods
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
