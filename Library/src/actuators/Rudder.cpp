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
//  Copyright (c) 2021-2025 Nils Bore, Patryk Cieslak. All rights reserved.
//

#include "actuators/Rudder.h"

#include "core/SimulationApp.h"
#include "core/SimulationManager.h"
#include "graphics/GLSLShader.h"
#include "graphics/OpenGLContent.h"
#include "entities/SolidEntity.h"

namespace sf
{

Rudder::Rudder(std::string uniqueName, SolidEntity* rudder, Scalar area, Scalar liftCoeff, Scalar dragCoeff, Scalar stallAngle, 
    Scalar maxAngle, bool inverted, Scalar maxAngularRate) : LinkActuator(uniqueName)
{
    this->dragCoeff_ = dragCoeff;
    this->liftCoeff_ = liftCoeff;
    this->area_ = area;
    this->stallAngle_ = stallAngle;
    this->maxAngle_ = maxAngle;
    this->maxAngularRate_ = btFabs(maxAngularRate);
    inv_ = inverted;

    setpoint_ = Scalar(0);
    theta_ = Scalar(0);
    this->rudder_ = rudder;
    this->rudder_->BuildGraphicalObject();
}

Rudder::~Rudder()
{
    if(rudder_ != nullptr)
        delete rudder_;
}

ActuatorType Rudder::getType() const
{
    return ActuatorType::RUDDER;
}

void Rudder::setSetpoint(Scalar s)
{
    if(inv_) s *= Scalar(-1);
    setpoint_ = std::max(std::min(s, maxAngle_), -maxAngle_);
}

Scalar Rudder::getSetpoint() const
{
    return inv_ ? -setpoint_ : setpoint_;
}

Scalar Rudder::getAngle() const
{
    return theta_;
}

void Rudder::Update(Scalar dt)
{
    //Update rudder angle
    if(maxAngularRate_ > Scalar(0) && btFabs(setpoint_-theta_)/dt > maxAngularRate_)
    {
        Scalar dTheta = setpoint_-theta_ > Scalar(0) ? maxAngularRate_ * dt : -maxAngularRate_ * dt;
        theta_ += dTheta;
    }
    else
        theta_ = setpoint_;

    if(attach_ != NULL)
    {
        Quaternion rudderRot(theta_, 0, 0);

        //Get transforms
        Transform solidTrans = attach_->getCGTransform();
        // o2a is the transform of the actuator
        Transform rudderTrans = attach_->getOTransform() * o2a_ * Transform(rudderRot);
        Vector3 relPos = rudderTrans.getOrigin() - solidTrans.getOrigin();

        Ocean* ocn = SimulationApp::getApp()->getSimulationManager()->getOcean();

        Vector3 absVel = attach_->getLinearVelocityInLocalPoint(relPos);
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
            Scalar X = .5 * area_ * rho * liftCoeff_;

            // Equation 3.2.19
            Scalar lift = du2 * X;

            // Equation 3.2.21
            Scalar drag = angle * du2 * X;

            // Drag is opposite to velocity
            dragV_ = -velocity;
            dragV_.normalize() *= drag;

            // Lift is restricted to XY plane
            // liftV = (-vy, vx, 0)
            liftV_ = Vector3(0., 0., -1.).cross(velocity);
            liftV_.normalize() *= lift;

            // if angle greater than stall -> no lift
            if (fabs(angle) > stallAngle_)
            {
                liftV_ = Vector3(0, 0, 0);
            }

            // Torque vectors
            Vector3 dragT = relPos.cross(rudderTrans.getBasis() * dragV_);
            Vector3 liftT = relPos.cross(rudderTrans.getBasis() * liftV_);

            // Apply forces and torques
            attach_->ApplyTorque(dragT);
            attach_->ApplyTorque(liftT);
            attach_->ApplyCentralForce(rudderTrans.getBasis() * dragV_);
            attach_->ApplyCentralForce(rudderTrans.getBasis() * liftV_);
        }
        else
        {
            dragV_ = Vector3(0, 0, 0);
            liftV_ = Vector3(0, 0, 0);
        }
    }
}

std::vector<Renderable> Rudder::Render()
{
    Transform rudderTrans = Transform::getIdentity();
    if(attach_ != NULL)
        rudderTrans = attach_->getOTransform() * o2a_;
    else
        LinkActuator::Render();
    
    //Rotate rudder
    rudderTrans *= Transform(Quaternion(theta_, 0, 0)) * rudder_->getO2GTransform();
    
    //Add renderable
    std::vector<Renderable> items(0);
    Renderable item;
    item.type = RenderableType::SOLID;
    item.materialName = rudder_->getMaterial().name;
    item.objectId = rudder_->getGraphicalObject();
    item.lookId = dm_ == DisplayMode::GRAPHICAL ? rudder_->getLook() : -1;
	item.model = glMatrixFromTransform(rudderTrans);
    items.push_back(item);
    
    item.type = RenderableType::ACTUATOR_LINES;
    item.data = std::make_shared<std::vector<glm::vec3>>();
    auto points = item.getDataAsPoints();
    points->push_back(glm::vec3(0,0,0));
    Vector3 VG = .1*(rudder_->getO2GTransform().inverse().getBasis()*(liftV_ + dragV_));
    points->push_back(glm::vec3(VG.getX(),VG.getY(),VG.getZ()));
    items.push_back(item);
    
    return items;
}
    
}
