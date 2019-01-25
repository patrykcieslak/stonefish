//
//  Thruster.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 10/10/2017.
//  Copyright (c) 2017-2018 Patryk Cieslak. All rights reserved.
//

#include "actuators/Thruster.h"

#include "core/SimulationApp.h"
#include "core/SimulationManager.h"
#include "graphics/GLSLShader.h"
#include "graphics/OpenGLContent.h"
#include "entities/SolidEntity.h"

namespace sf
{

Thruster::Thruster(std::string uniqueName, SolidEntity* propeller, Scalar diameter, Scalar thrustCoeff, Scalar torqueCoeff, Scalar maxRPM) : LinkActuator(uniqueName)
{
    D = diameter;
    kT = thrustCoeff;
    kQ = torqueCoeff;
    kp = Scalar(0.01); //Scalar(15.0);
    ki = Scalar(0.0); //Scalar(3.0);
    iLim = Scalar(10.0);
    omegaLim = maxRPM/Scalar(60) * Scalar(2) * M_PI; //In rad/s
    
    theta = Scalar(0);
    omega = Scalar(0);
    thrust = Scalar(0);
    torque = Scalar(0);
    setpoint = Scalar(0);
    iError = Scalar(0);
    
    prop = propeller;
    prop->BuildGraphicalObject();
}

Thruster::~Thruster()
{
    if(prop != NULL)
        delete prop;
}

void Thruster::setSetpoint(Scalar s)
{
    setpoint = s < Scalar(-1) ? Scalar(-1) : (s > Scalar(1) ? Scalar(1) : s);
}

Scalar Thruster::getSetpoint()
{
    return setpoint;
}

Scalar Thruster::getAngle()
{
    return theta;
}

Scalar Thruster::getOmega()
{
    return omega;
}

Scalar Thruster::getThrust()
{
    return thrust;
}

void Thruster::Update(Scalar dt)
{
    //Update thruster velocity
    Scalar error = setpoint * omegaLim - omega;
    Scalar motorTorque = kp * error + ki * iError; //error*kp*btFabs(setpoint) + iError*ki*btFabs(setpoint); //Seaeye mimicking
    omega += (motorTorque - torque)/prop->getInertia().x() * dt; //Damping due to axial torque
    theta += omega * dt; //Just for animation
    
    //Integrate error
    iError += error * dt;
    iError = iError > iLim ? iLim : iError;
    
    if(attach != NULL)
    {
        //Get transforms
        Transform solidTrans = attach->getCGTransform();
        Transform thrustTrans = attach->getOTransform() * o2a;
        Vector3 relPos = thrustTrans.getOrigin() - solidTrans.getOrigin();
        Vector3 velocity = attach->getLinearVelocityInLocalPoint(relPos);
        
        //Calculate thrust
        Ocean* liquid = SimulationApp::getApp()->getSimulationManager()->getOcean();
        
        if(liquid->IsInsideFluid(thrustTrans.getOrigin()))
        {
            //kT depends on advance ratio J = u/(omega*D)
            Scalar A = M_PI*D*D/Scalar(4);
            Scalar k1(-0.3);
            Scalar k2(-0.3);
            Scalar k3 = Scalar(2)*kT/M_PI;
            Scalar u = -thrustTrans.getBasis().getColumn(0).dot(liquid->GetFluidVelocity(thrustTrans.getOrigin()) - velocity); //Incoming fluid velocity
            Scalar rate = omega/(Scalar(2) * M_PI);
            thrust = Scalar(2) * liquid->getLiquid()->density * A * (k1*u*u + k2*u*D*rate + k3*D*D*rate*rate);
            
            if(omega < Scalar(0))
                thrust = -thrust;
            
            //Scalar kt = thrust/(liquid->getFluid()->density * D*D*D*D * btFabs(omega)*omega);
            //std::cout << getName() << " omega: " << omega << " u:" << u << " kT:" << kt << std::endl;
            
            //thrust = liquid->getFluid()->density * kT * btFabs(omega)*omega * D*D*D*D;
            torque = liquid->getLiquid()->density * kQ * btFabs(rate)*rate * D*D*D*D*D;
            Vector3 thrustV(thrust, 0, 0);
            Vector3 torqueV(-torque, 0, 0); //Torque is the loading of propeller due to water drag
            
            //Apply forces and torques
            attach->ApplyCentralForce(thrustTrans.getBasis() * thrustV);
            attach->ApplyTorque((thrustTrans.getOrigin() - solidTrans.getOrigin()).cross(thrustTrans.getBasis() * thrustV));
            attach->ApplyTorque(thrustTrans.getBasis() * torqueV);
            
            printf("Thrust: %1.3f Torque: %1.3f\n Speed: % 1.3f", thrustV.getX(), torqueV.getX(), omega);
        }
    }
}

std::vector<Renderable> Thruster::Render()
{
    Transform thrustTrans = Transform::getIdentity();
    if(attach != NULL)
        thrustTrans = attach->getOTransform() * o2a;
    else
        LinkActuator::Render();
    
    //Rotate propeller
    thrustTrans *= Transform(Quaternion(0, 0, theta), Vector3(0,0,0));
    
    //Add renderable
    std::vector<Renderable> items(0);
    Renderable item;
    item.type = RenderableType::SOLID;
    item.objectId = prop->getObject();
    item.lookId = prop->getLook();
	item.model = glMatrixFromTransform(thrustTrans);
    items.push_back(item);
    
    item.type = RenderableType::ACTUATOR_LINES;
    item.points.push_back(glm::vec3(0,0,0));
    item.points.push_back(glm::vec3(0.1f*thrust,0,0));
    items.push_back(item);
    
    return items;
}
    
}
