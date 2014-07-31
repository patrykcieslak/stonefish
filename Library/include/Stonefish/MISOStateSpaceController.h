//
//  MISOStateSpaceController.h
//  Stonefish
//
//  Created by Patryk Cieslak on 24/05/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_MISOStateSpaceController__
#define __Stonefish_MISOStateSpaceController__

#include "FeedbackController.h"
#include "Mux.h"
#include "DCMotor.h"

/*! MISO controller for controlling SIMO systems */
class MISOStateSpaceController : public FeedbackController
{
public:
    MISOStateSpaceController(std::string uniqueName, Mux* inputs, DCMotor* output, btScalar maxOutput, btScalar frequency = btScalar(-1.));
    ~MISOStateSpaceController();
    
    void Reset();
    
    void SetGains(const std::vector<btScalar>& g);
    
private:
    void Tick(btScalar dt);
    
    Mux* input;
    DCMotor* output;
    btScalar maxOutput;
    std::vector<btScalar> gains;
};

#endif
