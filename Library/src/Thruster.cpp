//
//  Thruster.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 10/10/2017.
//  Copyright (c) 2017 Patryk Cieslak. All rights reserved.
//

#include "Thruster.h"
#include "OpenGLContent.h"
#include "Ocean.h"
#include "MathsUtil.hpp"

Thruster::Thruster(std::string uniqueName, btScalar diameter, btScalar inertia, btScalar thrustCoeff, btScalar torqueCoeff, btScalar gainP, btScalar gainI, 
                   std::string propellerModelPath, btScalar scale, bool smooth, int look) : Actuator(uniqueName)
{
    D = UnitSystem::SetLength(diameter);
    I = inertia; //Needs proper unit system calculation!
    kT = thrustCoeff;
    kQ = torqueCoeff;
    kp = gainP;
    ki = gainI;
    
    theta = btScalar(0);
    omega = btScalar(0);
    setpoint = btScalar(0);
    
    objectId = OpenGLContent::getInstance()->BuildObject(OpenGLContent::LoadMesh(propellerModelPath, scale, smooth));	
    lookId = look;
    
    attach = NULL;
    attachFE = NULL;
    linkId = 0;
    pos.setIdentity();
}

Thruster::~Thruster()
{
}

ActuatorType Thruster::getType()
{
    return ACTUATOR_THRUSTER;
}

void Thruster::AttachToSolid(SolidEntity* solid, const btTransform& position)
{
    attach = solid;
    pos = position;
}

void Thruster::AttachToSolid(FeatherstoneEntity* fe, unsigned int link, const btTransform& position)
{
    attachFE = fe;
    linkId = link;
    pos = position;
}

std::vector<Renderable> Thruster::Render()
{
    std::vector<Renderable> items(0);
    btTransform thrustTrans = btTransform::getIdentity();
    
    if(attach != NULL)
    {
        thrustTrans = attach->getTransform() * attach->getLocalTransform().inverse() * pos;
    }
    else if(attachFE != NULL)
    {
        FeatherstoneLink link = attachFE->getLink(linkId);
        thrustTrans = link.solid->getTransform() * link.solid->getLocalTransform().inverse() * pos;
    }
    else
    {
        return items;
    }
    
    //Add renderable
    Renderable item;
    item.objectId = objectId;
    item.lookId = lookId;
	item.dispCoordSys = false;
	item.model = glMatrixFromBtTransform(thrustTrans);
    item.csModel = item.model;
	items.push_back(item);
    return items;
}

void Thruster::Setpoint(btScalar value)
{
    setpoint = value < btScalar(-1) ? btScalar(-1) : (value > btScalar(1) ? btScalar(1) : value);
}

void Thruster::Update(btScalar dt)
{
    omega = setpoint;
    
    btScalar thrust = omega*1000.0;
    btScalar torque = omega*0.0;
    
    btVector3 thrustV(thrust, 0, 0);
    btVector3 torqueV(torque, 0, 0);
    
    if(attach != NULL)
    {
        btTransform solidTrans = attach->getTransform() * attach->getLocalTransform().inverse();
        btTransform thrustTrans = solidTrans * pos;
        
        attach->ApplyCentralForce(thrustTrans.getBasis() * thrustV);
        attach->ApplyTorque((thrustTrans.getOrigin() - solidTrans.getOrigin()).cross(thrustTrans.getBasis() * thrustV));
        attach->ApplyTorque(thrustTrans.getBasis() * torqueV);
    }
    else if(attachFE != NULL)
    {
        FeatherstoneLink link = attachFE->getLink(linkId);
        btTransform linkTrans = link.solid->getTransform() * link.solid->getLocalTransform().inverse();
        btTransform thrustTrans = linkTrans * pos;
        
        attachFE->AddLinkForce(linkId, thrustTrans.getBasis() * thrustV);
        attachFE->AddLinkTorque(linkId, (thrustTrans.getOrigin() - linkTrans.getOrigin()).cross(thrustTrans.getBasis() * thrustV));
        attachFE->AddLinkTorque(linkId, thrustTrans.getBasis() * torqueV);
    }
}