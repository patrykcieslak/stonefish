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
//  Copyright(c) 2017-2025 Patryk Cieslak. All rights reserved.
//

#include "entities/forcefields/Ocean.h"

#include <algorithm>
#include "utils/SystemUtil.hpp"
#include "entities/forcefields/VelocityField.h"
#include "entities/SolidEntity.h"
#include "entities/CableEntity.h"
#include "graphics/OpenGLFlatOcean.h"
#include "graphics/OpenGLRealOcean.h"
#include "actuators/Thruster.h"
#include "core/SimulationApp.h"
#include "core/SimulationManager.h"

namespace sf
{

Ocean::Ocean(std::string uniqueName, Scalar waves, Fluid l) : ForcefieldEntity(uniqueName)
{
    ghost_->setCollisionFlags(ghost_->getCollisionFlags() | btCollisionObject::CF_STATIC_OBJECT);
    oceanState_ = waves > Scalar(2.0) ? Scalar(2.0) : waves;
     
    Scalar size(100000);
    depth_ = size;
    Vector3 halfExtents = Vector3(size/Scalar(2), size/Scalar(2), size/Scalar(2));
    ghost_->setWorldTransform(Transform(Quaternion::getIdentity(), Vector3(0, 0, size/Scalar(2) - oceanState_*Scalar(3)))); //Move ocean influence zone a bit up to account for waves
    ghost_->setCollisionShape(new btBoxShape(halfExtents));
    
    currents_ = std::vector<VelocityField*>(0);
    currentsEnabled_ = false;
    
    liquid_ = l;
    wavesDebug_.type = RenderableType::HYDRO_POINTS;
    wavesDebug_.model = glm::mat4(1.f);
    wavesDebug_.data = std::make_shared<std::vector<glm::vec3>>();
    waterType_ = Scalar(0.0);
    glOcean_ = nullptr;
}

Ocean::~Ocean()
{
    if(currents_.size() > 0)
    {
        for(unsigned int i=0; i<currents_.size(); ++i)
            delete currents_[i];
        currents_.clear();
    }
    
    if(glOcean_ != nullptr)
        delete glOcean_;
}

bool Ocean::hasWaves() const
{
    return oceanState_ > Scalar(0);
}

bool Ocean::hasParticles() const
{
    if(glOcean_ != nullptr)
        return glOcean_->getParticlesEnabled();
    else
        return false;
}

Scalar Ocean::getWaterType() const
{
    return waterType_;
}
        
OpenGLOcean* Ocean::getOpenGLOcean()
{
    return glOcean_;
}

ForcefieldType Ocean::getForcefieldType()
{
    return ForcefieldType::OCEAN;
}

Fluid Ocean::getLiquid() const
{
    return liquid_;
}

VelocityField* Ocean::getVelocityField(size_t index)
{
    if(index < currents_.size())
        return currents_[index];
    else
        return nullptr;
}

void Ocean::setWaterType(Scalar jerlov)
{ 
    if(glOcean_ != nullptr)
    {
        waterType_ = jerlov > Scalar(1) ? Scalar(1) : (jerlov < Scalar(0) ? Scalar(0) : jerlov);
        glOcean_->setWaterType((float)waterType_);
    }
}

void Ocean::setParticles(bool enabled)
{
    if(glOcean_ != nullptr)
        glOcean_->setParticles(enabled);
}

void Ocean::SetConditions(Scalar waterTemp)
{
    if(glOcean_ != nullptr)
        glOcean_->setWaterTemperature((float)waterTemp);
}

void Ocean::AddVelocityField(VelocityField* field)
{
    currents_.push_back(field);
}

bool Ocean::IsInsideFluid(const Vector3& point)
{
    return GetDepth(point) >= Scalar(0);
}

float Ocean::GetDepth(const glm::vec3& point)
{
    if(hasWaves()) //Geometric waves
    {
        GLfloat waveHeight = glOcean_->ComputeWaveHeight(point.x, point.y);
        glm::vec3 wavePoint(point.x, point.y, waveHeight);
#ifdef DEBUG_WAVES
        wavesDebug.getDataAsPoints()->push_back(wavePoint);
#endif
        return point.z - waveHeight;
    }
    else //Flat surface
    {
        glm::vec3 wavePoint(point.x, point.y, 0.f);
#ifdef DEBUG_WAVES  
        wavesDebug.getDataAsPoints()->push_back(wavePoint);
#endif
        return point.z;
    }
}

Scalar Ocean::GetDepth(const Vector3& point)
{
    return Scalar(GetDepth(glm::vec3((GLfloat)point.getX(), (GLfloat)point.getY(), (GLfloat)point.getZ())));
}

Scalar Ocean::GetPressure(const Vector3& point)
{
    Scalar g = SimulationApp::getApp()->getSimulationManager()->getGravity().getZ();
    Scalar d = GetDepth(point);
    Scalar pressure = d > Scalar(0) ? d*liquid_.density*g : Scalar(0);
    return pressure;
}

Vector3 Ocean::GetFluidVelocity(const Vector3& point) const
{
    if(currentsEnabled_)
    {
        Vector3 fv = V0();
        for(size_t i=0; i<currents_.size(); ++i)
        {
            if(currents_[i]->isEnabled())
                fv += currents_[i]->GetVelocityAtPoint(point);
        }
        return fv;
    }
    return V0();
}

glm::vec3 Ocean::GetFluidVelocity(const glm::vec3& point) const
{
    return glVectorFromVector(GetFluidVelocity(Vector3(point.x, point.y, point.z)));
}

void Ocean::EnableCurrents()
{
    currentsEnabled_ = true;
}

void Ocean::DisableCurrents()
{
    currentsEnabled_ = false;
}

void Ocean::UpdateCurrentsData()
{
    if(glOcean_ != NULL)
        glOcean_->UpdateOceanCurrentsData(glOceanCurrentsUBOData_);
}

void Ocean::ApplyFluidForces(btDynamicsWorld* world, btCollisionObject* co, bool recompute)
{
    Entity* ent;
    
    if (btRigidBody* rb = dynamic_cast<btRigidBody*>(co))
    {
        if (rb->isStaticOrKinematicObject())
            return;
        else
            ent = static_cast<Entity*>(rb->getUserPointer());
    }
    else if (btMultiBodyLinkCollider* mbl = dynamic_cast<btMultiBodyLinkCollider*>(co))
    {
        if (mbl->isStaticOrKinematicObject())
            return;
        else
            ent = static_cast<Entity*>(mbl->getUserPointer());
    }
    else if (btSoftBody* sb = dynamic_cast<btSoftBody*>(co))
    {
        ent = static_cast<Entity*>(sb->getUserPointer());
    }
    else
        return;
      
    HydrodynamicsSettings settings;
    
    if (ent->getType() == EntityType::SOLID)
    {
        if(recompute)
        {
            settings.dampingForces = true;
            settings.reallisticBuoyancy = true;
            ((SolidEntity*)ent)->ComputeHydrodynamicForces(settings, this);
        }
        
        ((SolidEntity*)ent)->ApplyHydrodynamicForces();
    }
    else if (ent->getType() == EntityType::CABLE)
    {
        if(recompute)
        {
            settings.dampingForces = true;
            settings.reallisticBuoyancy = true;
            ((CableEntity*)ent)->ComputeHydrodynamicForces(settings, this);
        }
        
        ((CableEntity*)ent)->ApplyHydrodynamicForces();
    }
}

void Ocean::InitGraphics(SDL_mutex* hydrodynamics)
{
    if(oceanState_ > 0.0)
        glOcean_ = new OpenGLRealOcean(depth_, oceanState_, hydrodynamics);
    else
        glOcean_ = new OpenGLFlatOcean(depth_);
    setWaterType(0.2);
}

std::vector<Renderable> Ocean::Render()
{
    std::vector<std::unique_ptr<Actuator>> act;
    return Render(act);
}

std::vector<Renderable> Ocean::Render(const std::vector<std::unique_ptr<Actuator>>& act)
{
    std::vector<Renderable> items(0);
    
    //Update currents data
    glOceanCurrentsUBOData_.gravity = glm::vec3(0.f,0.f,9.81f);
    glOceanCurrentsUBOData_.numCurrents = 0;

    if(currentsEnabled_)
    {
        for(size_t i=0; i<currents_.size(); ++i)
            if(currents_[i]->isEnabled())
            {
                std::vector<Renderable> citems = currents_[i]->Render(glOceanCurrentsUBOData_.currents[glOceanCurrentsUBOData_.numCurrents]);
                items.insert(items.end(), citems.begin(), citems.end());
                ++glOceanCurrentsUBOData_.numCurrents;
            }
    }
    
    for(size_t i=0; i<act.size(); ++i)
        if(act[i]->getType() == ActuatorType::THRUSTER)
        {
            Thruster* th = static_cast<Thruster*>(act[i].get());
            Transform thFrame = th->getActuatorFrame();
            Vector3 thPos = thFrame.getOrigin();
            Vector3 thDir = -thFrame.getBasis().getColumn(0);
            Scalar R = th->getPropellerDiameter()/Scalar(2);
            Scalar vel = (th->isPropellerRight() ? Scalar(0.1) : Scalar(-0.1)) * th->getThrust();
            glOceanCurrentsUBOData_.currents[glOceanCurrentsUBOData_.numCurrents].posR = glm::vec4((GLfloat)thPos.getX(), 
                                                                             (GLfloat)thPos.getY(), 
                                                                             (GLfloat)thPos.getZ(), (GLfloat)R);
            glOceanCurrentsUBOData_.currents[glOceanCurrentsUBOData_.numCurrents].dirV = glm::vec4((GLfloat)thDir.getX(),
                                                                             (GLfloat)thDir.getY(),
                                                                             (GLfloat)thDir.getZ(),
                                                                             (GLfloat)vel);
            glOceanCurrentsUBOData_.currents[glOceanCurrentsUBOData_.numCurrents].params = glm::vec3(0.f);
            glOceanCurrentsUBOData_.currents[glOceanCurrentsUBOData_.numCurrents].type = 10;
            ++glOceanCurrentsUBOData_.numCurrents;
        }

    if(wavesDebug_.getDataAsPoints()->size() > 0)
    {
        items.push_back(wavesDebug_);
        wavesDebug_.getDataAsPoints()->clear();
    }

    return items;
}

}
