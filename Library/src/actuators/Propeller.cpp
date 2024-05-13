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
//  Propeller.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 06/05/2019.
//  Copyright (c) 2019-2024 Patryk Cieslak. All rights reserved.
//

#include "actuators/Propeller.h"

#include "core/SimulationApp.h"
#include "core/SimulationManager.h"
#include "graphics/GLSLShader.h"
#include "graphics/OpenGLContent.h"
#include "entities/SolidEntity.h"

namespace sf
{

Propeller::Propeller(std::string uniqueName, SolidEntity* propeller, Scalar diameter, Scalar thrustCoeff, Scalar torqueCoeff, Scalar maxRPM, bool rightHand, bool inverted) : LinkActuator(uniqueName)
{
    D = diameter;
    kT0 = thrustCoeff;
    kQ0 = torqueCoeff;
    kp = Scalar(8.0);
    ki = Scalar(3.0);
    iLim = Scalar(2.0);
    RH = rightHand;
    inv = inverted;
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

Propeller::~Propeller()
{
    if(prop != nullptr)
        delete prop;
}

ActuatorType Propeller::getType() const
{
    return ActuatorType::PROPELLER;
}

void Propeller::setSetpoint(Scalar s)
{
    if(inv) s *= Scalar(-1);
    setpoint = s < Scalar(-1) ? Scalar(-1) : (s > Scalar(1) ? Scalar(1) : s);
    ResetWatchdog();
}

Scalar Propeller::getSetpoint() const
{
    return inv ? -setpoint : setpoint;
}

Scalar Propeller::getAngle() const
{
    return theta;
}

Scalar Propeller::getOmega() const
{
    return omega;
}

Scalar Propeller::getThrust() const
{
    return thrust;
}

Scalar Propeller::getTorque() const
{
    return torque;
}

void Propeller::Update(Scalar dt)
{
    Actuator::Update(dt);

    //Update thruster velocity
    Scalar error = setpoint * omegaLim - omega;
    Scalar motorTorque = kp * error + ki * iError;
    omega += (motorTorque - torque)*dt;
    theta += omega * dt; //Just for animation
    
    //Integrate error
    iError += error * dt;
    iError = iError > iLim ? iLim : (iError < -iLim ? -iLim : iError);
    
    if(attach != NULL)
    {
        //Get transforms
        Transform solidTrans = attach->getCGTransform();
        Transform propTrans = attach->getOTransform() * o2a;
        Vector3 relPos = propTrans.getOrigin() - solidTrans.getOrigin();
        Vector3 velocity = attach->getLinearVelocityInLocalPoint(relPos);
        
        Atmosphere* atm = SimulationApp::getApp()->getSimulationManager()->getAtmosphere();
        
        if(atm->IsInsideFluid(propTrans.getOrigin()))
        {
            //Calculate thrust
            //Thrust coefficient depends on advance ratio J = u/(n*D), kT0 -> J=0
            //The lower the p/D the more linear the dependence of Kt on J
            Scalar n = omega/(Scalar(2) * M_PI);
            Scalar u = -propTrans.getBasis().getColumn(0).dot(atm->GetFluidVelocity(propTrans.getOrigin()) - velocity); //Incoming air velocity
            Scalar alpha(-0.095/0.8);
            //kT(J) = kT0 + alpha * J --> approximated with linear function
            thrust = (RH ? Scalar(1) : Scalar(-1)) * atm->getGas().density * D*D*D * btFabs(n) * (D*kT0*n + alpha*u);
            torque = -kQ0 * atm->getGas().density * btFabs(n)*n * D*D*D*D*D; //Torque is the loading of propeller due to drag
            
            //Apply forces and torques
            Vector3 thrustV(thrust, 0, 0);
            Vector3 torqueV(torque, 0, 0); 
            attach->ApplyCentralForce(propTrans.getBasis() * thrustV);
            attach->ApplyTorque((propTrans.getOrigin() - solidTrans.getOrigin()).cross(propTrans.getBasis() * thrustV));
            attach->ApplyTorque(propTrans.getBasis() * torqueV);
        }
    }
}

std::vector<Renderable> Propeller::Render()
{
    Transform propTrans = Transform::getIdentity();
    if(attach != nullptr)
        propTrans = attach->getOTransform() * o2a;
    else
        LinkActuator::Render();
    
    //Rotate propeller
    propTrans *= Transform(Quaternion(0, 0, theta), Vector3(0,0,0));
    
    //Add renderable
    std::vector<Renderable> items(0);
    Renderable item;
    item.type = RenderableType::SOLID;
    item.materialName = prop->getMaterial().name;
    item.objectId = prop->getGraphicalObject();
    item.lookId = dm == DisplayMode::GRAPHICAL ? prop->getLook() : -1;
	item.model = glMatrixFromTransform(propTrans);
    items.push_back(item);
    
    item.type = RenderableType::ACTUATOR_LINES;
    item.points.push_back(glm::vec3(0,0,0));
    item.points.push_back(glm::vec3(0.1f*thrust,0,0));
    items.push_back(item);
    
    return items;
}
    
void Propeller::WatchdogTimeout()
{
    setSetpoint(Scalar(0));
}

}
