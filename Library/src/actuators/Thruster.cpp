//
//  Thruster.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 10/10/2017.
//  Copyright (c) 2017 Patryk Cieslak. All rights reserved.
//

#include "actuators/Thruster.h"

#include "core/SimulationApp.h"
#include "graphics/OpenGLContent.h"
#include "utils/MathsUtil.hpp"

Thruster::Thruster(std::string uniqueName, SolidEntity* propeller, btScalar diameter, btScalar thrustCoeff, btScalar torqueCoeff, btScalar maxRPM) : Actuator(uniqueName)
{
    D = UnitSystem::SetLength(diameter);
    kT = thrustCoeff;
    kQ = torqueCoeff;
    kp = btScalar(15.0);
    ki = btScalar(3.0);
    iLim = btScalar(10.0);
    omegaLim = maxRPM/btScalar(60); //In 1/s
    
    theta = btScalar(0);
    omega = btScalar(0);
    thrust = btScalar(0);
    torque = btScalar(0);
    setpoint = btScalar(0);
    iError = btScalar(0);
    
    prop = propeller;
    prop->BuildGraphicalObject();
    attach = NULL;
    attachFE = NULL;
    linkId = 0;
    pos.setIdentity();
}

Thruster::~Thruster()
{
    if(prop != NULL)
        delete prop;
}

ActuatorType Thruster::getType()
{
    return ACTUATOR_THRUSTER;
}


void Thruster::setSetpoint(btScalar value)
{
    setpoint = value < btScalar(-1) ? btScalar(-1) : (value > btScalar(1) ? btScalar(1) : value);
}

btScalar Thruster::getSetpoint()
{
    return setpoint;
}

btScalar Thruster::getAngle()
{
    return theta;
}

btScalar Thruster::getOmega()
{
    return omega;
}

btScalar Thruster::getThrust()
{
    return thrust;
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
        thrustTrans = attach->getTransform() * attach->getGeomToCOGTransform().inverse() * pos;
    }
    else if(attachFE != NULL)
    {
        FeatherstoneLink link = attachFE->getLink(linkId);
        thrustTrans = link.solid->getTransform() * link.solid->getGeomToCOGTransform().inverse() * pos;
    }
    else
    {
        return items;
    }
    
    //Rotate propeller
    thrustTrans *= btTransform(btQuaternion(0, 0, theta), btVector3(0,0,0));
    
    //Add renderable
    Renderable item;
    item.type = RenderableType::SOLID;
    item.objectId = prop->getObject();
    item.lookId = prop->getLook();
	item.model = glMatrixFromBtTransform(thrustTrans);
    items.push_back(item);
    
    item.type = RenderableType::ACTUATOR_LINES;
    item.points.push_back(glm::vec3(0,0,0));
    item.points.push_back(glm::vec3(0.1f*thrust,0,0));
    items.push_back(item);
    
    return items;
}

void Thruster::Update(btScalar dt)
{
    //Update thruster velocity
    btScalar error = setpoint*omegaLim - omega;
    btScalar epsilon = error*kp + iError*ki; //error*kp*btFabs(setpoint) + iError*ki*btFabs(setpoint); //Seaeye mimicking
    iError += error*dt;
    iError = iError > iLim ? iLim : iError;
    
    omega += (epsilon - torque)*dt; //Damping due to axial torque
    theta += (omega * btScalar(2) * M_PI)*dt; //Just for animation (in Radians)
    
    //Get transforms
    btTransform solidTrans;
    btTransform thrustTrans;
    btVector3 velocity;
    
    if(attach != NULL)
    {
        solidTrans = attach->getTransform();
        thrustTrans = attach->getTransform() * attach->getGeomToCOGTransform().inverse() * pos;
        btVector3 relPos = thrustTrans.getOrigin() - solidTrans.getOrigin();
        velocity = attach->getLinearVelocityInLocalPoint(relPos);
    }
    else if(attachFE != NULL)
    {
        FeatherstoneLink link = attachFE->getLink(linkId);
        solidTrans = link.solid->getTransform();
        thrustTrans = link.solid->getTransform() * link.solid->getGeomToCOGTransform().inverse() * pos;
        btVector3 relPos = thrustTrans.getOrigin() - solidTrans.getOrigin();
        velocity = link.solid->getLinearVelocityInLocalPoint(relPos);
    }
    else 
        return;

    //Calculate thrust
    Ocean* liquid = SimulationApp::getApp()->getSimulationManager()->getOcean();
    
    if(liquid->IsInsideFluid(thrustTrans.getOrigin()))
    {
        //kT depends on advance ratio J = u/(omega*D)
        btScalar A = M_PI*D*D/btScalar(4);
        btScalar k1(-0.3);
        btScalar k2(-0.3);
        btScalar k3 = btScalar(2)*kT/M_PI;
        btScalar u = -thrustTrans.getBasis().getColumn(0).dot(liquid->GetFluidVelocity(thrustTrans.getOrigin()) - velocity); //Incoming fluid velocity
        thrust = btScalar(2) * liquid->getLiquid()->density * A * (k1*u*u + k2*u*D*omega + k3*D*D*omega*omega);
        if(omega < btScalar(0))
            thrust = -thrust;
            
        //btScalar kt = thrust/(liquid->getFluid()->density * D*D*D*D * btFabs(omega)*omega);
        //std::cout << getName() << " omega: " << omega << " u:" << u << " kT:" << kt << std::endl;
        
        //thrust = liquid->getFluid()->density * kT * btFabs(omega)*omega * D*D*D*D;
        torque = liquid->getLiquid()->density * kQ * btFabs(omega)*omega * D*D*D*D*D;
        btVector3 thrustV(thrust, 0, 0);
        btVector3 torqueV(-torque, 0, 0); //Torque is the loading of propeller due to water drag
    
        //Apply forces and torques
        if(attach != NULL)
        {
            attach->ApplyCentralForce(thrustTrans.getBasis() * thrustV);
            attach->ApplyTorque((thrustTrans.getOrigin() - solidTrans.getOrigin()).cross(thrustTrans.getBasis() * thrustV));
            attach->ApplyTorque(thrustTrans.getBasis() * torqueV);
        }
        else
        {
            attachFE->AddLinkForce(linkId, thrustTrans.getBasis() * thrustV);
            attachFE->AddLinkTorque(linkId, (thrustTrans.getOrigin() - solidTrans.getOrigin()).cross(thrustTrans.getBasis() * thrustV));
            attachFE->AddLinkTorque(linkId, thrustTrans.getBasis() * torqueV);
        }
    }
}
