//
//  Accelerometer.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 23/03/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#include "Accelerometer.h"

Accelerometer::Accelerometer(std::string uniqueName, SolidEntity* attachment, btTransform relFrame, AxisType senseAxis, btScalar rangeMin, btScalar rangeMax, btScalar sensitivity, btScalar zeroVoltage, btScalar noisePSD, ADC* adc, bool measuresInG, unsigned int historyLength):Sensor(uniqueName, historyLength)
{
    solid = attachment;
    relToSolid = relFrame;
    axis = senseAxis;
    range[0] = rangeMin;
    range[1] = rangeMax;
    sens = sensitivity;
    zeroV = zeroVoltage;
    this->noisePSD = noisePSD;
    this->adc = adc;
    G = measuresInG;
    
    Reset();
}

void Accelerometer::Reset()
{
    lastV = btVector3(0.,0.,0.);
}

void Accelerometer::Update(btScalar dt)
{
    //calculate transformation from global to acc frame
    btMatrix3x3 toAccFrame = relToSolid.getBasis().inverse() * solid->getRigidBody()->getCenterOfMassTransform().getBasis().inverse();
    
    //inertial component
    btVector3 actualV = solid->getRigidBody()->getVelocityInLocalPoint(relToSolid.getOrigin()); //get velocity in sensor location
    actualV = toAccFrame * actualV;
    btVector3 accel = -(actualV - lastV)/dt;
    lastV = actualV;
    
    //gravity component
    btVector3 grav = solid->getRigidBody()->getGravity();
    grav = toAccFrame * grav;
    accel += grav;
    
    //select axis and convert to external unit system or G's
    btScalar acc = accel[axis];
    
    if(G)
        acc /= solid->getRigidBody()->getGravity().norm();
    else
        acc = UnitSystem::GetLength(acc);
    
    //add limits/noise/nonlinearity
    acc = acc < range[0] ? range[0] : (acc > range[1] ? range[1] : acc);
    
    //put through ADC
    acc = (adc->MeasureVoltage(acc * sens + zeroV) - zeroV) / sens; //sensitivity V/g V/(m/s^2)
    
    //save sample
    Sample s(1, &acc);
    AddSampleToHistory(s);
}

unsigned short Accelerometer::getNumOfDimensions()
{
    return 1;
}