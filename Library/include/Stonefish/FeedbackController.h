//
//  FeedbackController.h
//  Stonefish
//
//  Created by Patryk Cieslak on 18/07/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#ifndef Stonefish_FeedbackController_h
#define Stonefish_FeedbackController_h

#include "Controller.h"
#include "SignalMux.h"

/*! Abstract class representing a feedback controller */
class FeedbackController : public Controller
{
public:
    FeedbackController(std::string uniqueName, unsigned int numberOfInputs, btScalar frequency = btScalar(-1.));
    virtual ~FeedbackController();
    
    virtual void Reset() = 0;
    
    void setReferenceSignalGenerator(unsigned int inputId, SignalGenerator* sg);
    void setReferenceSignalMux(SignalMux* sm);
    void setReferenceValue(unsigned int inputId, btScalar value);
    void setReferenceValues(const std::vector<btScalar>& values);
    std::vector<btScalar> getReferenceValues();
    unsigned int getNumOfInputs();
    ControllerType getType();
    
protected:
    virtual void Tick(btScalar dt) = 0;
    
private:
    std::vector<btScalar> reference;
    SignalGenerator* referenceGen;
    unsigned int referenceGenInput;
    SignalMux* referenceMux;
};



#endif
