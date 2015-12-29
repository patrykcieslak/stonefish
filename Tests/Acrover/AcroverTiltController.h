//
//  AcroverTiltController.h
//  Stonefish
//
//  Created by Patryk Cieslak on 14/09/2015.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish__AcroverTiltController__
#define __Stonefish__AcroverTiltController__

#include "FeedbackController.h"
#include "AcroverDriveController.h"
#include "FakeIMU.h"
#include "FakeRotaryEncoder.h"
#include <Eigen/Core>

class AcroverTiltController : public FeedbackController
{
public:
    AcroverTiltController(std::string uniqueName, FakeIMU* cartImu, FakeRotaryEncoder* pendulumEnc, FakeRotaryEncoder* wheelEnc, AcroverDriveController* pendulumDrive, btScalar frequency = btScalar(-1.));
    ~AcroverTiltController();
    
    void Reset();
    
    void setDesiredTilt(btScalar tilt);
    
private:
    void Tick(btScalar dt);
    
    FakeIMU* imu;
    FakeRotaryEncoder* lEnc;
    FakeRotaryEncoder* wEnc;
    AcroverDriveController* drive;
    btScalar tyreRadius;
    btScalar maxTilt;
    
    Eigen::VectorXd L1; //1
    Eigen::VectorXd L2; //rho
    Eigen::VectorXd L3; //rho^2
    
    Eigen::MatrixXd X1; //1
    Eigen::MatrixXd X2; //rho
    Eigen::MatrixXd X3; //rho^2
};

#endif
