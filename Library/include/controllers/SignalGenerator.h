//
//  SignalGenerator.h
//  Stonefish
//
//  Created by Patryk Cieslak on 07/07/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_SignalGenerator__
#define __Stonefish_SignalGenerator__

#include "controllers/PathGenerator2D.h"

namespace sf
{

typedef enum {SIGNAL_ADD, SIGNAL_SUBTRACT, SIGNAL_MULTIPLY} SignalCombineType;

/*! An abstract time domain signal */
class Signal
{
public:
    Signal(SignalCombineType combineOperation) : combineOp(combineOperation) {}
    virtual ~Signal() {}
    
    virtual Scalar ValueAtTime(Scalar t) = 0;
    
    SignalCombineType getCombineOperation() { return combineOp; }
    
private:
    SignalCombineType combineOp;
};

/*! A constant time domain signal */
class ConstSignal : public Signal
{
public:
    ConstSignal(Scalar value, SignalCombineType combineOperation = SIGNAL_ADD);
    
    Scalar ValueAtTime(Scalar t);
    
private:
    Scalar val;
};

/*! A step time domain signal */
class StepSignal : public Signal
{
public:
    StepSignal(Scalar initialValue, Scalar finalValue, Scalar stepTime, Scalar stepDuration, SignalCombineType combineOperation = SIGNAL_ADD);
    
    Scalar ValueAtTime(Scalar t);
    
private:
    Scalar val0;
    Scalar val1;
    Scalar stepT;
    Scalar stepD;
};

/*! A PWL time domain signal */
class PwlSignal : public Signal
{
public:
    PwlSignal(Scalar initialValue, SignalCombineType combineOperation = SIGNAL_ADD);
    
    bool AddValueAtTime(Scalar v, Scalar t);
    Scalar ValueAtTime(Scalar t);
    
private:
    std::vector<Point2D> points;
};

/*! A sinusoidal signal */
class SinSignal : public Signal
{
public:
    SinSignal(Scalar frequency, Scalar amplitude, Scalar initialPhase, SignalCombineType combineOperation = SIGNAL_ADD);
    
    Scalar ValueAtTime(Scalar t);
    
private:
    Scalar freq;
    Scalar amp;
    Scalar phase;
};

/*! A time domain signal generator */
class SignalGenerator
{
public:
    SignalGenerator();
    virtual ~SignalGenerator();
    
    void AddComponent(Signal* c);
    Scalar ValueAtTime(Scalar t);
    
private:
    std::vector<Signal*> components;
};

}

#endif
