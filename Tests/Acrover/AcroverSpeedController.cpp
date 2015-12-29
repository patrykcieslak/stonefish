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
 
    gains = Eigen::VectorXd(4);
    gains << -109.5851,  -11.8287 ,  13.5276  ,  3.1623;
    
    Reset();
}

#pragma mark Destructor
AcroverSpeedController::~AcroverSpeedController()
{
}


#pragma mark - Accessors
void AcroverSpeedController::setDesiredSpeed(btScalar speed)
{
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
    errorIntegral += (measurements[2]-ref[2])*dt;
   // errorIntegral = errorIntegral > 1.0 ? 1.0 : (errorIntegral < -1.0 ? -1.0 : errorIntegral);
    
    //Calculate control
    btScalar control = 0;
    
    for(unsigned int i = 0; i < 4; ++i)
        control += (measurements[i]-ref[i]) * gains(i);
    
    //printf("Int: %1.5f dGamma: %1.5f Psi: %1.5f refdGamma: %1.5f refPsi: %1.5f Control: %1.5f\n", errorIntegral, measurements[2], measurements[0], ref[2], ref[0], control);
    
    //printf("Psi_ref: %1.5f dGamma_ref: %1.5f ", ref[0], ref[2]);
    
    //Apply control
    drive->setTorque(control);
    
}