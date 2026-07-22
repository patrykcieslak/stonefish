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
//  Copyright (c) 2021-2026 Nils Bore, Patryk Cieslak. All rights reserved.
//

#include "actuators/Rudder.h"

#include "core/SimulationApp.h"
#include "core/SimulationManager.h"
#include "core/DeviceFactory.h"
#include "graphics/GLSLShader.h"
#include "graphics/OpenGLContent.h"
#include "entities/SolidEntity.h"
#include "entities/solids/Polyhedron.h"
#include "utils/SystemUtil.hpp"

namespace sf
{

Rudder::Rudder(const std::string& uniqueName, std::unique_ptr<SolidEntity> rudder, Scalar area, Scalar liftCoeff, Scalar dragCoeff, Scalar stallAngle, 
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
    rudder_ = std::move(rudder);
    rudder_->BuildGraphicalObject();
}

LinkActuatorType Rudder::getLinkActuatorType() const
{
    return LinkActuatorType::RUDDER;
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

// Statics

ConstructInfo Rudder::getConstructInfo()
{
    ConstructInfo info;
    ConstructInfoNode node;
    
    // Specs
    node.optional = false;
    node.attributes.insert({"drag_coeff", {ConstructInfoValueType::SCALAR, false}});
    node.attributes.insert({"lift_coeff", {ConstructInfoValueType::SCALAR, false}});
    node.attributes.insert({"area", {ConstructInfoValueType::SCALAR, false}});
    node.attributes.insert({"max_angle", {ConstructInfoValueType::SCALAR, false}});
    node.attributes.insert({"stall_angle", {ConstructInfoValueType::SCALAR, true}});
    node.attributes.insert({"max_angular_rate", {ConstructInfoValueType::SCALAR, true}});
    node.attributes.insert({"inverted", {ConstructInfoValueType::BOOL, true}});
    info.nodes.insert({"specs", node});

    // Visual
    node.attributes.clear();
    node.optional = false;

    ConstructInfoNode childNode;
    childNode.optional = false;
    childNode.attributes.insert({"filename", {ConstructInfoValueType::STRING, false}});
    childNode.attributes.insert({"scale", {ConstructInfoValueType::SCALAR, true}});
    node.childNodes.insert({"mesh", childNode});

    childNode.attributes.clear();
    childNode.attributes.insert({"name", {ConstructInfoValueType::STRING, false}});
    node.childNodes.insert({"material", childNode});
    node.childNodes.insert({"look", childNode});
    
    childNode.attributes.clear();
    childNode.attributes.insert({"T", {ConstructInfoValueType::TRANSFORM, false}});
    node.childNodes.insert({"origin", childNode});

    info.nodes.insert({"visual", node});

    return info;
}

std::unique_ptr<Rudder> Rudder::Construct(const std::string& uniqueName, ConstructInfo& info)
{
    // Required
    Scalar dragCoeff = std::get<Scalar>(info.nodes.at("specs").attributes.at("drag_coeff").value);
    Scalar liftCoeff = std::get<Scalar>(info.nodes.at("specs").attributes.at("lift_coeff").value);
    Scalar area = std::get<Scalar>(info.nodes.at("specs").attributes.at("area").value);
    Scalar maxAngle = std::get<Scalar>(info.nodes.at("specs").attributes.at("max_angle").value);

    // Optional
    Scalar stallAngle = Scalar(M_PI_4);
    Scalar maxAngularRate = Scalar(0.);
    bool inverted = false;

    ConstructInfoValue& value = info.nodes.at("specs").attributes.at("stall_angle");
    if (value.valid)
        stallAngle = std::get<Scalar>(value.value);
    value = info.nodes.at("specs").attributes.at("max_angular_rate");
    if (value.valid)
        maxAngularRate = std::get<Scalar>(value.value);    
    value = info.nodes.at("specs").attributes.at("inverted");
    if (value.valid)
        inverted = std::get<bool>(value.value);

    // Required (visual)
    std::string meshFilename = std::get<std::string>(info.nodes.at("visual").childNodes.at("mesh").attributes.at("filename").value);
    Scalar meshScale = std::get<Scalar>(info.nodes.at("visual").childNodes.at("mesh").attributes.at("scale").value);
    std::string materialName = std::get<std::string>(info.nodes.at("visual").childNodes.at("material").attributes.at("name").value);
    std::string lookName = std::get<std::string>(info.nodes.at("visual").childNodes.at("look").attributes.at("name").value);
    Transform meshOrigin = std::get<Transform>(info.nodes.at("visual").childNodes.at("origin").attributes.at("T").value);

    // Construct
    PhysicsSettings phy;
    phy.mode = PhysicsMode::SUBMERGED;
    phy.collisions = false;
    phy.buoyancy = false;

    std::unique_ptr<Polyhedron> rudder = std::make_unique<Polyhedron>(uniqueName + "/Rudder", phy, GetFullPath(std::string(meshFilename)), 
        meshScale, meshOrigin, materialName, lookName);
        
    return std::make_unique<Rudder>(uniqueName, std::move(rudder), area, liftCoeff, dragCoeff, stallAngle, maxAngle, inverted, maxAngularRate);
}

REGISTER_ACTUATOR("rudder", Rudder)
    
}
