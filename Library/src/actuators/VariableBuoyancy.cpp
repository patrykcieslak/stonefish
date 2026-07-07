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
//  VariableBuoyancy.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 07/11/2019.
//  Copyright (c) 2019-2025 Patryk Cieslak. All rights reserved.
//

#include "actuators/VariableBuoyancy.h"

#include "core/SimulationApp.h"
#include "core/SimulationManager.h"
#include <algorithm>

namespace sf 
{
    
VariableBuoyancy::VariableBuoyancy(std::string uniqueName, const std::vector<std::string>& volumeMeshPaths, Scalar initialVolume) : LinkActuator(uniqueName)
{
    if(volumeMeshPaths.size() < 2)
        cCritical("VBS volume definition requires loading at least two meshes - for the full/empty states!");
    
    density_ = Scalar(1000.0);
    Ocean* ocn;
    if((ocn = SimulationApp::getApp()->getSimulationManager()->getOcean()) != nullptr)
        density_ = ocn->getLiquid().density;
    gravity_ = SimulationApp::getApp()->getSimulationManager()->getGravity();
    
    for(size_t i=0; i<volumeMeshPaths.size(); ++i)
    {
        Mesh* mesh = LoadGeometryFromFile(volumeMeshPaths[i], 1.f);
        if(mesh == NULL)
            abort();
        Vprops_.push_back(ComputePhysicalProperties(mesh, Scalar(0), density_));
        delete mesh;
    }
    auto volumeCompare = [](MeshProperties& mp1, MeshProperties& mp2) { return mp1.volume < mp2.volume; };
    std::sort(Vprops_.begin(), Vprops_.end(), volumeCompare);
    
    flowRate_ = Scalar(0);
    force_ = V0();
    Vmin_ = Vprops_.front().volume;
    Vmax_ = Vprops_.back().volume;
    
    cInfo("VBS created with Vmin: %1.3lf Vmax: %1.3lf", Vmin_, Vmax_);
    
    V_ = initialVolume > Vmax_ ? Vmax_ : (initialVolume < Vmin_ ? Vmin_ : initialVolume);
    
    Scalar mass;
    InterpolateVProps(V_, mass, CG_);
}    

ActuatorType VariableBuoyancy::getType() const
{
    return ActuatorType::VBS;
}
        
void VariableBuoyancy::setFlowRate(Scalar rate)
{
    flowRate_ = rate;
}
    
Scalar VariableBuoyancy::getFlowRate() const
{
    return flowRate_;
}
    
Scalar VariableBuoyancy::getLiquidVolume() const
{
    return V_;
}

Scalar VariableBuoyancy::getForce() const
{
    return force_.safeNorm();
}

void VariableBuoyancy::InterpolateVProps(Scalar volume, Scalar& m, Vector3& cg)
{
    if(volume <= Vmin_)
    {
        m = Vprops_.front().mass;
        cg = Vprops_.front().CG;
    }
    else if(volume >= Vmax_)
    {
        m = Vprops_.back().mass;
        cg = Vprops_.back().CG;
    }
    else 
    {
        m = volume*density_; 
        
        //Find interpolation pair
        size_t i;
        for(i=0; i<Vprops_.size(); ++i)
            if(Vprops_[i].volume > volume) 
                break;

        //Linear interpolation
        cg = Vprops_[i-1].CG + (Vprops_[i].CG - Vprops_[i-1].CG)/(Vprops_[i].volume - Vprops_[i-1].volume) * (volume - Vprops_[i-1].volume); 
    }
}
    
void VariableBuoyancy::Update(Scalar dt)
{
    Ocean* ocn = SimulationApp::getApp()->getSimulationManager()->getOcean();
    if(ocn != nullptr && attach_ != NULL)
    {
        //Update volume
        V_ += flowRate_*dt;
        V_ = V_ < Vmin_ ? Vmin_ : (V_ > Vmax_ ? Vmax_ : V_);
        Vector3 cg;
        Scalar m;
        InterpolateVProps(V_, m, CG_);
        
        //Compute forces
        force_ = m*gravity_;

        //Apply forces and torques
        Vector3 solidCG = attach_->getCGTransform().getOrigin();
        Vector3 vbsCG = attach_->getOTransform() * o2a_ * CG_;
        attach_->ApplyCentralForce(force_);
        attach_->ApplyTorque((vbsCG - solidCG).cross(force_));
    }
}

std::vector<Renderable> VariableBuoyancy::Render()
{
    Transform vbsTrans = Transform::getIdentity();
    if(attach_ != NULL)
        vbsTrans.setOrigin(attach_->getOTransform() * o2a_ * CG_);
    else
        LinkActuator::Render();
    
    //Add renderable
    std::vector<Renderable> items(0);
    Renderable item;
    item.type = RenderableType::ACTUATOR_LINES;
    item.model = glMatrixFromTransform(vbsTrans);
    item.data = std::make_shared<std::vector<glm::vec3>>();
    auto points = item.getDataAsPoints();
    points->push_back(glm::vec3(0,0,0));
    points->push_back(0.1f * glm::vec3((GLfloat)force_.x(), (GLfloat)force_.y(), (GLfloat)force_.z()));
    items.push_back(item);
    
    return items;
}
    
    
}
