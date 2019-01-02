//
//  MISOStateSpaceController.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 24/05/2014.
//  Copyright (c) 2014-2018 Patryk Cieslak. All rights reserved.
//

#include "controllers/MISOStateSpaceController.h"

#include "actuators/DCMotor.h"
#include "sensors/scalar/Mux.h"

namespace sf
{

MISOStateSpaceController::MISOStateSpaceController(std::string uniqueName, Mux* inputs, DCMotor* output, Scalar maxOutput, Scalar frequency) : FeedbackController(uniqueName, inputs->getNumOfComponents(), frequency)
{
    this->input = inputs;
    this->output = output;
    this->maxOutput = maxOutput;
    
    for(unsigned int i = 0; i < input->getNumOfComponents(); i++)
        gains.push_back(Scalar(0.));
}

MISOStateSpaceController::~MISOStateSpaceController()
{
    gains.clear();
}

void MISOStateSpaceController::SetGains(const std::vector<Scalar>& g)
{
    if(g.size() == gains.size())
        gains = std::vector<Scalar>(g);
}

void MISOStateSpaceController::Reset()
{
}

void MISOStateSpaceController::Tick(Scalar dt)
{
    //Update desired values
    std::vector<Scalar> ref = getReferenceValues();
   
    //Get last measurements
    Scalar* measurements = input->getLastSample();
    
    //Calculate control
    Scalar control = 0;
    
    for(unsigned int i = 0; i < gains.size(); i++)
        control += (ref[i] - measurements[i]) * gains[i];
    
    //Limit control
    control = control > maxOutput ? maxOutput : (control < -maxOutput ? -maxOutput : control);
    
    //Apply control
    output->setIntensity(control);
}

}
