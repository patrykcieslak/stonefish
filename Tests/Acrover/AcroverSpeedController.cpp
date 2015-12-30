//
//  AcroverSpeedController.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 15/09/2015.
//  Copyright (c) 2015 Patryk Cieslak. All rights reserved.
//

#include "AcroverSpeedController.h"

#pragma mark Constructors
AcroverSpeedController::AcroverSpeedController(std::string uniqueName, FakeIMU* cartImu, FakeRotaryEncoder* wheelEnc, AcroverDriveController* wheelDrive, btScalar frequency) : FeedbackController(uniqueName, 4, frequency)
{
    imu = cartImu;
    enc = wheelEnc;
    drive = wheelDrive;
    maxSpeed = 20; //rad/s
    
    gains = Eigen::VectorXd(4);
    gains <<  -20.6219, -3.6803, 5.1075, -0.3162;
    
    Reset();
}

#pragma mark Destructor
AcroverSpeedController::~AcroverSpeedController()
{
}


#pragma mark - Accessors
void AcroverSpeedController::setDesiredSpeed(btScalar speed)
{
    speed = btFabs(speed) > maxSpeed ? (speed > btScalar(0.) ? maxSpeed : -maxSpeed) : speed;
    setReferenceValue(2, UnitSystem::SetAngularVelocity(speed));
}

#pragma mark - Methods
void AcroverSpeedController::Reset()
{
    errorIntegral = btScalar(0.);
}

void AcroverSpeedController::Tick(btScalar dt)
{
    //Update desired values
    std::vector<btScalar> ref = getReferenceValues();
    
    //Read sensors
    std::vector<btScalar> measurements;
    measurements.push_back(imu->getLastSample().getValue(1)); //psi
    measurements.push_back(imu->getLastSample().getValue(4)); //dpsi
    measurements.push_back(-enc->getLastSample().getValue(1)); //dgamma
    measurements.push_back(errorIntegral); //integral
    
    //Calculate error integral
    errorIntegral += (ref[2]-measurements[2])*dt;
    errorIntegral = errorIntegral > 1.0 ? 1.0 : (errorIntegral < -1.0 ? -1.0 : errorIntegral);
    
    //Calculate control
    btScalar control = 0;
    
    for(unsigned int i = 0; i < 4; ++i)
        control += (ref[i]-measurements[i]) * gains(i);
    
    //printf("Ref: %1.5f Psi: %1.5f dPsi: %1.5f dGamma: %1.5f Integral: %1.5f Control: %1.5f\n", ref[2], measurements[0], measurements[1], measurements[2], measurements[3], control);
    //printf("1: %1.5f 2: %1.5f 3: %1.5f\n", imu->getLastSample().getValue(3), imu->getLastSample().getValue(4), imu->getLastSample().getValue(5));
    
    //Apply control
    drive->setTorque(-control);
    
}