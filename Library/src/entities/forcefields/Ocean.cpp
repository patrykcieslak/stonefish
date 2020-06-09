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
//  Ocean.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 19/10/17.
//  Copyright(c) 2017-2018 Patryk Cieslak. All rights reserved.
//

#include "entities/forcefields/Ocean.h"

#include <algorithm>
#include "utils/SystemUtil.hpp"
#include "entities/forcefields/VelocityField.h"
#include "entities/SolidEntity.h"
#include "graphics/OpenGLFlatOcean.h"
#include "graphics/OpenGLRealOcean.h"
#include "actuators/Thruster.h"

namespace sf
{

Ocean::Ocean(std::string uniqueName, Scalar waves, Fluid l) : ForcefieldEntity(uniqueName)
{
    ghost->setCollisionFlags(ghost->getCollisionFlags() | btCollisionObject::CF_STATIC_OBJECT);
    oceanState = waves > Scalar(2.0) ? Scalar(2.0) : waves;
     
    Scalar size(100000);
    depth = size;
    Vector3 halfExtents = Vector3(size/Scalar(2), size/Scalar(2), size/Scalar(2));
    ghost->setWorldTransform(Transform(Quaternion::getIdentity(), Vector3(0, 0, size/Scalar(2) - oceanState*Scalar(3)))); //Move ocean influence zone a bit up to account for waves
    ghost->setCollisionShape(new btBoxShape(halfExtents));
    
    currents = std::vector<VelocityField*>(0);
    currentsEnabled = false;
    
    liquid = l;
    wavesDebug.type = RenderableType::HYDRO_POINTS;
    wavesDebug.model = glm::mat4(1.f);
    waterType = Scalar(0.0);
    glOcean = NULL;
}

Ocean::~Ocean()
{
    if(currents.size() > 0)
    {
        for(unsigned int i=0; i<currents.size(); ++i)
            delete currents[i];
        currents.clear();
    }
    
    if(glOcean != NULL)
        delete glOcean;
}

bool Ocean::hasWaves() const
{
    return oceanState > Scalar(0);
}

Scalar Ocean::getWaterType()
{
    return waterType;
}
        
OpenGLOcean* Ocean::getOpenGLOcean()
{
    return glOcean;
}

ForcefieldType Ocean::getForcefieldType()
{
    return FORCEFIELD_OCEAN;
}

Fluid Ocean::getLiquid() const
{
    return liquid;
}
    
void Ocean::SetupWaterProperties(Scalar jerlov)
{ 
    if(glOcean != NULL)
    {
        waterType = jerlov > Scalar(1) ? Scalar(1) : (jerlov < Scalar(0) ? Scalar(0) : jerlov);
        glOcean->setWaterType((float)waterType);
    }
}

void Ocean::AddVelocityField(VelocityField* field)
{
    currents.push_back(field);
}

bool Ocean::IsInsideFluid(const Vector3& point)
{
    return GetDepth(point) >= Scalar(0);
}

Scalar Ocean::GetDepth(const Vector3& point)
{
    if(hasWaves()) //Geometric waves
    {
        GLfloat waveHeight = glOcean->ComputeWaveHeight((GLfloat)point.x(), (GLfloat)point.y());
        glm::vec3 wavePoint((GLfloat)point.x(), (GLfloat)point.y(), waveHeight);
        wavesDebug.points.push_back(wavePoint);
        return point.z() - Scalar(waveHeight);
    }
    else //Flat surface
    {
        glm::vec3 wavePoint((GLfloat)point.x(), (GLfloat)point.y(), 0.f);
        wavesDebug.points.push_back(wavePoint);
        return point.z();
    }
}

Scalar Ocean::GetPressure(const Vector3& point)
{
    Scalar g = 9.81;
    Scalar d = GetDepth(point);
    Scalar pressure = d > Scalar(0) ? d*liquid.density*g : Scalar(0);
    return pressure;
}

Vector3 Ocean::GetFluidVelocity(const Vector3& point) const
{
    if(currentsEnabled)
    {
        Vector3 fv = V0();
        for(size_t i=0; i<currents.size(); ++i)
            fv += currents[i]->GetVelocityAtPoint(point);
        return fv;
    }
    else
        return V0();
}

void Ocean::EnableCurrents()
{
    currentsEnabled = true;
}

void Ocean::DisableCurrents()
{
    currentsEnabled = false;
}

void Ocean::UpdateCurrentsData()
{
    if(glOcean != NULL)
        glOcean->UpdateOceanCurrentsData(glOceanCurrentsUBOData);
}

void Ocean::ApplyFluidForces(const FluidDynamicsType fdt, btDynamicsWorld* world, btCollisionObject* co, bool recompute)
{
    Entity* ent;
    btRigidBody* rb = btRigidBody::upcast(co);
    btMultiBodyLinkCollider* mbl = btMultiBodyLinkCollider::upcast(co);
    
    if(rb != 0)
    {
        if(rb->isStaticOrKinematicObject())
            return;
        else
            ent = (Entity*)rb->getUserPointer();
    }
    else if(mbl != 0)
    {
        if(mbl->isStaticOrKinematicObject())
            return;
        else
            ent = (Entity*)mbl->getUserPointer();
    }
    else
        return;
    
    HydrodynamicsSettings settings;
    settings.algorithm = fdt;
    
    if(ent->getType() == ENTITY_SOLID)
    {
        if(recompute)
        {
            settings.dampingForces = true;
            settings.reallisticBuoyancy = true;
            ((SolidEntity*)ent)->ComputeHydrodynamicForces(settings, this);
        }
        
        ((SolidEntity*)ent)->ApplyHydrodynamicForces();
    }
}

void Ocean::InitGraphics(SDL_mutex* hydrodynamics)
{
    if(oceanState > 0.0)
        glOcean = new OpenGLRealOcean(depth, oceanState, hydrodynamics);
    else
        glOcean = new OpenGLFlatOcean(depth);
    SetupWaterProperties(0.2);
}

std::vector<Renderable> Ocean::Render()
{
    std::vector<Actuator*> act;
    return Render(act);
}

std::vector<Renderable> Ocean::Render(const std::vector<Actuator*>& act)
{
    std::vector<Renderable> items(0);
    
    //Update currents data
    glOceanCurrentsUBOData.gravity = glm::vec3(0.f,0.f,9.81f);
    glOceanCurrentsUBOData.numCurrents = 0;

    if(currentsEnabled)
    {
        for(size_t i=0; i<currents.size(); ++i)
        {
            std::vector<Renderable> citems = currents[i]->Render(glOceanCurrentsUBOData.currents[glOceanCurrentsUBOData.numCurrents]);
            items.insert(items.end(), citems.begin(), citems.end());
            ++glOceanCurrentsUBOData.numCurrents;
        }
    }
    
    for(size_t i=0; i<act.size(); ++i)
    {
        if(act[i]->getType() == ActuatorType::ACTUATOR_THRUSTER)
        {
            Thruster* th = (Thruster*)act[i];
            Transform thFrame = th->getActuatorFrame();
            Vector3 thPos = thFrame.getOrigin();
            Vector3 thDir = -thFrame.getBasis().getColumn(0);
            Scalar R = th->getDiameter()/Scalar(2);
            Scalar vel = th->getOmega()/Scalar(10);
            glOceanCurrentsUBOData.currents[glOceanCurrentsUBOData.numCurrents].posR = glm::vec4((GLfloat)thPos.getX(), 
                                                                             (GLfloat)thPos.getY(), 
                                                                             (GLfloat)thPos.getZ(), (GLfloat)R);
            glOceanCurrentsUBOData.currents[glOceanCurrentsUBOData.numCurrents].dirV = glm::vec4((GLfloat)thDir.getX(),
                                                                             (GLfloat)thDir.getY(),
                                                                             (GLfloat)thDir.getZ(),
                                                                             (GLfloat)vel);
            glOceanCurrentsUBOData.currents[glOceanCurrentsUBOData.numCurrents].params = glm::vec3(0.f);
            glOceanCurrentsUBOData.currents[glOceanCurrentsUBOData.numCurrents].type = 10;
            ++glOceanCurrentsUBOData.numCurrents;
        }
    }

    if(wavesDebug.points.size() > 0)
    {
        items.push_back(wavesDebug);
        wavesDebug.points.clear();
    }

    return items;
}

}
