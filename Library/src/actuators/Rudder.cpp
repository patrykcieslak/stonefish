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
//  Rudder.cpp
//  Stonefish
//
//  Created by Nils Bore on 29/01/2021.
//  Copyright (c) 2021-2023 Nils Bore, Patryk Cieslak. All rights reserved.
//

#include "actuators/Rudder.h"

#include "core/SimulationApp.h"
#include "core/SimulationManager.h"
#include "graphics/GLSLShader.h"
#include "graphics/OpenGLContent.h"
#include "entities/SolidEntity.h"

namespace sf
{

Rudder::Rudder(std::string uniqueName, SolidEntity* rudder, Scalar area, Scalar liftCoeff, Scalar dragCoeff, Scalar stallAngle, Scalar maxAngle, bool inverted) : LinkActuator(uniqueName)
{
    this->dragCoeff = dragCoeff;
    this->liftCoeff = liftCoeff;
    this->area = area;
    this->stallAngle = stallAngle;
    this->maxAngle = maxAngle;
    inv = inverted;

    setpoint = Scalar(0);
    theta = Scalar(0);
    this->rudder = rudder;
    this->rudder->BuildGraphicalObject();
}

Rudder::~Rudder()
{
    if(rudder != nullptr)
        delete rudder;
}

ActuatorType Rudder::getType() const
{
    return ActuatorType::RUDDER;
}

void Rudder::setSetpoint(Scalar s)
{
    if(inv) s *= Scalar(-1);
    setpoint = std::max(std::min(s, maxAngle), -maxAngle);
}

Scalar Rudder::getSetpoint() const
{
    return inv ? -setpoint : setpoint;
}

Scalar Rudder::getAngle() const
{
    return theta;
}

void Rudder::Update(Scalar dt)
{
    //Update rudder angle
    theta = setpoint;

    if(attach != NULL)
    {
        Quaternion rudderRot(theta, 0, 0);

        //Get transforms
        Transform solidTrans = attach->getCGTransform();
        // o2a is the transform of the actuator
        Transform rudderTrans = attach->getOTransform() * o2a * Transform(rudderRot);
        Vector3 relPos = rudderTrans.getOrigin() - solidTrans.getOrigin();

        Ocean* ocn = SimulationApp::getApp()->getSimulationManager()->getOcean();

        Vector3 absVel = attach->getLinearVelocityInLocalPoint(relPos);
        Vector3 fluidVel = ocn->GetFluidVelocity(rudderTrans.getOrigin());
        Vector3 velocity = rudderTrans.getBasis().transpose()*(absVel - fluidVel);

        Scalar angle = atan2(velocity.getY(), velocity.getX());

        // If rudder facing velocity, reverse equations
        if (fabs(angle) > .5*M_PI)
        {
            angle = atan2(-velocity.getY(), -velocity.getX());
        }

        if(ocn != nullptr && ocn->IsInsideFluid(rudderTrans.getOrigin()) && !velocity.isZero())
        {
            // Calculate quadratic approximations for lift and drag

            // Equations from:
            // Engelhardtsen, Oystein. 3D AUV Collision Avoidance.
            // MS thesis. Institutt for teknisk kybernetikk, 2007.

            // Fluid density
            Scalar rho = ocn->getLiquid().density;

            // Speed
            Scalar u = velocity.length();

            // d = angle
            Scalar du2 = angle * u*u;

            // Approximated rudder parameter
            Scalar X = .5 * area * rho * liftCoeff;

            // Equation 3.2.19
            Scalar lift = du2 * X;

            // Equation 3.2.21
            Scalar drag = angle * du2 * X;

            // Drag is opposite to velocity
            dragV = -velocity;
            dragV.normalize() *= drag;

            // Lift is restricted to XY plane
            // liftV = (-vy, vx, 0)
            liftV = Vector3(0., 0., -1.).cross(velocity);
            liftV.normalize() *= lift;

            // if angle greater than stall -> no lift
            if (fabs(angle) > stallAngle)
            {
                liftV = Vector3(0, 0, 0);
            }

            // Torque vectors
            Vector3 dragT = relPos.cross(rudderTrans.getBasis() * dragV);
            Vector3 liftT = relPos.cross(rudderTrans.getBasis() * liftV);

            // Apply forces and torques
            attach->ApplyTorque(dragT);
            attach->ApplyTorque(liftT);
            attach->ApplyCentralForce(rudderTrans.getBasis() * dragV);
            attach->ApplyCentralForce(rudderTrans.getBasis() * liftV);
        }
        else
        {
            dragV = Vector3(0, 0, 0);
            liftV = Vector3(0, 0, 0);
        }
    }
}

std::vector<Renderable> Rudder::Render()
{
    Transform rudderTrans = Transform::getIdentity();
    if(attach != NULL)
        rudderTrans = attach->getOTransform() * o2a;
    else
        LinkActuator::Render();
    
    //Rotate rudder
    rudderTrans *= Transform(Quaternion(theta, 0, 0)) * rudder->getO2GTransform();
    
    //Add renderable
    std::vector<Renderable> items(0);
    Renderable item;
    item.type = RenderableType::SOLID;
    item.materialName = rudder->getMaterial().name;
    item.objectId = rudder->getGraphicalObject();
    item.lookId = dm == DisplayMode::GRAPHICAL ? rudder->getLook() : -1;
	item.model = glMatrixFromTransform(rudderTrans);
    items.push_back(item);
    
    item.type = RenderableType::ACTUATOR_LINES;
    item.points.push_back(glm::vec3(0,0,0));
    Vector3 VG = .1*(rudder->getO2GTransform().inverse().getBasis()*(liftV + dragV));
    item.points.push_back(glm::vec3(VG.getX(),VG.getY(),VG.getZ()));
    items.push_back(item);
    
    return items;
}
    
}
