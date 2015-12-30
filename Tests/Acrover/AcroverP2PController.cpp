//
//  AcroverP2PController.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 29/12/2015.
//  Copyright © 2015 Patryk Cieslak. All rights reserved.
//

#include "AcroverP2PController.h"

AcroverP2PController::AcroverP2PController(std::string uniqueName, Trajectory* robotPosition, RotaryEncoder* wheelEncoder, AcroverSpeedController* speedController, AcroverTiltController* tiltController, btScalar frequency) : FeedbackController(uniqueName, 2, frequency)
{
    robotPos = robotPosition;
    wheelEnc = wheelEncoder;
    speedCtrl = speedController;
    tiltCtrl = tiltController;
    
    Kv = btScalar(0.);
    Kh = btScalar(0.);
}

AcroverP2PController::~AcroverP2PController()
{
}

void AcroverP2PController::setGains(btScalar velocity, btScalar heading)
{
    Kv = velocity;
    Kh = heading;
}

void AcroverP2PController::setDesiredPosition(btScalar x, btScalar y)
{
    setReferenceValue(0, UnitSystem::SetLength(x));
    setReferenceValue(1, UnitSystem::SetLength(y));
}

void AcroverP2PController::Reset()
{
}

void AcroverP2PController::Tick(btScalar dt)
{
    std::vector<btScalar> ref = getReferenceValues();
    std::vector<btScalar> measuredPos = robotPos->getLastSample().getData();
    btScalar dgamma = -wheelEnc->getLastSample().getValue(1);
    
    //Calculate position error
    btScalar errorX = ref[0]-measuredPos[0];
    btScalar errorY = ref[1]-measuredPos[1];
    btScalar positionError = btSqrt(errorX * errorX + errorY * errorY);
    
    if(positionError < btScalar(0.06))
    {
        speedCtrl->setDesiredSpeed(0.);
        tiltCtrl->setDesiredTilt(0.);
        Stop();
    }
    else
    {
        //Calculate direction
        btScalar dir = btAtan2(errorY, errorX);
        btScalar headingError = dir - measuredPos[5];
        headingError = headingError >= M_PI ? -(2*M_PI-headingError) : (headingError < -M_PI ? 2*M_PI + headingError : headingError);
    
        //Calculate control
        btScalar speedControl = Kv * positionError;
        btScalar tiltControl = -btAtan(Kh*headingError);//*btFabs(dgamma+0.5));
    
        speedCtrl->setDesiredSpeed(speedControl);
        tiltCtrl->setDesiredTilt(tiltControl);
    }
}