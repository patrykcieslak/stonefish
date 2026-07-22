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
//  Created by Patryk Cieslak on 07/04/2024.
//  Copyright (c) 2024-2026 Patryk Cieslak. All rights reserved.
//

#include "actuators/SimpleThruster.h"

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

SimpleThruster::SimpleThruster(const std::string& uniqueName, std::unique_ptr<SolidEntity> propeller, bool rightHand, bool inverted) 
    : LinkActuator(uniqueName), propeller_(std::move(propeller)), RH_(rightHand), inv_(inverted)
{
    propeller_->BuildGraphicalObject();

    theta_ = Scalar(0);
    thrust_ = Scalar(0);
    torque_ = Scalar(0);
    setThrustLimits(BT_LARGE_FLOAT, BT_LARGE_FLOAT); // No limits
    setSetpoint(Scalar(0.), Scalar(0.));
}

LinkActuatorType SimpleThruster::getLinkActuatorType() const
{
    return LinkActuatorType::SIMPLE_THRUSTER;
}

void SimpleThruster::setSetpoint(Scalar thrust, Scalar torque)
{
    sThrust_ = btClamped(thrust, -limits_.second, limits_.first);   
    sTorque_ = torque;

    if(inv_)
    { 
        sThrust_ *= Scalar(-1);
        sTorque_ *= Scalar(-1);
    }

    ResetWatchdog();
}

void SimpleThruster::setThrustLimits(Scalar positive, Scalar negative)
{
    limits_.first = btClamped(positive, Scalar(0), Scalar(BT_LARGE_FLOAT));
    limits_.second = btClamped(btFabs(negative), Scalar(0), Scalar(BT_LARGE_FLOAT));
}

Scalar SimpleThruster::getAngle() const
{
    return theta_;
}

Scalar SimpleThruster::getThrustSetpoint() const
{
    return sThrust_;
}

Scalar SimpleThruster::getThrust() const
{
    return thrust_;
}

Scalar SimpleThruster::getTorque() const
{
    return torque_;
}

void SimpleThruster::Update(Scalar dt)
{
    Actuator::Update(dt);

    if(attach_ != nullptr)
    {
        //Update thruster rotation (visualization only)
        Scalar omega(0);
        if(!btFuzzyZero(sThrust_))
        {
            omega = sThrust_ > Scalar(0) ? Scalar(2.0*M_PI) : Scalar(-2.0*M_PI);
            omega = RH_ ? omega : -omega; 
        }
        theta_ += omega * dt; //Just for animation
    
        //Get transforms
        Transform solidTrans = attach_->getCGTransform();
        Transform thrustTrans = attach_->getOTransform() * o2a_;
        
        //Calculate thrust
        Ocean* ocn = SimulationApp::getApp()->getSimulationManager()->getOcean();
        if(ocn != nullptr && ocn->IsInsideFluid(thrustTrans.getOrigin()))
        {
            thrust_ = sThrust_;
            torque_ = sTorque_;

            //Apply forces and torques
            Vector3 thrustV(thrust_, 0, 0);
            Vector3 torqueV(torque_, 0, 0);
            attach_->ApplyCentralForce(thrustTrans.getBasis() * thrustV);
            attach_->ApplyTorque((thrustTrans.getOrigin() - solidTrans.getOrigin()).cross(thrustTrans.getBasis() * thrustV));
            attach_->ApplyTorque(thrustTrans.getBasis() * torqueV);
        }
        else
        {
            thrust_ = Scalar(0);
            torque_ = Scalar(0);
        }
    }
}

std::vector<Renderable> SimpleThruster::Render()
{
    Transform thrustTrans = Transform::getIdentity();
    if(attach_ != nullptr)
        thrustTrans = attach_->getOTransform() * o2a_;
    else
        LinkActuator::Render();
    
    //Rotate propeller
    thrustTrans *= Transform(Quaternion(0, 0, theta_), Vector3(0,0,0));
    
    //Add renderable
    std::vector<Renderable> items(0);
    Renderable item;
    item.type = RenderableType::SOLID;
    item.materialName = propeller_->getMaterial().name;
    item.objectId = propeller_->getGraphicalObject();
    item.lookId = dm_ == DisplayMode::GRAPHICAL ? propeller_->getLook() : -1;
	item.model = glMatrixFromTransform(thrustTrans);
    items.push_back(item);
    
    item.type = RenderableType::ACTUATOR_LINES;
    item.data = std::make_shared<std::vector<glm::vec3>>();
    auto points = item.getDataAsPoints();
    points->push_back(glm::vec3(0,0,0));
    points->push_back(glm::vec3(0.1f*thrust_,0,0));
    items.push_back(item);
    
    return items;
}

void SimpleThruster::WatchdogTimeout()
{
    setSetpoint(Scalar(0), Scalar(0));
}

// Statics

ConstructInfo SimpleThruster::getConstructInfo()
{
    ConstructInfo info;
    ConstructInfoValue value;
    ConstructInfoNode node;
    
    // Specs
    value.optional = true;
    node.optional = true;
    value.valueType = ConstructInfoValueType::BOOL;
    node.attributes.insert({"inverted", value});
    info.nodes.insert({"specs", node});

    // Limits
    node.attributes.clear();
    value.valueType = ConstructInfoValueType::SCALAR;
    node.attributes.insert({"max_positive_thrust", value});
    node.attributes.insert({"max_negative_thrust", value});
    info.nodes.insert({"limits", node});

    // Propeller
    node.attributes.clear();
    value.optional = false;
    node.optional = false;
    value.valueType = ConstructInfoValueType::BOOL;
    node.attributes.insert({"right", value});

    ConstructInfoNode childNode;
    childNode.optional = false;
    value.optional = false;
    value.valueType = ConstructInfoValueType::STRING;
    childNode.attributes.insert({"filename", value});
    value.optional = true;
    value.valueType = ConstructInfoValueType::SCALAR;
    childNode.attributes.insert({"scale", value});
    node.childNodes.insert({"mesh", childNode});

    childNode.attributes.clear();
    childNode.optional = false;
    value.optional = false;
    value.valueType = ConstructInfoValueType::STRING;
    childNode.attributes.insert({"name", value});
    node.childNodes.insert({"material", childNode});
    node.childNodes.insert({"look", childNode});

    info.nodes.insert({"propeller", node});

    return info;
}

std::unique_ptr<SimpleThruster> SimpleThruster::Construct(const std::string& uniqueName, ConstructInfo& info)
{
    // Optional
    bool inverted = false;
    ConstructInfoValue& value = info.nodes.at("specs").attributes.at("inverted");
    if (value.valid)
        inverted = std::get<bool>(value.value);
    Scalar maxPositiveThrust (BT_LARGE_FLOAT);
    Scalar maxNegativeThrust (BT_LARGE_FLOAT);
    value = info.nodes.at("limits").attributes.at("max_positive_thrust");
    if (value.valid)
        maxPositiveThrust = std::get<Scalar>(value.value);
    value = info.nodes.at("limits").attributes.at("max_negative_thrust");
    if (value.valid)
        maxNegativeThrust = std::get<Scalar>(value.value);
    
    // Required (propeller mesh)
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
    phy.mode = PhysicsMode::SUBMERGED;
    std::unique_ptr<Polyhedron> mesh = std::make_unique<Polyhedron>(uniqueName + "/Propeller", phy, GetFullPath(meshFilename), 
        meshScale, I4(), materialName, lookName);
        
    std::unique_ptr<SimpleThruster> actuator = std::make_unique<SimpleThruster>(uniqueName, std::move(mesh), right, inverted);
    actuator->setThrustLimits(maxPositiveThrust, maxNegativeThrust);

    return actuator;
}

REGISTER_ACTUATOR("simple_thruster", SimpleThruster)
    
}
