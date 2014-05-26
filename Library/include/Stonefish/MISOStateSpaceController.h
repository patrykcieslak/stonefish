//
//  MISOStateSpaceController.h
//  Stonefish
//
//  Created by Patryk Cieslak on 24/05/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_MISOStateSpaceController__
#define __Stonefish_MISOStateSpaceController__

#include "Controller.h"
#include "Mux.h"
#include "DCMotor.h"

class MISOStateSpaceController : public Controller
{
public:
    MISOStateSpaceController(std::string uniqueName, Mux* inputs, DCMotor* output, btScalar maxOutput, btScalar frequency);
    ~MISOStateSpaceController();
    
    void SetGains(const std::vector<btScalar>& g);
    void Reset();
    
    ControllerType getType();
    
private:
    void Tick();
    
    Mux* input;
    DCMotor* output;
    btScalar maxOutput;
    std::vector<btScalar> gains;
};

#endif
