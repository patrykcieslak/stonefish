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
//  Copyright (c) 2019-2023 Patryk Cieslak. All rights reserved.
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
    
    density = Scalar(1000.0);
    Ocean* ocn;
    if((ocn = SimulationApp::getApp()->getSimulationManager()->getOcean()) != nullptr)
        density = ocn->getLiquid().density;
    gravity = SimulationApp::getApp()->getSimulationManager()->getGravity();
    
    for(size_t i=0; i<volumeMeshPaths.size(); ++i)
    {
        Mesh* mesh = LoadGeometryFromFile(volumeMeshPaths[i], 1.f);
        if(mesh == NULL)
            abort();
        Vprops.push_back(ComputePhysicalProperties(mesh, Scalar(0), density));
        delete mesh;
    }
    auto volumeCompare = [](MeshProperties& mp1, MeshProperties& mp2) { return mp1.volume < mp2.volume; };
    std::sort(Vprops.begin(), Vprops.end(), volumeCompare);
    
    flowRate = Scalar(0);
    force = V0();
    Vmin = Vprops.front().volume;
    Vmax = Vprops.back().volume;
    
    cInfo("VBS created with Vmin: %1.3lf Vmax: %1.3lf", Vmin, Vmax);
    
    V = initialVolume > Vmax ? Vmax : (initialVolume < Vmin ? Vmin : initialVolume);
    
    Scalar mass;
    InterpolateVProps(V, mass, CG);
}    

ActuatorType VariableBuoyancy::getType() const
{
    return ActuatorType::VBS;
}
        
void VariableBuoyancy::setFlowRate(Scalar rate)
{
    flowRate = rate;
}
    
Scalar VariableBuoyancy::getFlowRate() const
{
    return flowRate;
}
    
Scalar VariableBuoyancy::getLiquidVolume() const
{
    return V;
}

Scalar VariableBuoyancy::getForce() const
{
    return force.safeNorm();
}

void VariableBuoyancy::InterpolateVProps(Scalar volume, Scalar& m, Vector3& cg)
{
    if(volume <= Vmin)
    {
        m = Vprops.front().mass;
        cg = Vprops.front().CG;
    }
    else if(volume >= Vmax)
    {
        m = Vprops.back().mass;
        cg = Vprops.back().CG;
    }
    else 
    {
        m = volume*density; 
        
        //Find interpolation pair
        size_t i;
        for(i=0; i<Vprops.size(); ++i)
            if(Vprops[i].volume > volume) 
                break;

        //Linear interpolation
        cg = Vprops[i-1].CG + (Vprops[i].CG - Vprops[i-1].CG)/(Vprops[i].volume - Vprops[i-1].volume) * (volume - Vprops[i-1].volume); 
    }
}
    
void VariableBuoyancy::Update(Scalar dt)
{
    Ocean* ocn = SimulationApp::getApp()->getSimulationManager()->getOcean();
    if(ocn != nullptr && attach != NULL)
    {
        //Update volume
        V += flowRate*dt;
        V = V < Vmin ? Vmin : (V > Vmax ? Vmax : V);
        Vector3 cg;
        Scalar m;
        InterpolateVProps(V, m, CG);
        
        //Compute forces
        force = m*gravity;

        //Apply forces and torques
        Vector3 solidCG = attach->getCGTransform().getOrigin();
        Vector3 vbsCG = attach->getOTransform() * o2a * CG;
        attach->ApplyCentralForce(force);
        attach->ApplyTorque((vbsCG - solidCG).cross(force));
    }
}

std::vector<Renderable> VariableBuoyancy::Render()
{
    Transform vbsTrans = Transform::getIdentity();
    if(attach != NULL)
        vbsTrans.setOrigin(attach->getOTransform() * o2a * CG);
    else
        LinkActuator::Render();
    
    //Add renderable
    std::vector<Renderable> items(0);
    Renderable item;
    item.type = RenderableType::ACTUATOR_LINES;
    item.model = glMatrixFromTransform(vbsTrans);
    item.points.push_back(glm::vec3(0,0,0));
    item.points.push_back(0.1f * glm::vec3((GLfloat)force.x(), (GLfloat)force.y(), (GLfloat)force.z()));
    items.push_back(item);
    
    return items;
}
    
    
}
