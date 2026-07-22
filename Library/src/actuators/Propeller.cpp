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
//  Copyright (c) 2019-2026 Patryk Cieslak. All rights reserved.
//

#include "actuators/Propeller.h"

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

Propeller::Propeller(const std::string& uniqueName, std::unique_ptr<SolidEntity> propeller, Scalar diameter, 
    const std::pair<Scalar, Scalar>& thrustCoeff, Scalar torqueCoeff, Scalar maxRPM, bool rightHand, bool inverted) : LinkActuator(uniqueName),
    propeller_(std::move(propeller)), D_(diameter), kT0_(thrustCoeff), kQ0_(torqueCoeff), RH_(rightHand), inv_(inverted)
{
    propeller_->BuildGraphicalObject();

    // State
    theta_ = Scalar(0);
    omega_ = Scalar(0);
    thrust_ = Scalar(0);
    torque_ = Scalar(0);
    iError_ = Scalar(0);
    
    // Controller
    setpoint_ = Scalar(0);
    kp_ = Scalar(8.0);
    ki_ = Scalar(3.0);
    iLim_ = Scalar(2.0);
    omegaLim_ = maxRPM/Scalar(60) * Scalar(2) * M_PI; //In rad/s
}

LinkActuatorType Propeller::getLinkActuatorType() const
{
    return LinkActuatorType::PROPELLER;
}

void Propeller::setSetpoint(Scalar s)
{
    if (inv_) s *= Scalar(-1);
    setpoint_ = s < Scalar(-1) ? Scalar(-1) : (s > Scalar(1) ? Scalar(1) : s);
    ResetWatchdog();
}

Scalar Propeller::getSetpoint() const
{
    return inv_ ? -setpoint_ : setpoint_;
}

Scalar Propeller::getAngle() const
{
    return theta_;
}

Scalar Propeller::getOmega() const
{
    return omega_;
}

Scalar Propeller::getThrust() const
{
    return thrust_;
}

Scalar Propeller::getTorque() const
{
    return torque_;
}

void Propeller::Update(Scalar dt)
{
    Actuator::Update(dt);

    //Update thruster velocity
    Scalar error = setpoint_ * omegaLim_ - omega_;
    Scalar motorTorque = kp_ * error + ki_ * iError_;
    omega_ += (motorTorque - torque_)*dt;
    theta_ += omega_ * dt; //Just for animation
    
    //Integrate error
    iError_ += error * dt;
    iError_ = iError_ > iLim_ ? iLim_ : (iError_ < -iLim_ ? -iLim_ : iError_);
    
    if(attach_ != NULL)
    {
        //Get transforms
        Transform solidTrans = attach_->getCGTransform();
        Transform propTrans = attach_->getOTransform() * o2a_;
        Vector3 relPos = propTrans.getOrigin() - solidTrans.getOrigin();
        Vector3 velocity = attach_->getLinearVelocityInLocalPoint(relPos);
        
        Atmosphere* atm = SimulationApp::getApp()->getSimulationManager()->getAtmosphere();
        
        if(atm->IsInsideFluid(propTrans.getOrigin()))
        {
            //Get proper thrust coefficient
            bool backward = (RH_ && omega_ < Scalar(0)) || (!RH_ && omega_ > Scalar(0));
            Scalar kT = backward ? kT0_.second : kT0_.first;

            //Calculate thrust
            //Thrust coefficient depends on advance ratio J = u/(n*D), kT0 -> J=0
            //The lower the p/D the more linear the dependence of Kt on J
            Scalar n = omega_/(Scalar(2) * M_PI);
            Scalar u = -propTrans.getBasis().getColumn(0).dot(atm->GetFluidVelocity(propTrans.getOrigin()) - velocity); //Incoming air velocity
            Scalar alpha(-0.095/0.8);
            //kT(J) = kT0 + alpha * J --> approximated with linear function
            thrust_ = (RH_ ? Scalar(1) : Scalar(-1)) * atm->getGas().density * D_*D_*D_ * btFabs(n) * (D_*kT*n + alpha*u);
            torque_ = -kQ0_ * atm->getGas().density * btFabs(n)*n * D_*D_*D_*D_*D_; //Torque is the loading of propeller due to drag
            
            //Apply forces and torques
            Vector3 thrustV(thrust_, 0, 0);
            Vector3 torqueV(torque_, 0, 0); 
            attach_->ApplyCentralForce(propTrans.getBasis() * thrustV);
            attach_->ApplyTorque((propTrans.getOrigin() - solidTrans.getOrigin()).cross(propTrans.getBasis() * thrustV));
            attach_->ApplyTorque(propTrans.getBasis() * torqueV);
        }
    }
}

std::vector<Renderable> Propeller::Render()
{
    Transform propTrans = Transform::getIdentity();
    if(attach_ != nullptr)
        propTrans = attach_->getOTransform() * o2a_;
    else
        LinkActuator::Render();
    
    //Rotate propeller
    propTrans *= Transform(Quaternion(0, 0, theta_), Vector3(0,0,0));
    
    //Add renderable
    std::vector<Renderable> items(0);
    Renderable item;
    item.type = RenderableType::SOLID;
    item.materialName = propeller_->getMaterial().name;
    item.objectId = propeller_->getGraphicalObject();
    item.lookId = dm_ == DisplayMode::GRAPHICAL ? propeller_->getLook() : -1;
	item.model = glMatrixFromTransform(propTrans);
    items.push_back(item);
    
    item.type = RenderableType::ACTUATOR_LINES;
    item.data = std::make_shared<std::vector<glm::vec3>>();
    auto points = item.getDataAsPoints();
    points->push_back(glm::vec3(0,0,0));
    points->push_back(glm::vec3(0.1f*thrust_,0,0));
    items.push_back(item);
    
    return items;
}
    
void Propeller::WatchdogTimeout()
{
    setSetpoint(Scalar(0));
}

// Statics

ConstructInfo Propeller::getConstructInfo()
{
    ConstructInfo info;
    ConstructInfoNode node;
    
    // Specs
    node.optional = false;
    node.attributes.insert({"thrust_coeff", {ConstructInfoValueType::SCALAR, false}});
    node.attributes.insert({"torque_coeff", {ConstructInfoValueType::SCALAR, false}});
    node.attributes.insert({"max_rpm", {ConstructInfoValueType::SCALAR, false}});
    node.attributes.insert({"thrust_coeff_reverse", {ConstructInfoValueType::SCALAR, true}});
    node.attributes.insert({"inverted", {ConstructInfoValueType::BOOL, true}});
    info.nodes.insert({"specs", node});

    // Propeller mesh
    node.attributes.clear();
    node.optional = false;
    node.attributes.insert({"diameter", {ConstructInfoValueType::SCALAR, false}});
    node.attributes.insert({"right", {ConstructInfoValueType::BOOL, false}});

    ConstructInfoNode childNode;
    childNode.optional = false;
    childNode.attributes.insert({"filename", {ConstructInfoValueType::STRING, false}});
    childNode.attributes.insert({"scale", {ConstructInfoValueType::SCALAR, true}});
    node.childNodes.insert({"mesh", childNode});

    childNode.attributes.clear();
    childNode.optional = false;
    childNode.attributes.insert({"name", {ConstructInfoValueType::STRING, false}});
    node.childNodes.insert({"material", childNode});
    node.childNodes.insert({"look", childNode});

    info.nodes.insert({"propeller", node});

    return info;
}

std::unique_ptr<Propeller> Propeller::Construct(const std::string& uniqueName, ConstructInfo& info)
{
    // Required
    std::pair<Scalar, Scalar> thrustCoeff;
    thrustCoeff.first = thrustCoeff.second = std::get<Scalar>(info.nodes.at("specs").attributes.at("thrust_coeff").value);
    Scalar torqueCoeff = std::get<Scalar>(info.nodes.at("specs").attributes.at("torque_coeff").value);
    Scalar maxRpm = std::get<Scalar>(info.nodes.at("specs").attributes.at("max_rpm").value);

    // Optional
    bool inverted = false;
    ConstructInfoValue& value = info.nodes.at("specs").attributes.at("thrust_coeff_backward");
    if (value.valid)
        thrustCoeff.second = std::get<Scalar>(value.value);
    value = info.nodes.at("specs").attributes.at("inverted");
    if (value.valid)
        inverted = std::get<bool>(value.value);

    // Required (propeller mesh)
    Scalar diameter = std::get<Scalar>(info.nodes.at("propeller").attributes.at("diameter").value);
    bool right = std::get<bool>(info.nodes.at("propeller").attributes.at("right").value);
    std::string meshFilename = std::get<std::string>(info.nodes.at("propeller").childNodes.at("mesh").attributes.at("filename").value);
    value = info.nodes.at("propeller").childNodes.at("mesh").attributes.at("scale");
    Scalar meshScale(1.);
    if (value.valid)
        meshScale = std::get<Scalar>(value.value);
    std::string materialName = std::get<std::string>(info.nodes.at("propeller").childNodes.at("material").attributes.at("name").value);
    std::string lookName = std::get<std::string>(info.nodes.at("propeller").childNodes.at("look").attributes.at("name").value);

    // Construct
    PhysicsSettings phy;
    phy.collisions = false;
    phy.buoyancy = false;
    phy.mode = PhysicsMode::AERODYNAMIC;
    std::unique_ptr<Polyhedron> mesh = std::make_unique<Polyhedron>(uniqueName + "/Propeller", phy, GetFullPath(meshFilename), 
        meshScale, I4(), materialName, lookName, Scalar(-1.), GeometryApproxType::CYLINDER);
        
    return std::make_unique<Propeller>(uniqueName, std::move(mesh), diameter, thrustCoeff, torqueCoeff, maxRpm, right, inverted);
}

REGISTER_ACTUATOR("propeller", Propeller)

}
