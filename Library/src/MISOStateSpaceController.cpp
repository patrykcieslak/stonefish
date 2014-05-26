//
//  MISOStateSpaceController.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 24/05/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#include "MISOStateSpaceController.h"

MISOStateSpaceController::MISOStateSpaceController(std::string uniqueName, Mux* inputs, DCMotor* output, btScalar maxOutput, btScalar frequency) : Controller(uniqueName, frequency)
{
    this->input = inputs;
    this->output = output;
    this->maxOutput = maxOutput;
    
    for(int i = 0; i < this->input->getNumOfComponents(); i++)
        gains.push_back(btScalar(0.));
}

MISOStateSpaceController::~MISOStateSpaceController()
{
    gains.clear();
}

void MISOStateSpaceController::Reset()
{
}

ControllerType MISOStateSpaceController::getType()
{
    return MISO;
}

void MISOStateSpaceController::Tick()
{
    //get last measurements
    btScalar* sample = input->getLastSample();
    
    //calculate and apply control
    btScalar control = 0;
    for(int i = 0; i < gains.size(); i++)
        control += sample[i] * gains[i];
    control = control > maxOutput ? maxOutput : (control < -maxOutput ? -maxOutput : control);
    output->setVoltage(-control);
}

void MISOStateSpaceController::SetGains(const std::vector<btScalar>& g)
{
    if(g.size() == gains.size())
    {
        for(int i = 0; i < gains.size(); i++)
            gains[i] = g[i];
    }
}

