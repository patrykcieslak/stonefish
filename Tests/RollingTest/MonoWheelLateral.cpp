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
MonoWheelLateral::MonoWheelLateral(std::string uniqueName, FakeIMU* cartImu, FakeRotaryEncoder* leverEnc, FakeRotaryEncoder* wheelEnc, Current* leverCurrent, DCMotor* leverMotor, btScalar maxVoltage, btScalar frequency) : FeedbackController(uniqueName, 5, frequency)
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
}

#pragma mark - Destructor
MonoWheelLateral::~MonoWheelLateral()
{
    for(int i = 0; i <5; i++)
        delete LF[i];
    
    gains.clear();
}

#pragma mark - Accessors
void MonoWheelLateral::setDesiredTilt(btScalar tilt)
{
    setReferenceValue(0, UnitSystem::SetAngle(tilt));
}

#pragma mark - Methods
void MonoWheelLateral::Reset()
{
}

void MonoWheelLateral::Tick(btScalar dt)
{
    //Update desired values
    std::vector<btScalar> ref = getReferenceValues();
    
    //Read sensors
    std::vector<btScalar> measurements;
    measurements.push_back(imu->getLastSample().getValue(0)); //theta
    measurements.push_back(lEnc->getLastSample().getValue(0)); //alpha
    measurements.push_back(imu->getLastSample().getValue(3)); //dtheta
    measurements.push_back(lEnc->getLastSample().getValue(1)); //dalpha
    measurements.push_back(current->getLastSample().getValue(0)); //current
    
    btScalar dGamma = wEnc->getLastSample().getValue(1);
    btScalar dPhi = imu->getLastSample().getValue(5);
    btScalar centrifugalAcc = dPhi * dGamma * tyreRadius; //this is simplified - this formula works for COG of robot
    //printf("Centrifugal: %1.5lf\n", centrifugalAcc);
    
    //Find stable alpha with iterative solution (not possible to derive direct solution)
    btScalar l = 0.15;
    btScalar r1 = 0.15;
    btScalar r2 = 0.12;
    btScalar m1 = 1.5;
    btScalar m2 = 0.5;
    btScalar g = 9.81;
    btScalar equality = 0.0;
    unsigned int iter = 0;
    
    do
    {
        //It works because the angles change in the same direction and monotonically -> have to show it!
        equality = -btSin(ref[0]+ref[1])-(m2 * r2 * centrifugalAcc * btCos(ref[0]+ref[1]) + m2*(-g*l*btSin(ref[0]) - centrifugalAcc*l*btCos(ref[0])) + m1*r1*(centrifugalAcc*btCos(ref[0])-g*btSin(ref[0])))/(m2*r2*g);
        ref[1] += equality * 0.01;
        iter++;
    }
    while(btFabs(equality) > 0.001);
    
    ref[1] = 0.0;
    
    //ref[1] = btAsin(l * btSin(ref[0])/r2 + m1*r1*btSin(ref[0])/(m2*r2))-ref[0]; //without centrifugal acceleration
    //printf("Theta: %1.5lf Alpha: %1.5lf Equality: %1.5f Iterations: %d\n", ref[0], ref[1], equality, iter);
    
    btScalar cgains[5] = {-4.6624e+02, 9.2232e+01,  -1.1031e+02,   1.1482e+01,   2.2862e-02};
    
    for(unsigned int i = 0; i < 5; ++i)
    {
        gains[i] = cgains[i];
        //gains[i] = LF[i]->Value(btFabs(centrifugalAcc), btFabs(dGamma));
        //printf("Gain %d: %1.8lf\n", i, gains[i]);
        //printf("Sensor %d: %1.8lf\n", i, measurements[i]);
    }
    
    //Calculate control
    btScalar control = 0;
    //btScalar Nx[5] = {0.012957, 0.072613, 0.500000, 0.499983, 1.074444};
    //btScalar Nu = 0.251187;
    //btScalar N = Nu + gains[0] * Nx[0] + gains[1] * Nx[1] + gains[2] * Nx[2] + gains[3] * Nx[3] + gains[4] * Nx[4];
    
    for(unsigned int i = 0; i < 5; ++i)
        //control += - measurements[i] * gains[i] + N * ref[i];
        control += -gains[i] * (measurements[i] - ref[i]);
        
    //Limit control
    control = control > maxV ? maxV : (control < - maxV ? - maxV : control);
    
    //Apply control
    motor->setVoltage(control);
}