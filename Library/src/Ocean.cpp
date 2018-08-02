//
//  Ocean.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 19/10/17.
//  Copyright(c) 2017 Patryk Cieslak. All rights reserved.
//

#include "Ocean.h"
#include "SolidEntity.h"
#include "SystemEntity.h"
#include "MathsUtil.hpp"
#include "SystemUtil.hpp"
#include <algorithm>

Ocean::Ocean(std::string uniqueName, Liquid* l) : ForcefieldEntity(uniqueName)
{
	ghost->setCollisionFlags(ghost->getCollisionFlags() | btCollisionObject::CF_STATIC_OBJECT);
    
	btScalar size(10000);
	depth = UnitSystem::SetLength(size);
    btVector3 halfExtents = btVector3(UnitSystem::SetLength(size/btScalar(2)), UnitSystem::SetLength(size/btScalar(2)), size/btScalar(2));
    ghost->setWorldTransform(btTransform(btQuaternion::getIdentity(), btVector3(0,0,size/btScalar(2)))); //Surface at 0
    ghost->setCollisionShape(new btBoxShape(halfExtents));
    
    currents = std::vector<VelocityField*>(0);
     
    glOcean = new OpenGLOcean();
    trueWaves = false;
    liquid = l;
    setAlgeaBloomFactor(0.2f);
    setTurbidity(100.f);
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
    delete glOcean;
}

void Ocean::setUseTrueWaves(bool waves)
{
    trueWaves = false; //No support for waves yet
}

bool Ocean::usesTrueWaves()
{
    return trueWaves;
}

OpenGLOcean* Ocean::getOpenGLOcean()
{
    return glOcean;
}

ForcefieldType Ocean::getForcefieldType()
{
    return FORCEFIELD_OCEAN;
}

const Liquid* Ocean::getLiquid() const
{
    return liquid;
}

void Ocean::setAlgeaBloomFactor(GLfloat f)
{
    algeaBloom = f < 0.f ? 0.f : (f > 1.f ? 1.f : f);
    glOcean->setLightAbsorption(ComputeLightAbsorption());
}

void Ocean::setTurbidity(GLfloat ntu)
{
    turbidity = ntu < 0.f ? 0.f : ntu;
	glOcean->setTurbidity(turbidity);
}
    
GLfloat Ocean::getAlgeaBloomFactor()
{
    return algeaBloom;
}

GLfloat Ocean::getTurbidity()
{
    return turbidity;
}

void Ocean::AddVelocityField(VelocityField* field)
{
    currents.push_back(field);
}

glm::vec3 Ocean::ComputeLightAbsorption()
{
    return glm::vec3(0.2f+0.2f*(algeaBloom), 0.02f, 0.02f+0.02f*(algeaBloom));
}

bool Ocean::IsInsideFluid(const btVector3& point) const
{
    return point.z() >= btScalar(0);
}

btScalar Ocean::GetDepth(const btVector3& point) const
{
    return point.z();
}

btScalar Ocean::GetPressure(const btVector3& point) const
{
    btScalar g = 9.81;
    btScalar d = point.z();
    btScalar pressure = d > btScalar(0) ? d*liquid->density*g : btScalar(0);
    return pressure;
}

btVector3 Ocean::GetFluidVelocity(const btVector3& point) const
{
    btVector3 fv(0,0,0);
    for(unsigned int i=0; i<currents.size(); ++i)
        fv += currents[i]->GetVelocityAtPoint(point);
    return fv;
}

void Ocean::GetSurface(btVector3& normal, btVector3& position) const
{
    normal = -ghost->getWorldTransform().getBasis().getColumn(2).normalized();
    position = ghost->getWorldTransform().getOrigin()+normal*(depth/2.0);
}

void Ocean::GetSurfaceEquation(double* plane4) const
{
    btVector3 normal = -ghost->getWorldTransform().getBasis().getColumn(2).normalized();
    btVector3 position = ghost->getWorldTransform().getOrigin()+normal*(depth/2.0);
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

std::vector<Renderable> Ocean::Render()
{
    std::vector<Renderable> items(0);
    
    for(unsigned int i=0; i<currents.size(); ++i)
    {
        std::vector<Renderable> citems = currents[i]->Render();
        items.insert(items.end(), citems.begin(), citems.end());
    }
    
    return items;
}