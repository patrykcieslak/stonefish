//
//  FeedbackController.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 18/07/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#include "FeedbackController.h"

FeedbackController::FeedbackController(std::string uniqueName, unsigned int numberOfInputs, btScalar frequency) : Controller(uniqueName, frequency)
{
    referenceGen = NULL;
    referenceGenInput = 0;
    referenceMux = NULL;
	output = 0;
    
    for(unsigned int i = 0; i < numberOfInputs; ++i)
        reference.push_back(btScalar(0.));
}

FeedbackController::~FeedbackController()
{
    if(referenceGen != NULL)
        delete referenceGen;
    
    if(referenceMux != NULL)
        delete referenceMux;
    
    reference.clear();
}

void FeedbackController::setReferenceSignalGenerator(unsigned int inputId, SignalGenerator* sg)
{
    if(sg != NULL && inputId < reference.size())
    {
        if(referenceMux != NULL)
        {
            delete referenceMux;
            referenceMux = NULL;
        }
        
        if(referenceGen != NULL)
            delete referenceGen;
        
        referenceGen = sg;
        referenceGenInput = inputId;
    }
    else if(sg == NULL)
    {
        if(referenceGen != NULL)
            delete referenceGen;
        
        referenceGen = NULL;
    }
}

void FeedbackController::setReferenceSignalMux(SignalMux* sm)
{
    if(sm != NULL && reference.size() > 1 && sm->getNumOfSignals() == reference.size())
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

void FeedbackController::setReferenceValue(unsigned int inputId, btScalar value)
{
    if(inputId < reference.size())
        reference[inputId] = value;
}

void FeedbackController::setReferenceValues(const std::vector<btScalar> &values)
{
    if(values.size() != reference.size())
        return;
    
    for(unsigned int i = 0; i < reference.size(); ++i)
        reference[i] = values[i];
}

std::vector<btScalar> FeedbackController::getReferenceValues()
{
    if(referenceGen != NULL)
    {
        std::vector<btScalar> ref = reference;
        ref[referenceGenInput] = referenceGen->ValueAtTime(runningTime);
        return ref;
    }
    else if(referenceMux != NULL)
        return referenceMux->ValuesAtTime(runningTime);
    else
        return reference;
}

btScalar FeedbackController::getLastOutput()
{
	return output;
}

unsigned int FeedbackController::getNumOfInputs()
{
    return (unsigned int)reference.size();
}

ControllerType FeedbackController::getType()
{
    return CONTROLLER_FEEDBACK;
}
