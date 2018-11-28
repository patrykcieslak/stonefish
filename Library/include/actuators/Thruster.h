//
//  Thruster.h
//  Stonefish
//
//  Created by Patryk Cieslak on 10/10/2017.
//  Copyright (c) 2017 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Thruster__
#define __Stonefish_Thruster__

#include "actuators/LinkActuator.h"

namespace sf
{

class Thruster : public LinkActuator
{
public:
    Thruster(std::string uniqueName, SolidEntity* propeller, Scalar diameter, Scalar thrustCoeff, Scalar torqueCoeff, Scalar maxRPM);
    virtual ~Thruster();

    void Update(Scalar dt);
    std::vector<Renderable> Render();
    
    void setSetpoint(Scalar value);
    Scalar getSetpoint();
    Scalar getThrust();
    Scalar getAngle();
    Scalar getOmega();
    
private:
    //Params
    Scalar D;
    Scalar I;
    Scalar kT;
    Scalar kQ;
    Scalar kp;
    Scalar ki;
    Scalar iLim;
    Scalar omegaLim;
    SolidEntity* prop;
    
    //States
    Scalar theta;
    Scalar omega;
    Scalar thrust;
    Scalar torque;
    Scalar setpoint;
    Scalar iError;
};
    
}

#endif
