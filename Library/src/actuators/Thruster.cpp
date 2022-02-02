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
//  Copyright (c) 2017-2020 Patryk Cieslak. All rights reserved.
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
    alpha = Scalar(-1) * kT.first;
    beta = Scalar(-1) * kQ;
    kp = Scalar(8.0);
    ki = Scalar(3.0);
    iLim = Scalar(2.0);
    RH = rightHand;
    inv = inverted;
    omegaLim = maxRPM/Scalar(60) * Scalar(2) * M_PI; //[rad/s] (always positive)

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
    return ActuatorType::THRUSTER;
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

Scalar Thruster::getTorque()
{
    return torque;
}

Scalar Thruster::getDiameter()
{
    return D;
}

bool Thruster::isPropellerRight()
{
    return RH;
}

void Thruster::Update(Scalar dt)
{
    if(attach != NULL)
    {
        //Update thruster velocity
        Scalar error = setpoint * omegaLim - omega;
        Scalar motorTorque = kp * error + ki * iError;
        omega += (motorTorque - (-torque))*dt;
        theta += omega * dt; //Just for animation
    
        //Integrate error
        iError += error * dt;
        iError = iError > iLim ? iLim : (iError < -iLim ? -iLim : iError);
        
        //Get transforms
        Transform solidTrans = attach->getCGTransform();
        Transform thrustTrans = attach->getOTransform() * o2a;
        Vector3 relPos = thrustTrans.getOrigin() - solidTrans.getOrigin();
        Vector3 velocity = attach->getLinearVelocityInLocalPoint(relPos);
        
        //Calculate thrust
        Ocean* ocn = SimulationApp::getApp()->getSimulationManager()->getOcean();
        if(ocn != nullptr && ocn->IsInsideFluid(thrustTrans.getOrigin()))
        {
            bool backward = (RH && omega < Scalar(0)) || (!RH && omega > Scalar(0));
            
            /*kT and kQ depend on the advance ratio J
                J = u/(omega*D), where:
                u - ambient velocity [m/s]
                n - propeller rotational rate [1/s]
                D - propeller diameter [m] */
            Scalar n = (backward ? Scalar(-1) : Scalar(1)) * btFabs(omega)/(Scalar(2) * M_PI); // Accounts for propoller handedness
            Scalar u = -thrustTrans.getBasis().getColumn(0).dot(ocn->GetFluidVelocity(thrustTrans.getOrigin()) - velocity); //Incoming water velocity
            
            //Thrust
            Scalar kT0 = backward ? kT.second : kT.first; //In case of non-symmetrical thrusters the coefficient may be different
            //kT(J) = kT0 + alpha * J --> approximated with linear function
            thrust = ocn->getLiquid().density * D*D*D * btFabs(n) * (D*kT0*n + alpha*u);
            
            //Torque
            Scalar kQ0 = kQ;
            //kQ(J) = kQ0 + beta * J --> approximated with linear function
            torque = (RH ? Scalar(-1) : Scalar(1)) * ocn->getLiquid().density * D*D*D*D * btFabs(n) * (D*kQ0*n + beta*u); //Torque is the loading of propeller due to water resistance (reaction force)

            //Apply forces and torques
            Vector3 thrustV(thrust, 0, 0);
            Vector3 torqueV(torque, 0, 0);
            attach->ApplyCentralForce(thrustTrans.getBasis() * thrustV);
            attach->ApplyTorque((thrustTrans.getOrigin() - solidTrans.getOrigin()).cross(thrustTrans.getBasis() * thrustV));
            attach->ApplyTorque(thrustTrans.getBasis() * torqueV);
        }
        else
        {
            thrust = Scalar(0);
            torque = Scalar(0);
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
    item.lookId = dm == DisplayMode::GRAPHICAL ? prop->getLook() : -1;
	item.model = glMatrixFromTransform(thrustTrans);
    items.push_back(item);
    
    item.type = RenderableType::ACTUATOR_LINES;
    item.points.push_back(glm::vec3(0,0,0));
    item.points.push_back(glm::vec3(0.1f*thrust,0,0));
    items.push_back(item);
    
    return items;
}
    
}
