//
//  Compound.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 19/09/17.
//  Copyright (c) 2017 Patryk Cieslak. All rights reserved.
//

#include "entities/solids/Compound.h"

#include "core/SimulationApp.h"
#include "utils/MathUtil.hpp"

using namespace sf;

Compound::Compound(std::string uniqueName, SolidEntity* firstExternalPart, const Transform& position) : SolidEntity(uniqueName, firstExternalPart->getMaterial(), firstExternalPart->getLook())
{
    volume = 0;
	mass = 0;
	Ipri = Vector3(0,0,0);
	mesh = NULL;
	AddExternalPart(firstExternalPart, position);
}

Compound::~Compound()
{
    for(unsigned int i=0; i<parts.size(); ++i)
        delete parts[i].solid;
    
    parts.clear();
}

SolidType Compound::getSolidType()
{
    return SOLID_COMPOUND;
}

std::vector<Vertex>* Compound::getMeshVertices()
{
    std::vector<Vertex>* pVert = new std::vector<Vertex>(0);
    
    for(unsigned int i=0; i<parts.size(); ++i)
    {
        if(parts[i].isExternal)
        {
            std::vector<Vertex>* pPartVert = parts[i].solid->getMeshVertices();
            
            for(unsigned int h=0; h < pPartVert->size(); ++h)
            {
                glm::mat4 trans = glMatrixFromBtTransform(parts[i].position);
                glm::vec4 vTrans = trans * glm::vec4((*pPartVert)[h].pos, 1.f);
                Vertex v;
                v.pos = glm::vec3(vTrans);
                pVert->push_back(v);
            }
            
            delete pPartVert;
        }
    }
    
    return pVert;
}

void Compound::AddInternalPart(SolidEntity* solid, const Transform& position)
{
    if(solid != NULL)
    {
        Part part;
        part.solid = solid;
        part.position = position;
        part.isExternal = false;
        parts.push_back(part);
		RecalculatePhysicalProperties();
    }
}

void Compound::AddExternalPart(SolidEntity* solid, const Transform& position)
{
    if(solid != NULL)
    {
        Part part;
        part.solid = solid;
        part.position = position;
        part.isExternal = true;
        parts.push_back(part);
		RecalculatePhysicalProperties();
    }
}

void Compound::RecalculatePhysicalProperties()
{
	//Calculate rigid body properties
    /*
      1. Calculate compound mass and compound COG (sum of location - local * m / M)
      2. Calculate inertia of part in global frame and compound COG (rotate and translate inertia tensor) 3x3
         and calculate compound inertia 3x3 (sum of parts inertia)
      3. Calculate primary moments of inertia
      4. Find primary axes of inertia
      5. Rotate frame to match primary axes and move to COG
    */
     
    //1. Calculate compound mass and COG
    Vector3 compoundCOG(0,0,0);
    Vector3 compoundCOB(0,0,0);
    Scalar compoundMass = 0;
	Scalar compoundVolume = 0;
    T_G2CG = Transform::getIdentity();
        
    for(unsigned int i=0; i<parts.size(); ++i)
    {
        compoundMass += parts[i].solid->getMass();
        //compoundCOG += (bodyParts[i].position.getOrigin() + bodyParts[i].solid->getLocalTransform().getOrigin())*bodyParts[i].solid->getMass();
        compoundCOG += (parts[i].position*parts[i].solid->getG2CGTransform().getOrigin())*parts[i].solid->getMass();
        
        if(parts[i].solid->isBuoyant())
        {
            compoundVolume += parts[i].solid->getVolume();
            compoundCOB += (parts[i].position*parts[i].solid->getCBPositionInGFrame())*parts[i].solid->getVolume();
        }
    }
    
    compoundCOG /= compoundMass;
    T_G2CG.setOrigin(compoundCOG);
    
    if(compoundVolume > Scalar(0))
        CB = compoundCOB / compoundVolume;
        
    //2. Calculate compound inertia matrix
    Matrix3 I = Matrix3(0,0,0,0,0,0,0,0,0);
        
    for(unsigned int i=0; i<parts.size(); ++i)
    {
        //Calculate inertia matrix 3x3 of solid in the global frame and COG
        Vector3 solidPriInertia = parts[i].solid->getInertia();
        Matrix3 solidInertia = Matrix3(solidPriInertia.x(), 0, 0, 0, solidPriInertia.y(), 0, 0, 0, solidPriInertia.z());
            
        //Rotate inertia tensor from local to global
        Matrix3 rotation = parts[i].position.getBasis()*parts[i].solid->getG2CGTransform().getBasis();
        solidInertia = rotation * solidInertia * rotation.transpose();
            
        //Translate inertia tensor from local COG to global COG
        Vector3 translation = parts[i].position.getOrigin()+parts[i].solid->getG2CGTransform().getOrigin()-compoundCOG;
        solidInertia = solidInertia +  Matrix3(translation.y()*translation.y()+translation.z()*translation.z(), -translation.x()*translation.y(), -translation.x()*translation.z(),
                                                    -translation.y()*translation.x(), translation.x()*translation.x()+translation.z()*translation.z(), -translation.y()*translation.z(),
                                                    -translation.z()*translation.x(), -translation.z()*translation.y(), translation.x()*translation.x()+translation.y()*translation.y()).scaled(Vector3(parts[i].solid->getMass(), parts[i].solid->getMass(), parts[i].solid->getMass()));
            
        //Accumulate inertia tensor
        I += solidInertia;
    }
	
	//3. Find compound moments of inertia
	Vector3 compoundPriInertia(I.getRow(0).getX(), I.getRow(1).getY(), I.getRow(2).getZ());
	
	//Check if inertia matrix is not diagonal
	if(!(btFuzzyZero(I.getRow(0).getY()) && btFuzzyZero(I.getRow(0).getZ())
	     && btFuzzyZero(I.getRow(1).getX()) && btFuzzyZero(I.getRow(1).getZ())
	     && btFuzzyZero(I.getRow(2).getX()) && btFuzzyZero(I.getRow(2).getY())))
	{
		//3.1. Calculate principal moments of inertia
		Scalar T = I[0][0] + I[1][1] + I[2][2]; //Ixx + Iyy + Izz
		Scalar II = I[0][0]*I[1][1] + I[0][0]*I[2][2] + I[1][1]*I[2][2] - I[0][1]*I[0][1] - I[0][2]*I[0][2] - I[1][2]*I[1][2]; //Ixx Iyy + Ixx Izz + Iyy Izz - Ixy^2 - Ixz^2 - Iyz^2
		Scalar U = btSqrt(T*T-Scalar(3.)*II)/Scalar(3.);
		Scalar theta = btAcos((-Scalar(2.)*T*T*T + Scalar(9.)*T*II - Scalar(27.)*I.determinant())/(Scalar(54.)*U*U*U));
		Scalar A = T/Scalar(3.) - Scalar(2.)*U*btCos(theta/Scalar(3.));
		Scalar B = T/Scalar(3.) - Scalar(2.)*U*btCos(theta/Scalar(3.) - Scalar(2.)*M_PI/Scalar(3.));
		Scalar C = T/Scalar(3.) - Scalar(2.)*U*btCos(theta/Scalar(3.) + Scalar(2.)*M_PI/Scalar(3.));
		compoundPriInertia = Vector3(A, B, C);
    
		//3.2. Calculate principal axes of inertia
		Matrix3 L;
		Vector3 axis1,axis2,axis3;
		axis1 = findInertiaAxis(I, A);
		axis2 = findInertiaAxis(I, B);
		axis3 = axis1.cross(axis2);
		axis2 = axis3.cross(axis1);
    
		//3.3. Rotate body so that principal axes are parallel to (x,y,z) system
		Matrix3 rotMat(axis1[0],axis2[0],axis3[0], axis1[1],axis2[1],axis3[1], axis1[2],axis2[2],axis3[2]);
		T_G2CG.setBasis(rotMat);
    }
	
	mass = compoundMass;
	volume = compoundVolume;
	Ipri = compoundPriInertia;
    
    ComputeHydrodynamicProxy(HYDRO_PROXY_ELLIPSOID);
}

btCollisionShape* Compound::BuildCollisionShape()
{
	//Build collision shape from external parts
    btCompoundShape* colShape = new btCompoundShape();
    for(unsigned int i=0; i<parts.size(); ++i)
    {
        if(parts[i].isExternal)
        {
            Transform childTrans = parts[i].position;
            btCollisionShape* partColShape = parts[i].solid->BuildCollisionShape();
            colShape->addChildShape(childTrans, partColShape);
        }
    }
	return colShape;
}

void Compound::ComputeFluidForces(HydrodynamicsSettings settings, Ocean* liquid)
{
    if(!computeHydro)
        return;
    
    BodyFluidPosition bf = CheckBodyFluidPosition(liquid);
    
    //If completely outside fluid just set all torques and forces to 0
    if(bf == BodyFluidPosition::OUTSIDE_FLUID)
    {
        Fb.setZero();
        Tb.setZero();
        Fds.setZero();
        Tds.setZero();
        Fdp.setZero();
        Tdp.setZero();
        return;
    }
    
    //Get velocities and transformations
    Transform T = getCGTransform() * T_G2CG.inverse();
    Vector3 v = getLinearVelocity();
    Vector3 omega = getAngularVelocity();
	
    //Clear forces that will be recomputed
    if(settings.reallisticBuoyancy)
    {
        Fb.setZero();
        Tb.setZero();
    }
    
    if(settings.dampingForces)
    {
        Fds.setZero();
        Tds.setZero();
        Fdp.setZero();
        Tdp.setZero();
    }
    
    if(settings.reallisticBuoyancy || settings.dampingForces)
    {
        Vector3 Fbp(0,0,0);
        Vector3 Tbp(0,0,0);
        Vector3 Fdsp(0,0,0);
        Vector3 Tdsp(0,0,0);
        Vector3 Fdpp(0,0,0);
        Vector3 Tdpp(0,0,0);
	
        for(unsigned int i=0; i<parts.size(); ++i)
        {
            if(parts[i].isExternal)
            {
                if(bf == BodyFluidPosition::INSIDE_FLUID)
                    parts[i].solid->ComputeFluidForcesSubmerged(settings, liquid, getCGTransform(), T * parts[i].position, v, omega, Fbp, Tbp, Fdsp, Tdsp, Fdpp, Tdpp);
                else //CROSSING_FLUID_SURFACE
                    parts[i].solid->ComputeFluidForcesSurface(settings, liquid, getCGTransform(), T * parts[i].position, v, omega, Fbp, Tbp, Fdsp, Tdsp, Fdpp, Tdpp);
                
                Fb += Fbp;
                Tb += Tbp;
                Fds += Fdsp;
                Tds += Tdsp;
                Fdp += Fdpp;
                Tdp += Tdpp;
            }
            else if(settings.reallisticBuoyancy)
            {
                HydrodynamicsSettings iSettings = settings;
                iSettings.dampingForces = false;
                
                if(bf == BodyFluidPosition::INSIDE_FLUID)
                    parts[i].solid->ComputeFluidForcesSubmerged(iSettings, liquid, getCGTransform(), T * parts[i].position, v, omega, Fbp, Tbp, Fdsp, Tdsp, Fdpp, Tdpp);
                else //CROSSING_FLUID_SURFACE
                    parts[i].solid->ComputeFluidForcesSurface(iSettings, liquid, getCGTransform(), T * parts[i].position, v, omega, Fbp, Tbp, Fdsp, Tdsp, Fdpp, Tdpp);
                
                Fb += Fbp;
                Tb += Tbp;
            }
        }
        
        if(settings.dampingForces)
        {
            Transform hpTrans = getCGTransform() * T_G2CG.inverse() * hydroProxyTransform;
            
            switch(hydroProxyType)
            {
                case HYDRO_PROXY_NONE:
                    //No info to correct
                    break;
                
                case HYDRO_PROXY_SPHERE:
                    //No need to correct
                    break;
                    
                case HYDRO_PROXY_CYLINDER:
                {
                    //Correct drag based on cylindrical approximation of shape
                    Vector3 cFd = hpTrans.getBasis().inverse() * Fdp;
                    Vector3 Cd(0.5, 1.0, 0.5);
                    cFd = Vector3(Cd.x()*cFd.x(), Cd.y()*cFd.y(), Cd.z()*cFd.z());
                    Fdp = hpTrans.getBasis() * cFd;
                }
                    break;
                    
                case HYDRO_PROXY_ELLIPSOID:
                {
                    //Correct drag based on ellipsoid approximation of shape
                    Vector3 eFd = hpTrans.getBasis().inverse() * Fdp;
                    //Vector3 eTd = eTrans.getBasis().inverse() * Td;
                    Vector3 Cd(Scalar(1)/hydroProxyParams[0] , Scalar(1)/hydroProxyParams[1], Scalar(1)/hydroProxyParams[2]);
                    Scalar maxCd = btMax(btMax(Cd.x(), Cd.y()), Cd.z());
                    Cd /= maxCd;
                    eFd = Vector3(Cd.x()*eFd.x(), Cd.y()*eFd.y(), Cd.z()*eFd.z());
                    Fdp = hpTrans.getBasis() * eFd;
                }
                break;
            }
        }
    }
}

void Compound::BuildGraphicalObject()
{
	for(unsigned int i=0; i<parts.size(); ++i)
		parts[i].solid->BuildGraphicalObject();
}

std::vector<Renderable> Compound::Render()
{
	std::vector<Renderable> items(0);
	
	if(isRenderable())
	{
		Transform cgCompoundTrans = getCGTransform();
		Transform oCompoundTrans =  getCGTransform() * T_G2CG.inverse();
	
        Renderable item;
        item.type = RenderableType::SOLID_CS;
        item.model = glMatrixFromBtTransform(cgCompoundTrans);
        items.push_back(item);
        
        Vector3 cobWorld = oCompoundTrans * CB;
        item.type = RenderableType::HYDRO_CS;
        item.model = glMatrixFromBtTransform(Transform(Quaternion::getIdentity(), cobWorld));
        item.points.push_back(glm::vec3(volume, volume, volume));
        items.push_back(item);
    
		for(unsigned int i=0; i<parts.size(); ++i)
		{
			Transform oTrans = oCompoundTrans * parts[i].position;
			
			Renderable item;
            item.type = RenderableType::SOLID;
			item.objectId = parts[i].solid->getObject();
			item.lookId = parts[i].solid->getLook();
			item.model = glMatrixFromBtTransform(oTrans);
			items.push_back(item);
            
            item.type = RenderableType::HYDRO_ELLIPSOID;
            item.model = glMatrixFromBtTransform(oCompoundTrans * hydroProxyTransform);
            item.points.push_back(glm::vec3((GLfloat)hydroProxyParams[0], (GLfloat)hydroProxyParams[1], (GLfloat)hydroProxyParams[2]));
            items.push_back(item);
		}
	}
		
	return items;
}
