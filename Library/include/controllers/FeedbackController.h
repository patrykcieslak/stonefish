//
//  FeedbackController.h
//  Stonefish
//
//  Created by Patryk Cieslak on 18/07/2014.
//  Copyright (c) 2014-2018 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_FeedbackController__
#define __Stonefish_FeedbackController__

#include "controllers/Controller.h"

namespace sf
{
    class SignalMux;
    class SignalGenerator;
    
    //! Abstract class representing a feedback controller.
    class FeedbackController : public Controller
    {
    public:
        FeedbackController(std::string uniqueName, unsigned int numberOfInputs, Scalar frequency);
        virtual ~FeedbackController();
        
        virtual void Reset() = 0;
        
        void setReferenceSignalGenerator(unsigned int inputId, SignalGenerator* sg);
        void setReferenceSignalMux(SignalMux* sm);
        void setReferenceValue(unsigned int inputId, Scalar value);
        void setReferenceValues(const std::vector<Scalar>& values);
        std::vector<Scalar> getReferenceValues();
        Scalar getLastOutput();
        unsigned int getNumOfInputs();
        ControllerType getType();
        
    protected:
        virtual void Tick(Scalar dt) = 0;
        Scalar output;
        
    private:
        std::vector<Scalar> reference;
        SignalGenerator* referenceGen;
        unsigned int referenceGenInput;
        SignalMux* referenceMux;
    };
}

#endif
