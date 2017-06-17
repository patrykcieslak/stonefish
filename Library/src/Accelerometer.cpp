//
//  Accelerometer.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 23/03/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#include "Accelerometer.h"
#include "SimulationApp.h"

Accelerometer::Accelerometer(std::string uniqueName, SolidEntity* attachment, btTransform relFrame, AxisType senseAxis, btScalar rangeMin, btScalar rangeMax, btScalar sensitivity, btScalar zeroVoltage, btScalar noisePSD, ADC* adc, bool measuresInG, btScalar frequency, unsigned int historyLength):SimpleSensor(uniqueName, frequency, historyLength)
{
    solid = attachment;
    relToSolid = relFrame;
    axis = senseAxis;
    G = measuresInG;
    
    std::string axisName[3] = {"X", "Y", "Z"};
    channels.push_back(SensorChannel("Acceleration " + axisName[axis], G ? QUANTITY_UNITLESS : QUANTITY_ACCELERATION));
    
    range[0] = G ? rangeMin : UnitSystem::SetAcceleration(rangeMin);
    range[1] = G ? rangeMax : UnitSystem::SetAcceleration(rangeMax);
    sens = G ? sensitivity : UnitSystem::SetAcceleration(sensitivity);
    
    zeroV = zeroVoltage;
    this->noisePSD = noisePSD;
    this->adc = adc;
}

void Accelerometer::Reset()
{
    //calculate transformation from global to acc frame
    btMatrix3x3 toAccFrame = relToSolid.getBasis().inverse() * solid->getTransform().getBasis().inverse();
    
    //get velocity
    lastV = solid->getLinearVelocityInLocalPoint(relToSolid.getOrigin());
    lastV = toAccFrame * lastV;
    
    SimpleSensor::Reset();
}

void Accelerometer::InternalUpdate(btScalar dt)
{
    //calculate transformation from global to acc frame
    btMatrix3x3 toAccFrame = relToSolid.getBasis().inverse() * solid->getTransform().getBasis().inverse();
    
    //inertial component
    btVector3 actualV = solid->getLinearVelocityInLocalPoint(relToSolid.getOrigin()); //get velocity in sensor location
    actualV = toAccFrame * actualV;
    btVector3 accel = -(actualV - lastV)/dt;
    lastV = actualV;
    
    //gravity component
    btVector3 gravity = SimulationApp::getApp()->getSimulationManager()->getGravity();
    accel += toAccFrame * gravity;
    
    //select axis
    btScalar acc = accel[axis];
    if(G) acc /= gravity.norm();
    
    //add limits/noise/nonlinearity
    acc = acc < range[0] ? range[0] : (acc > range[1] ? range[1] : acc);
    
    //put through ADC
    acc = (adc->MeasureVoltage(acc * sens + zeroV) - zeroV) / sens; //sensitivity V/g V/(m/s^2)
    
    //save sample
    Sample s(1, &acc);
    AddSampleToHistory(s);
}