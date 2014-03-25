//
//  Submersible.h
//  Stonefish
//
//  Created by Patryk Cieslak on 8/20/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish__Submersible__
#define __Stonefish__Submersible__

#include "Machine.h"
#include "CompoundEntity.h"
#include "Thruster.h"
#include "ControlSurface.h"
#include "Sensor.h"

class Submersible : public Machine
{
public:
    Submersible(std::string uniqueName);
    virtual ~Submersible(void);
    
    MachineType getType();
    void Render();
    
    btTransform GetTransform();
    void SetTransform(const btTransform& trans);
    void AddToDynamicsWorld(btDynamicsWorld* world);
    void RemoveFromDynamicsWorld(btDynamicsWorld* world);
    
    void AddFramePart(SolidEntity* solid, const btTransform& location);
    void RemoveFramePart(std::string uniqueName);
    SolidEntity* GetFramePart(unsigned int index);
    unsigned int FramePartsCount();
    
    
    void AddThruster(Thruster* th, const btTransform& location);
    void AddControlSurface(ControlSurface* cs, const btTransform& location);
    void AddSensor(Sensor* s, const btTransform& location);

    void SetThrust(int thrusterId, btScalar value);
    void SetControlAngle(int controlSurfaceId, btScalar value);
    
    void ApplyForces();
    void ApplyGravity();
    void ApplyPropellingForces();
    void ApplyFluidForces();
    
private:
    CompoundEntity* frame;
    std::vector<Sensor*> sensors;
    std::vector<Thruster*> thrusters;
    std::vector<ControlSurface*> controlSurfaces;
};

#endif