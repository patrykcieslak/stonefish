//
//  Thruster.h
//  Stonefish
//
//  Created by Patryk Cieslak on 10/10/2017.
//  Copyright (c) 2017 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Thruster__
#define __Stonefish_Thruster__

#include "Actuator.h"
#include "SolidEntity.h"
#include "FeatherstoneEntity.h"

class Thruster : public Actuator
{
public:
    Thruster(std::string uniqueName, btScalar diameter, btScalar inertia, btScalar thrustCoeff, btScalar torqueCoeff, btScalar gainP, btScalar gainI, 
             std::string propellerModelPath, btScalar scale, bool smooth, int look);
    virtual ~Thruster();

    void Setpoint(btScalar value);
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
    SolidEntity* attach;
    FeatherstoneEntity* attachFE; 
    unsigned int linkId;
    btTransform pos;

    //States
    btScalar theta;
    btScalar omega;
    btScalar setpoint;

    //Rendering
    int objectId;
    int lookId;
};

#endif
