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
    eleapsedTime = btScalar(0.);
    runningTime = btScalar(0.);
    running = false;
    referenceGen = NULL;
    referenceInput = 0;
    referenceMux = NULL;
}

#pragma mark - Destructor
Controller::~Controller()
{
    nameManager.RemoveName(name);
    
    if(referenceGen != NULL)
        delete referenceGen;
    
    if(referenceMux != NULL)
        delete referenceMux;
}

#pragma mark - Accessors
void Controller::setReferenceSignalGenerator(unsigned int inputId, SignalGenerator* sg)
{
    if(sg != NULL && inputId < getNumOfInputs())
    {
        if(referenceMux != NULL)
        {
            delete referenceMux;
            referenceMux = NULL;
        }
        
        if(referenceGen != NULL)
            delete referenceGen;
        
        referenceGen = sg;
        referenceInput = inputId;
    }
    else if(sg == NULL)
    {
        if(referenceGen != NULL)
            delete referenceGen;
        
        referenceGen = NULL;
    }
}

void Controller::setReferenceSignalMux(SignalMux* sm)
{
    if(sm != NULL && getNumOfInputs() > 1 && sm->getNumOfSignals() == getNumOfInputs())
    {
        if(referenceGen != NULL)
        {
            delete referenceGen;
            referenceGen = NULL;
        }
        
        if(referenceMux != NULL)
            delete referenceMux;
        
        referenceMux = sm;
    }
    else if(sm == NULL)
    {
        if(referenceMux != NULL)
            delete referenceMux;
        
        referenceMux = NULL;
    }
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
