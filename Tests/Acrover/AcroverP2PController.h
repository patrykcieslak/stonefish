//
//  AcroverP2PController.hpp
//  Stonefish
//
//  Created by Patryk Cieslak on 29/12/2015.
//  Copyright © 2015 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish__AcroverP2PController__
#define __Stonefish__AcroverP2PController__

#include "FeedbackController.h"
#include "Trajectory.h"
#include "RotaryEncoder.h"
#include "AcroverSpeedController.h"
#include "AcroverTiltController.h"

class AcroverP2PController : public FeedbackController
{
public:
    AcroverP2PController(std::string uniqueName, Trajectory* robotPosition, RotaryEncoder* wheelEncoder, AcroverSpeedController* speedController, AcroverTiltController* tiltController, btScalar frequency = btScalar(-1.));
    ~AcroverP2PController();
    
    void setGains(btScalar velocity, btScalar heading);
    void setDesiredPosition(btScalar x, btScalar y);
    void Reset();
    void Tick(btScalar dt);
    
private:
    Trajectory* robotPos;
    RotaryEncoder* wheelEnc;
    AcroverSpeedController* speedCtrl;
    AcroverTiltController* tiltCtrl;

    btScalar Kh, Kv;
};

#endif
