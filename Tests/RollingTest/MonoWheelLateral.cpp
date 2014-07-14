//
//  MonoWheelLateral.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 21/06/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#include "MonoWheelLateral.h"
#include "ScientificFileUtil.h"
#include "SystemUtil.h"

#pragma mark Constructors
MonoWheelLateral::MonoWheelLateral(std::string uniqueName, FakeIMU* cartImu, FakeRotaryEncoder* leverEnc, FakeRotaryEncoder* wheelEnc, Current* leverCurrent, DCMotor* leverMotor, btScalar maxVoltage, btScalar frequency) : Controller(uniqueName, frequency)
{
    imu = cartImu;
    lEnc = leverEnc;
    wEnc = wheelEnc;
    current = leverCurrent;
    motor = leverMotor;
    tyreRadius = 0.205;
    maxV = maxVoltage;
    
    //Polynomial gain coefficients
    //LF1 - theta
    //LF2 - alpha
    //LF3 - dtheta
    //LF4 - dalpha
    //LF5 - current
    char path[1024];
    GetCWD(path, 1024);
    GetDataPath(path, 1024-32);
    strcat(path, "gainfit.oct");
    
    ScientificData* data = LoadOctaveData(path);
    LF[0] = new PolySurface(data->getMatrix("G1"));
    LF[1] = new PolySurface(data->getMatrix("G2"));
    LF[2] = new PolySurface(data->getMatrix("G3"));
    LF[3] = new PolySurface(data->getMatrix("G4"));
    LF[4] = new PolySurface(data->getMatrix("G5"));
    
    //Initialize vectors
    gains.push_back(btScalar(0.));
    gains.push_back(btScalar(0.));
    gains.push_back(btScalar(0.));
    gains.push_back(btScalar(0.));
    gains.push_back(btScalar(0.));
    
    desiredValues.push_back(btScalar(0.));
    desiredValues.push_back(btScalar(0.));
    desiredValues.push_back(btScalar(0.));
    desiredValues.push_back(btScalar(0.));
    desiredValues.push_back(btScalar(0.));
}

#pragma mark - Destructor
MonoWheelLateral::~MonoWheelLateral()
{
    for(int i = 0; i <5; i++)
        delete LF[i];
    
    gains.clear();
    desiredValues.clear();
}

#pragma mark - Accessors
unsigned int MonoWheelLateral::getNumOfInputs()
{
    return 5;
}

ControllerType MonoWheelLateral::getType()
{
    return CONTROLLER_CUSTOM;
}

void MonoWheelLateral::setDesiredTilt(btScalar tilt)
{
    desiredValues[0] = UnitSystem::SetAngle(tilt);
}

#pragma mark - Methods
void MonoWheelLateral::Reset()
{
}

void MonoWheelLateral::Tick(btScalar dt)
{
    //Update desired values
    btScalar* ref = NULL;
    
    if(referenceGen != NULL)
    {
        ref = new btScalar[desiredValues.size()];
        ref[referenceInput] = referenceGen->ValueAtTime(runningTime); //set generated reference
        for(unsigned int i = 0; i < desiredValues.size(); ++i) //fill rest of inputs with desired values vector
            if(i != referenceInput)
                ref[i] = desiredValues[i];
    }
    else if(referenceMux != NULL)
    {
        ref = referenceMux->ValuesAtTime(runningTime);
    }

    //Read sensors
    std::vector<btScalar> measurements;
    measurements.push_back(imu->getLastSample().getValue(0)); //theta
    measurements.push_back(lEnc->getLastSample().getValue(0)); //alpha
    measurements.push_back(imu->getLastSample().getValue(3)); //dtheta
    measurements.push_back(lEnc->getLastSample().getValue(1)); //dalpha
    measurements.push_back(current->getLastSample().getValue(0)); //current
    
    btScalar dGamma = wEnc->getLastSample().getValue(1);
    btScalar dPhi = imu->getLastSample().getValue(5);
    
    //Calculate gains
    btScalar centrifugalAcc = dPhi * dGamma * tyreRadius;
    
    //printf("Centrifugal: %1.5lf\n", centrifugalAcc);
    
    for(unsigned int i = 0; i < 5; i++)
    {
        gains[i] = LF[i]->Value(btFabs(centrifugalAcc), btFabs(dGamma));
        //printf("Gain %d: %1.8lf\n", i, gains[i]);
        //printf("Sensor %d: %1.8lf\n", i, measurements[i]);
    }
    
    //Calculate control
    btScalar control = 0;
    
    if(ref != NULL)
    {
        for(int i = 0; i < gains.size(); i++)
            control += (ref[i] - measurements[i]) * gains[i];
        
        delete [] ref;
    }
    else //no signal generator hook up
    {
        for(int i = 0; i < gains.size(); i++)
            control += (desiredValues[i] - measurements[i]) * gains[i];
    }
    
    //for(int i = 0; i < 5; i++)
    //    control += (desiredValues[i] - measurements[i]) * gains[i];
    
    //Limit control
    control = control > maxV ? maxV : (control < - maxV ? - maxV : control);
    
    //Apply control
    motor->setVoltage(control);
}