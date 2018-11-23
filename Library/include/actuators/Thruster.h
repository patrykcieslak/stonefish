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
    Thruster(std::string uniqueName, SolidEntity* propeller, btScalar diameter, btScalar thrustCoeff, btScalar torqueCoeff, btScalar maxRPM);
    virtual ~Thruster();

    void Update(btScalar dt);
    std::vector<Renderable> Render();
    
    void setSetpoint(btScalar value);
    btScalar getSetpoint();
    btScalar getThrust();
    btScalar getAngle();
    btScalar getOmega();
    
private:
    //Params
    btScalar D;
    btScalar I;
    btScalar kT;
    btScalar kQ;
    btScalar kp;
    btScalar ki;
    btScalar iLim;
    btScalar omegaLim;
    SolidEntity* prop;
    
    //States
    btScalar theta;
    btScalar omega;
    btScalar thrust;
    btScalar torque;
    btScalar setpoint;
    btScalar iError;
};
    
}

#endif
