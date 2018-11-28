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

Scalar SignalGenerator::ValueAtTime(Scalar t)
{
    Scalar value = Scalar(0.);
    
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

ConstSignal::ConstSignal(Scalar value, SignalCombineType combineOperation) : Signal(combineOperation)
{
    val = value;
}

Scalar ConstSignal::ValueAtTime(Scalar t)
{
    return val;
}

StepSignal::StepSignal(Scalar initialValue, Scalar finalValue, Scalar stepTime, Scalar stepDuration, SignalCombineType combineOperation) : Signal(combineOperation)
{
    val0 = initialValue;
    val1 = finalValue;
    stepT = stepTime < Scalar(0.) ? Scalar(0.) : stepTime;
    stepD = stepDuration < Scalar(0.) ? Scalar(0.) : stepDuration;
}

Scalar StepSignal::ValueAtTime(Scalar t)
{
    if(t < stepT)
        return val0;
    else if(t >= stepT+stepD)
        return val1;
    else
        return (t - stepT)/stepD * (val1 - val0) + val0;
}

PwlSignal::PwlSignal(Scalar initialValue, SignalCombineType combineOperation)  : Signal(combineOperation)
{
    points.push_back(Point2D(0, initialValue));
}

bool PwlSignal::AddValueAtTime(Scalar v, Scalar t)
{
    if(t >= points.back().x)
    {
        points.push_back(Point2D(t, v));
        return true;
    }
    else
        return false;
}

Scalar PwlSignal::ValueAtTime(Scalar t)
{
    //Time at end of signal definition
    if(t <= Scalar(0.))
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
    return Scalar(0.);
}

SinSignal::SinSignal(Scalar frequency, Scalar amplitude, Scalar initialPhase, SignalCombineType combineOperation) : Signal(combineOperation)
{
    freq = frequency > Scalar(0.) ? frequency : Scalar(1.); //Defaults to 1Hz
    amp = btFabs(amplitude);
    phase = initialPhase;
}

Scalar SinSignal::ValueAtTime(Scalar t)
{
    return amp * btSin(t * freq * SIMD_2_PI + phase);
}
