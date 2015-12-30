//
//  AcroverSpeedController.h
//  Stonefish
//
//  Created by Patryk Cieslak on 15/09/2015.
//  Copyright (c) 2015 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish__AcroverSpeedController__
#define __Stonefish__AcroverSpeedController__

#include "FeedbackController.h"
#include "AcroverDriveController.h"
#include "FakeIMU.h"
#include "FakeRotaryEncoder.h"
#include <Eigen/Core>

class AcroverSpeedController : public FeedbackController
{
public:
    AcroverSpeedController(std::string uniqueName, FakeIMU* cartImu, FakeRotaryEncoder* wheelEnc, AcroverDriveController* wheelDrive, btScalar frequency = btScalar(-1.));
    ~AcroverSpeedController();
    
    void Reset();
    
    void setDesiredSpeed(btScalar speed);
    
private:
    void Tick(btScalar dt);
    
    FakeIMU* imu;
    FakeRotaryEncoder* enc;
    AcroverDriveController* drive;
    btScalar maxSpeed;
    
    Eigen::VectorXd gains;
    btScalar errorIntegral;
};

#endif /* defined(__Stonefish__AcroverSpeedController__) */
