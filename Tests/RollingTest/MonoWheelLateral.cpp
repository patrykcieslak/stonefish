//
//  MonoWheelLateral.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 21/06/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#include "MonoWheelLateral.h"

#pragma mark Constructors
MonoWheelLateral::MonoWheelLateral(std::string uniqueName, FakeIMU* cartImu, FakeRotaryEncoder* leverEnc, Current* leverCurrent, DCMotor* leverMotor, btScalar frequency) : Controller(uniqueName, frequency)
{
    imu = cartImu;
    enc = leverEnc;
    current = leverCurrent;
    motor = leverMotor;
    
    //Polynomial gain coefficients
    btMatrixXu LF1Coeff(4,4);
    LF1Coeff.setElem(0, 0, 1);
    LF1Coeff.setElem(0, 0, 1);
    LF1Coeff.setElem(0, 0, 1);
    LF1Coeff.setElem(0, 0, 1);
    LF1Coeff.setElem(0, 0, 1);
    LF1Coeff.setElem(0, 0, 1);
    LF1Coeff.setElem(0, 0, 1);
    LF1Coeff.setElem(0, 0, 1);
    LF1Coeff.setElem(0, 0, 1);
    LF1Coeff.setElem(0, 0, 1);
    LF1Coeff.setElem(0, 0, 1);
    LF1Coeff.setElem(0, 0, 1);
    LF1Coeff.setElem(0, 0, 1);
    LF1Coeff.setElem(0, 0, 1);
    LF1Coeff.setElem(0, 0, 1);
    LF1Coeff.setElem(0, 0, 1);
    LF[0] = new PolySurface(LF1Coeff);
    
    btMatrixXu LF2Coeff(3,3);
    LF2Coeff.setElem(0, 0, 1);
    LF2Coeff.setElem(0, 0, 1);
    LF2Coeff.setElem(0, 0, 1);
    LF2Coeff.setElem(0, 0, 1);
    LF2Coeff.setElem(0, 0, 1);
    LF2Coeff.setElem(0, 0, 1);
    LF2Coeff.setElem(0, 0, 1);
    LF2Coeff.setElem(0, 0, 1);
    LF2Coeff.setElem(0, 0, 1);
    LF[1] = new PolySurface(LF2Coeff);
    
    
}

#pragma mark - Destructor
MonoWheelLateral::~MonoWheelLateral()
{
    for(int i = 0; i <5; i++)
        delete LF[i];
}

#pragma mark - Controller
void MonoWheelLateral::Reset()
{
    
}

ControllerType MonoWheelLateral::getType()
{
    return CONTROLLER_CUSTOM;
}

void MonoWheelLateral::Tick(btScalar dt)
{
    
    
    
    
    
    
    
    
    
    
    
}