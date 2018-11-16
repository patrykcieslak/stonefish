//
//  MISOStateSpaceController.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 24/05/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#include "controllers/MISOStateSpaceController.h"

MISOStateSpaceController::MISOStateSpaceController(std::string uniqueName, Mux* inputs, DCMotor* output, btScalar maxOutput, btScalar frequency) : FeedbackController(uniqueName, inputs->getNumOfComponents(), frequency)
{
    this->input = inputs;
    this->output = output;
    this->maxOutput = maxOutput;
    
    for(unsigned int i = 0; i < input->getNumOfComponents(); i++)
        gains.push_back(btScalar(0.));
}

MISOStateSpaceController::~MISOStateSpaceController()
{
    gains.clear();
}

void MISOStateSpaceController::SetGains(const std::vector<btScalar>& g)
{
    if(g.size() == gains.size())
        gains = std::vector<btScalar>(g);
}

void MISOStateSpaceController::Reset()
{
}

void MISOStateSpaceController::Tick(btScalar dt)
{
    //Update desired values
    std::vector<btScalar> ref = getReferenceValues();
   
    //Get last measurements
    btScalar* measurements = input->getLastSample();
    
    //Calculate control
    btScalar control = 0;
    
    for(unsigned int i = 0; i < gains.size(); i++)
        control += (ref[i] - measurements[i]) * gains[i];
    
    //Limit control
    control = control > maxOutput ? maxOutput : (control < -maxOutput ? -maxOutput : control);
    
    //Apply control
    output->setVoltage(control);
}
