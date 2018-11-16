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

typedef enum {SIGNAL_ADD, SIGNAL_SUBTRACT, SIGNAL_MULTIPLY} SignalCombineType;

/*! An abstract time domain signal */
class Signal
{
public:
    Signal(SignalCombineType combineOperation) : combineOp(combineOperation) {}
    virtual ~Signal() {}
    
    virtual btScalar ValueAtTime(btScalar t) = 0;
    
    SignalCombineType getCombineOperation() { return combineOp; }
    
private:
    SignalCombineType combineOp;
};

/*! A constant time domain signal */
class ConstSignal : public Signal
{
public:
    ConstSignal(btScalar value, SignalCombineType combineOperation = SIGNAL_ADD);
    
    btScalar ValueAtTime(btScalar t);
    
private:
    btScalar val;
};

/*! A step time domain signal */
class StepSignal : public Signal
{
public:
    StepSignal(btScalar initialValue, btScalar finalValue, btScalar stepTime, btScalar stepDuration, SignalCombineType combineOperation = SIGNAL_ADD);
    
    btScalar ValueAtTime(btScalar t);
    
private:
    btScalar val0;
    btScalar val1;
    btScalar stepT;
    btScalar stepD;
};

/*! A PWL time domain signal */
class PwlSignal : public Signal
{
public:
    PwlSignal(btScalar initialValue, SignalCombineType combineOperation = SIGNAL_ADD);
    
    bool AddValueAtTime(btScalar v, btScalar t);
    btScalar ValueAtTime(btScalar t);
    
private:
    std::vector<Point2D> points;
};

/*! A sinusoidal signal */
class SinSignal : public Signal
{
public:
    SinSignal(btScalar frequency, btScalar amplitude, btScalar initialPhase, SignalCombineType combineOperation = SIGNAL_ADD);
    
    btScalar ValueAtTime(btScalar t);
    
private:
    btScalar freq;
    btScalar amp;
    btScalar phase;
};

/*! A time domain signal generator */
class SignalGenerator
{
public:
    SignalGenerator();
    virtual ~SignalGenerator();
    
    void AddComponent(Signal* c);
    btScalar ValueAtTime(btScalar t);
    
private:
    std::vector<Signal*> components;
};


#endif
