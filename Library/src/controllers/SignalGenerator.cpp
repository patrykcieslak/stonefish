//
//  SignalGenerator.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 12/07/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#include "controllers/SignalGenerator.h"

using namespace sf;

SignalGenerator::SignalGenerator()
{
}

SignalGenerator::~SignalGenerator()
{
    for(unsigned int i = 0; i < components.size(); ++i)
        delete components[i];
    
    components.clear();
}

void SignalGenerator::AddComponent(Signal* c)
{
    if(c != NULL)
        components.push_back(c);
}

btScalar SignalGenerator::ValueAtTime(btScalar t)
{
    btScalar value = btScalar(0.);
    
    for(unsigned int i = 0; i < components.size(); ++i)
    {
        switch (components[i]->getCombineOperation())
        {
            case SIGNAL_ADD:
                value += components[i]->ValueAtTime(t);
                break;
                
            case SIGNAL_SUBTRACT:
                value -= components[i]->ValueAtTime(t);
                break;
                
            case SIGNAL_MULTIPLY:
                value *= components[i]->ValueAtTime(t);
                break;
        }
    }
    
    return value;
}

ConstSignal::ConstSignal(btScalar value, SignalCombineType combineOperation) : Signal(combineOperation)
{
    val = value;
}

btScalar ConstSignal::ValueAtTime(btScalar t)
{
    return val;
}

StepSignal::StepSignal(btScalar initialValue, btScalar finalValue, btScalar stepTime, btScalar stepDuration, SignalCombineType combineOperation) : Signal(combineOperation)
{
    val0 = initialValue;
    val1 = finalValue;
    stepT = stepTime < btScalar(0.) ? btScalar(0.) : stepTime;
    stepD = stepDuration < btScalar(0.) ? btScalar(0.) : stepDuration;
}

btScalar StepSignal::ValueAtTime(btScalar t)
{
    if(t < stepT)
        return val0;
    else if(t >= stepT+stepD)
        return val1;
    else
        return (t - stepT)/stepD * (val1 - val0) + val0;
}

PwlSignal::PwlSignal(btScalar initialValue, SignalCombineType combineOperation)  : Signal(combineOperation)
{
    points.push_back(Point2D(0, initialValue));
}

bool PwlSignal::AddValueAtTime(btScalar v, btScalar t)
{
    if(t >= points.back().x)
    {
        points.push_back(Point2D(t, v));
        return true;
    }
    else
        return false;
}

btScalar PwlSignal::ValueAtTime(btScalar t)
{
    //Time at end of signal definition
    if(t <= btScalar(0.))
        return points[0].y;
    
    if(t >= points.back().x)
        return points.back().y;
    
    //Time in the middle
    for(unsigned int i = 0; i < points.size() - 1; ++i)
    {
        if(t >= points[i].x && t <= points[i+1].x)
        {
            if(points[i].x == points[i+1].x) //Step with 0 time duration
                return points[i+1].y;
            else
                return (t - points[i].x)/(points[i+1].x - points[i].x) * (points[i+1].y - points[i].y) + points[i].y;
        }
    }
   
    //Should never be reached
    return btScalar(0.);
}

SinSignal::SinSignal(btScalar frequency, btScalar amplitude, btScalar initialPhase, SignalCombineType combineOperation) : Signal(combineOperation)
{
    freq = frequency > btScalar(0.) ? frequency : btScalar(1.); //Defaults to 1Hz
    amp = btFabs(amplitude);
    phase = UnitSystem::SetAngle(initialPhase);
}

btScalar SinSignal::ValueAtTime(btScalar t)
{
    return amp * btSin(t * freq * SIMD_2_PI + phase);
}
