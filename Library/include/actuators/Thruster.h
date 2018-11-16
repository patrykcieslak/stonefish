//
//  Thruster.h
//  Stonefish
//
//  Created by Patryk Cieslak on 10/10/2017.
//  Copyright (c) 2017 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Thruster__
#define __Stonefish_Thruster__

#include "entities/SolidEntity.h"
#include "entities/FeatherstoneEntity.h"
#include "actuators/Actuator.h"

class Thruster : public Actuator
{
public:
    Thruster(std::string uniqueName, SolidEntity* propeller, btScalar diameter, btScalar thrustCoeff, btScalar torqueCoeff, btScalar maxRPM);
    virtual ~Thruster();

    void setSetpoint(btScalar value);
    btScalar getSetpoint();
    btScalar getThrust();
    btScalar getAngle();
    btScalar getOmega();
    
    void Update(btScalar dt);
    std::vector<Renderable> Render();
    ActuatorType getType();
    
    void AttachToSolid(SolidEntity* solid, const btTransform& position);
    void AttachToSolid(FeatherstoneEntity* fe, unsigned int link, const btTransform& position);
    
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
    SolidEntity* attach;
    FeatherstoneEntity* attachFE; 
    unsigned int linkId;
    btTransform pos;

    //States
    btScalar theta;
    btScalar omega;
    btScalar thrust;
    btScalar torque;
    btScalar setpoint;
    btScalar iError;
};

#endif
