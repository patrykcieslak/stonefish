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
    
    for(int i = 0; i < input->getNumOfComponents(); i++)
    {
        gains.push_back(btScalar(0.));
        desiredValues.push_back(btScalar(0.));
    }
}

MISOStateSpaceController::~MISOStateSpaceController()
{
    gains.clear();
    desiredValues.clear();
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
    //Get last measurements
    btScalar* measurements = input->getLastSample();
    
    //Calculate control
    btScalar control = 0;
    for(int i = 0; i < gains.size(); i++)
        control += (desiredValues[i] - measurements[i]) * gains[i];
    
    //Limit control
    control = control > maxOutput ? maxOutput : (control < -maxOutput ? -maxOutput : control);
    
    //Apply control
    output->setVoltage(control);
}

void MISOStateSpaceController::SetGains(const std::vector<btScalar>& g)
{
    if(g.size() == gains.size())
        gains = std::vector<btScalar>(g);
}

void MISOStateSpaceController::SetDesiredValues(const std::vector<btScalar>& d)
{
    if(d.size() == desiredValues.size())
        desiredValues = std::vector<btScalar>(d);
}

