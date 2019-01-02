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
#include "graphics/OpenGLOcean.h"

namespace sf
{

Ocean::Ocean(std::string uniqueName, bool simulateWaves, Fluid* l) : ForcefieldEntity(uniqueName)
{
	ghost->setCollisionFlags(ghost->getCollisionFlags() | btCollisionObject::CF_STATIC_OBJECT);
    
	Scalar size(10000);
	depth = size;
    Vector3 halfExtents = Vector3(size/Scalar(2), size/Scalar(2), size/Scalar(2));
    ghost->setWorldTransform(Transform(Quaternion::getIdentity(), Vector3(0,0,size/Scalar(2)))); //Surface at 0
    ghost->setCollisionShape(new btBoxShape(halfExtents));
    
    currents = std::vector<VelocityField*>(0);
	
	glOcean = NULL;
    liquid = l;
    waves = simulateWaves;
    wavesDebug.type = RenderableType::HYDRO_POINTS;
    wavesDebug.model = glm::mat4(1.f);
    waterType = Scalar(0.5);
    turbidity = 1.0;
}

Ocean::~Ocean()
{
    if(currents.size() > 0)
    {
        for(unsigned int i=0; i<currents.size(); ++i)
            delete currents[i];
        currents.clear();
    }
    
    liquid = NULL;
	
	if(glOcean != NULL)
		delete glOcean;
}

bool Ocean::hasWaves()
{
    return waves;
}
    
void Ocean::setWaterType(Scalar t)
{
    waterType = t > Scalar(1) ? Scalar(1) : (t < Scalar(0) ? Scalar(0) : t);
    
    if(glOcean != NULL)
        glOcean->setWaterType((float)waterType);
}

Scalar Ocean::getWaterType()
{
    return waterType;
}

void Ocean::setTurbidity(Scalar t)
{
    turbidity = t < Scalar(0) ? Scalar(0) : t;
    
    if(glOcean != NULL)
        glOcean->setTurbidity((float)turbidity);
}
    
Scalar Ocean::getTurbidity()
{
    return turbidity;
}
    
OpenGLOcean* Ocean::getOpenGLOcean()
{
    return glOcean;
}

ForcefieldType Ocean::getForcefieldType()
{
    return FORCEFIELD_OCEAN;
}

const Fluid* Ocean::getLiquid() const
{
    return liquid;
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
    if(waves) //Geometric waves
    {
        GLfloat waveHeight = glOcean->getWaveHeight((GLfloat)point.x(), (GLfloat)point.y());
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
    Scalar pressure = d > Scalar(0) ? d*liquid->density*g : Scalar(0);
    return pressure;
}

Vector3 Ocean::GetFluidVelocity(const Vector3& point) const
{
    Vector3 fv(0,0,0);
    for(unsigned int i=0; i<currents.size(); ++i)
        fv += currents[i]->GetVelocityAtPoint(point);
    return fv;
}

void Ocean::GetSurface(Vector3& normal, Vector3& position) const
{
    normal = -ghost->getWorldTransform().getBasis().getColumn(2).normalized();
    position = ghost->getWorldTransform().getOrigin()+normal*(depth/2.0);
}

void Ocean::GetSurfaceEquation(double* plane4) const
{
    Vector3 normal = -ghost->getWorldTransform().getBasis().getColumn(2).normalized();
    Vector3 position = ghost->getWorldTransform().getOrigin()+normal*(depth/2.0);
    plane4[0] = normal.x();
    plane4[1] = normal.y();
    plane4[2] = normal.z();
    plane4[3] = -normal.dot(position);
}

void Ocean::ApplyFluidForces(const HydrodynamicsType ht, btDynamicsWorld* world, btCollisionObject* co, bool recompute)
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
	settings.algorithm = ht;
	
    if(ent->getType() == ENTITY_SOLID)
    {
        if(recompute)
        {
            settings.addedMassForces = true;
            settings.dampingForces = true;
            settings.reallisticBuoyancy = true;
            ((SolidEntity*)ent)->ComputeFluidForces(settings, this);
        }
        else
        {
            settings.addedMassForces = true;
            settings.dampingForces = false;
            settings.reallisticBuoyancy = false;
            ((SolidEntity*)ent)->ComputeFluidForces(settings, this);
        }
        
        ((SolidEntity*)ent)->ApplyFluidForces();
    }
}

void Ocean::InitGraphics(SDL_mutex* hydrodynamics)
{
	glOcean = new OpenGLOcean(waves, hydrodynamics);
    setWaterType(Scalar(0.5));
}

std::vector<Renderable> Ocean::Render()
{
    std::vector<Renderable> items(0);
    
    for(unsigned int i=0; i<currents.size(); ++i)
    {
        std::vector<Renderable> citems = currents[i]->Render();
        items.insert(items.end(), citems.begin(), citems.end());
    }
    
    if(wavesDebug.points.size() > 0)
    {
        items.push_back(wavesDebug);
        wavesDebug.points.clear();
    }
    
    return items;
}

}
