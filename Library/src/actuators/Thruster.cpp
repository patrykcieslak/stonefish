/*    
    This file is a part of Stonefish.

    Stonefish is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Stonefish is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

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

Thruster::Thruster(std::string uniqueName, SolidEntity* propeller, Scalar diameter, std::pair<Scalar, Scalar> thrustCoeff, Scalar torqueCoeff, Scalar maxRPM, bool rightHand, bool inverted) : LinkActuator(uniqueName)
{
    D = diameter;
    kT = thrustCoeff;
    kQ = torqueCoeff;
    kp = Scalar(3.0);
    ki = Scalar(2.0);
    iLim = Scalar(10.0);
    RH = rightHand;
    inv = inverted;
    omegaLim = (RH ? Scalar(1.0) : Scalar(-1.0)) *  maxRPM/Scalar(60) * Scalar(2) * M_PI; //In rad/s

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

ActuatorType Thruster::getType()
{
    return ActuatorType::ACTUATOR_THRUSTER;
}

void Thruster::setSetpoint(Scalar s)
{
    if(inv) s *= Scalar(-1);
    setpoint = s < Scalar(-1) ? Scalar(-1) : (s > Scalar(1) ? Scalar(1) : s);
}

Scalar Thruster::getSetpoint()
{
    return inv ? -setpoint : setpoint;
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
    if(attach != NULL)
    {
        //Update thruster velocity
        Scalar error = setpoint * omegaLim - omega;
        Scalar motorTorque = kp * error + ki * iError; //error*kp*btFabs(setpoint) + iError*ki*btFabs(setpoint); //Seaeye mimicking
        omega += (motorTorque - torque)*dt; ///prop->getInertia().x() * dt; //Damping due to axial torque
        theta += omega * dt; //Just for animation
    
        //Integrate error
        iError += error * dt;
        iError = iError > iLim ? iLim : iError;
        
        //Get transforms
        Transform solidTrans = attach->getCGTransform();
        Transform thrustTrans = attach->getOTransform() * o2a;
        Vector3 relPos = thrustTrans.getOrigin() - solidTrans.getOrigin();
        Vector3 velocity = attach->getLinearVelocityInLocalPoint(relPos);
        
        //Calculate thrust
        Ocean* liquid = SimulationApp::getApp()->getSimulationManager()->getOcean();
        
        if(liquid->IsInsideFluid(thrustTrans.getOrigin()))
        {
            bool backward = (RH && omega < Scalar(0)) || (!RH && omega > Scalar(0));
            
            //kT depends on advance ratio J = u/(omega*D)
            Scalar kT0 = backward ? kT.second : kT.first;            
            Scalar A = M_PI*D*D/Scalar(4);
            Scalar k1(-0.3);
            Scalar k2(-0.3);
            Scalar k3 = Scalar(2)*kT0/M_PI;
            Scalar u = -thrustTrans.getBasis().getColumn(0).dot(liquid->GetFluidVelocity(thrustTrans.getOrigin()) - velocity); //Incoming fluid velocity
            
            Scalar rate = (RH ? Scalar(1.0) : Scalar(-1.0)) * omega/(Scalar(2) * M_PI);
            thrust = Scalar(2) * liquid->getLiquid().density * A * (k1*u*u + k2*u*D*rate + k3*D*D*rate*rate);
            if(backward) 
                thrust = -thrust;
            
            //Scalar kt = thrust/(liquid->getFluid()->density * D*D*D*D * btFabs(omega)*omega);
            //std::cout << getName() << " omega: " << omega << " u:" << u << " kT:" << kt << std::endl;
            //thrust = liquid->getFluid()->density * kT * btFabs(omega)*omega * D*D*D*D;
            
            torque = liquid->getLiquid().density * kQ * btFabs(rate)*rate * D*D*D*D*D;
            if(!RH)
                torque = -torque;
            
            Vector3 thrustV(thrust, 0, 0);
            Vector3 torqueV(-torque, 0, 0); //Torque is the loading of propeller due to water drag
            
            //Apply forces and torques
            attach->ApplyCentralForce(thrustTrans.getBasis() * thrustV);
            attach->ApplyTorque((thrustTrans.getOrigin() - solidTrans.getOrigin()).cross(thrustTrans.getBasis() * thrustV));
            attach->ApplyTorque(thrustTrans.getBasis() * torqueV);
            
            //printf("%s setpoint: %1.3lf thrust: %1.3lf torque: %1.3lf\n", getName().c_str(), setpoint, thrust, torque);
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
    item.materialName = prop->getMaterial().name;
    item.objectId = prop->getGraphicalObject();
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
