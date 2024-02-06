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
//  Compound.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 19/09/17.
//  Copyright (c) 2017-2018 Patryk Cieslak. All rights reserved.
//

#include "entities/solids/Compound.h"

#include "core/SimulationApp.h"
#include "core/SimulationManager.h"
#include "utils/GeometryFileUtil.h"

namespace sf
{

Compound::Compound(std::string uniqueName, BodyPhysicsSettings phy, SolidEntity* firstExternalPart, const Transform& origin)
    : SolidEntity(uniqueName, phy, "", "", Scalar(-1))
{
    //All transformations are zero -> transforming the origin of a compound body doesn't make sense...
    phyMesh = NULL; // There is no single mesh
    volume = 0;
    mass = 0;
    Ipri = Vector3(0,0,0);
    displayInternals = false;
    
    AddExternalPart(firstExternalPart, origin);
}

Compound::~Compound()
{
    for(unsigned int i=0; i<parts.size(); ++i)
        delete parts[i].solid;
    parts.clear();
}

void Compound::setDisplayInternalParts(bool enabled)
{
    displayInternals = enabled;
}

bool Compound::isDisplayingInternalParts()
{
    return displayInternals;
}

Scalar Compound::getAugmentedMass() const
{
    return mass + aMass.x();
}
        
Vector3 Compound::getAugmentedInertia() const
{
    return Ipri;
}
    
Material Compound::getMaterial(size_t partId) const
{
    if(partId < parts.size())
        return parts[partId].solid->getMaterial();
    else
        return Material();
}

size_t Compound::getPartId(size_t collisionShapeId) const
{
    if(collisionShapeId < collisionPartId.size())
        return collisionPartId[collisionShapeId];
    else
        return 0;
}

const CompoundPart Compound::getPart(size_t partId) const
{
    try
    {
        return parts.at(partId);
    }
    catch(const std::exception& e)
    {
        CompoundPart nopart;
        nopart.solid = nullptr;
        return nopart;
    }
}
    
SolidType Compound::getSolidType()
{
    return SolidType::COMPOUND;
}

std::vector<Vector3>* Compound::getMeshVertices() const
{
    std::vector<Vector3>* pVert = new std::vector<Vector3>(0);
        
    for(size_t i=0; i<parts.size(); ++i)
    {
        if(parts[i].isExternal)
        {
            std::vector<Vector3>* pPartVert = parts[i].solid->getMeshVertices();
            Transform phyMeshTrans = parts[i].origin * parts[i].solid->getO2GTransform();
            for(size_t h=0; h < pPartVert->size(); ++h)
            {
                Vector3 v = phyMeshTrans * pPartVert->at(h);
                pVert->push_back(v);
            }    
            delete pPartVert;
        }
    }
        
    return pVert;
}
    
void Compound::AddInternalPart(SolidEntity* solid, const Transform& origin, bool alwaysVisible)
{
    if(solid != NULL)
    {
        CompoundPart part;
        part.solid = solid;
        part.origin = origin;
        part.isExternal = false;
        part.alwaysVisible = alwaysVisible;
        parts.push_back(part);
        RecalculatePhysicalProperties();
    }
}

void Compound::AddExternalPart(SolidEntity* solid, const Transform& origin)
{
    if(solid != NULL)
    {
        CompoundPart part;
        part.solid = solid;
        part.origin = origin;
        part.isExternal = true;
        part.alwaysVisible = false;
        parts.push_back(part);
        RecalculatePhysicalProperties();
    }
}

void Compound::RecalculatePhysicalProperties()
{
    //Calculate rigid body properties
    /*
      1. Calculate compound mass and compound CG (sum of location - local * m / M)
      2. Calculate inertia of part in global frame and compound CG (rotate and translate inertia tensor) 3x3
         and calculate compound inertia 3x3 (sum of parts inertia)
      3. Calculate primary moments of inertia
      4. Find primary axes of inertia
      5. Rotate frame to match primary axes and move to CG
    */
     
    //1. Calculate compound mass, CG and CB
    Vector3 compoundCG(0,0,0); //In compound body origin frame
    Vector3 compoundCB(0,0,0); //In compound body origin frame
    Scalar compoundMass(0);
    Scalar compoundAugmentedMass(0);
    Scalar compoundVolume(0);
    Scalar compoundSurface(0);
    
    T_CG2O = T_CG2C = T_CG2G = Transform::getIdentity();
    P_CB = Vector3(0,0,0);
        
    for(size_t i=0; i<parts.size(); ++i)
    {
        //Mechanical parameters
        compoundMass += parts[i].solid->getMass();
        compoundAugmentedMass += parts[i].isExternal ? parts[i].solid->getAugmentedMass() : parts[i].solid->getMass();
        compoundCG += (parts[i].origin * parts[i].solid->getCG2OTransform().inverse()).getOrigin() * parts[i].solid->getMass();
        
        if(parts[i].solid->isBuoyant())
        {
            compoundVolume += parts[i].solid->getVolume();
            compoundCB += parts[i].origin * parts[i].solid->getCG2OTransform().inverse() * parts[i].solid->getCB() * parts[i].solid->getVolume();
        }

        if(parts[i].isExternal)
            compoundSurface += parts[i].solid->getSurface();
    }
    
    //Set transform origin
    compoundCG /= compoundMass;
    if(compoundVolume > Scalar(0)) compoundCB /= compoundVolume;
    T_CG2O.setOrigin(-compoundCG);
    
    //2. Calculate compound inertia matrix
    Matrix3 I = Matrix3(0,0,0,0,0,0,0,0,0);
        
    for(unsigned int i=0; i<parts.size(); ++i)
    {
        //Calculate inertia matrix 3x3 of solid in the compound body origin frame and transform to CB
        Vector3 solidPriInertia = parts[i].isExternal ? parts[i].solid->getAugmentedInertia() : parts[i].solid->getInertia();
        Matrix3 solidInertia = Matrix3(solidPriInertia.x(), 0, 0, 0, solidPriInertia.y(), 0, 0, 0, solidPriInertia.z());
            
        //Rotate inertia tensor from part CG to compound CG
        Transform compToPart = T_CG2O * parts[i].origin * parts[i].solid->getCG2OTransform().inverse();
        solidInertia = compToPart.getBasis() * solidInertia * compToPart.getBasis().transpose();
            
        //Translate inertia tensor from part CG to compound CG
        Vector3 t = compToPart.getOrigin();
        Scalar m = parts[i].isExternal ? parts[i].solid->getAugmentedMass() : parts[i].solid->getMass();
        solidInertia += Matrix3(t.y()*t.y()+t.z()*t.z(),            -t.x()*t.y(),            -t.x()*t.z(),
                                           -t.y()*t.x(), t.x()*t.x()+t.z()*t.z(),            -t.y()*t.z(),
                                           -t.z()*t.x(),            -t.z()*t.y(), t.x()*t.x()+t.y()*t.y()).scaled(Vector3(m, m, m));
            
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
        axis1 = FindInertialAxis(I, A);
        axis2 = FindInertialAxis(I, B);
        axis3 = axis1.cross(axis2);
        axis2 = axis3.cross(axis1);
    
        //3.3. Rotate body so that principal axes are parallel to (x,y,z) system
        Matrix3 rotMat(axis1[0],axis2[0],axis3[0], axis1[1],axis2[1],axis3[1], axis1[2],axis2[2],axis3[2]);
        T_CG2O = Transform(rotMat, Vector3(0,0,0)).inverse() * T_CG2O;
    }
    
    T_CG2C = T_CG2G = T_CG2O;
    
    //Move CB to compound CG frame
    P_CB = T_CG2O * compoundCB;
    
    mass = compoundMass;
    aMass.setX(compoundAugmentedMass - compoundMass);
    aMass.setY(compoundAugmentedMass - compoundMass);
    aMass.setZ(compoundAugmentedMass - compoundMass);
    volume = compoundVolume;
    surface = compoundSurface;
    Ipri = compoundPriInertia;
}

btCollisionShape* Compound::BuildCollisionShape()
{
    //Build collision shape from external parts
    btCompoundShape* colShape = new btCompoundShape();
    for(size_t i = 0; i<parts.size(); ++i)
    {
        if(parts[i].isExternal)
        {
            Transform childTrans = parts[i].origin * parts[i].solid->getCG2OTransform().inverse() * parts[i].solid->getCG2CTransform();
            btCollisionShape* partColShape = parts[i].solid->BuildCollisionShape();
            colShape->addChildShape(childTrans, partColShape);
            collisionPartId.push_back(i);
        }
    }
    return colShape;
}

void Compound::ComputeHydrodynamicForces(HydrodynamicsSettings settings, Ocean* ocn)
{
    if(phy.mode != BodyPhysicsMode::FLOATING && phy.mode != BodyPhysicsMode::SUBMERGED) return;
    
    submerged.points.clear();

    BodyFluidPosition bf = CheckBodyFluidPosition(ocn);
     
    //If completely outside fluid just set all torques and forces to 0
    if(bf == BodyFluidPosition::OUTSIDE)
    {
        Fb.setZero();
        Tb.setZero();
        Fdq.setZero();
        Tdq.setZero();
        Fdf.setZero();
        Tdf.setZero();
        Swet = Scalar(0);
        Vsub = Scalar(0);
        return;
    }
    
    if(bf == BodyFluidPosition::INSIDE)
    {
        //Compute buoyancy based on CB position
        if(isBuoyant())
        {
            Fb = -volume*ocn->getLiquid().density * SimulationApp::getApp()->getSimulationManager()->getGravity();
            Tb = (getCGTransform() * P_CB - getCGTransform().getOrigin()).cross(Fb);
        }
        
        if(settings.dampingForces)
        {
            //Set zero
            Fdq.setZero();
            Tdq.setZero();
            Fdf.setZero();
            Tdf.setZero();
            
            //Get velocity data
            Vector3 v = getLinearVelocity();
            Vector3 omega = getAngularVelocity();
            
            //Create temporary vectors for summing
            Vector3 Fdqp(0,0,0);
            Vector3 Tdqp(0,0,0);
            Vector3 Fdfp(0,0,0);
            Vector3 Tdfp(0,0,0);
            
            for(size_t i=0; i<parts.size(); ++i) //Go through all parts
                if(parts[i].isExternal 
                    && (parts[i].solid->getBodyPhysicsMode() == BodyPhysicsMode::SUBMERGED
                    || parts[i].solid->getBodyPhysicsMode() == BodyPhysicsMode::FLOATING)) //Compute drag only for external parts
                {
                    Transform T_C_part = getOTransform() * parts[i].origin * parts[i].solid->getO2CTransform();
                    ComputeHydrodynamicForcesSubmerged(parts[i].solid->getPhysicsMesh(), ocn, getCGTransform(), T_C_part, v, omega, Fdqp, Tdqp, Fdfp, Tdfp);
                    parts[i].solid->CorrectHydrodynamicForces(ocn, Fdqp, Tdqp, Fdfp, Tdfp);
                    Fdq += Fdqp;
                    Tdq += Tdqp;
                    Fdf += Fdfp;
                    Tdf += Tdfp;
                }
        }

        Swet = surface;
        Vsub = volume;
    }
    else //CROSSING FLUID SURFACE (compound body but not necessarily all parts!)
    {
        if(settings.reallisticBuoyancy || settings.dampingForces)
        {
            //Clear forces that will be recomputed
            if(settings.reallisticBuoyancy)
            {
                Fb.setZero();
                Tb.setZero();
            }
        
            if(settings.dampingForces)
            {
                Fdq.setZero();
                Tdq.setZero();
                Fdf.setZero();
                Tdf.setZero();
            }

            Swet = Scalar(0);
            Vsub = Scalar(0);
        
            //Get velocity data
            Vector3 v = getLinearVelocity();
            Vector3 omega = getAngularVelocity();
        
            //Create temporary vectors for summing
            Vector3 Fbp(0,0,0);
            Vector3 Tbp(0,0,0);
            Vector3 Fdqp(0,0,0);
            Vector3 Tdqp(0,0,0);
            Vector3 Fdfp(0,0,0);
            Vector3 Tdfp(0,0,0);
            Scalar Swetp(0);
            Scalar Vsubp(0);

            for(size_t i=0; i<parts.size(); ++i) //Loop through all parts
            {
                if(parts[i].solid->getBodyPhysicsMode() != BodyPhysicsMode::SUBMERGED
                    && parts[i].solid->getBodyPhysicsMode() != BodyPhysicsMode::FLOATING)
                    continue;

                Transform T_C_part = getOTransform() * parts[i].origin * parts[i].solid->getO2CTransform();
                HydrodynamicsSettings pSettings = settings;
                pSettings.reallisticBuoyancy &= parts[i].solid->isBuoyant();

                if(parts[i].isExternal) //Compute buoyancy and drag
                {
                    ComputeHydrodynamicForcesSurface(pSettings, parts[i].solid->getPhysicsMesh(), ocn, getCGTransform(), T_C_part, v, omega, Fbp, Tbp, Fdqp, Tdqp, Fdfp, Tdfp, Swetp, Vsubp, submerged);
                    parts[i].solid->CorrectHydrodynamicForces(ocn, Fdqp, Tdqp, Fdfp, Tdfp);
                    Fb += Fbp;
                    Tb += Tbp;
                    Fdq += Fdqp;
                    Tdq += Tdqp;
                    Fdf += Fdfp;
                    Tdf += Tdfp;
                    Swet += Swetp;
                    Vsub += Vsubp;
                }
                else if(pSettings.reallisticBuoyancy) //Compute only buoyancy
                {
                    pSettings.dampingForces = false;
                    ComputeHydrodynamicForcesSurface(pSettings, parts[i].solid->getPhysicsMesh(), ocn, getCGTransform(), T_C_part, v, omega, Fbp, Tbp, Fdqp, Tdqp, Fdfp, Tdfp, Swetp, Vsubp, submerged);
                    Fb += Fbp;
                    Tb += Tbp;
                    Vsub += Vsubp;
                }
            }
        }
    }
}

void Compound::ComputeAerodynamicForces(Atmosphere* atm)
{
    if(phy.mode != BodyPhysicsMode::AERODYNAMIC) return;
    
    //Set zero
    Fda.setZero();
    Tda.setZero();
            
    //Get velocity data
    Vector3 v = getLinearVelocity();
    Vector3 omega = getAngularVelocity();
            
    //Create temporary vectors for summing
    Vector3 Fdap(0,0,0);
    Vector3 Tdap(0,0,0);
            
    for(size_t i=0; i<parts.size(); ++i) //Go through all parts
        if(parts[i].isExternal) //Compute drag only for external parts
        {
            Transform T_C_part = getOTransform() * parts[i].origin * parts[i].solid->getO2CTransform();
            SolidEntity::ComputeAerodynamicForces(parts[i].solid->getPhysicsMesh(), atm, getCGTransform(), T_C_part, v, omega, Fdap, Tdap);
            parts[i].solid->CorrectAerodynamicForces(atm, Fdap, Tdap);
            Fda += Fdap;
            Tda += Tdap;
        }
}

void Compound::BuildGraphicalObject()
{
    for(unsigned int i=0; i<parts.size(); ++i)
        parts[i].solid->BuildGraphicalObject();
}

std::vector<Renderable> Compound::Render(size_t partId)
{
    std::vector<Renderable> items(0);
    Transform oCompoundTrans = getOTransform();

    try
    {
        Renderable item;

        if((parts.at(partId).isExternal && !displayInternals)  
            || (!parts.at(partId).isExternal && displayInternals)
            || (parts.at(partId).alwaysVisible))
        {
            item.type = RenderableType::SOLID;
            item.materialName = parts.at(partId).solid->getMaterial().name;
                
            if(dm == DisplayMode::GRAPHICAL)
            {
                Transform oTrans = oCompoundTrans * parts.at(partId).origin * parts.at(partId).solid->getO2GTransform();
                item.objectId = parts.at(partId).solid->getGraphicalObject();
                item.lookId = parts.at(partId).solid->getLook();
                item.model = glMatrixFromTransform(oTrans);
                item.vel = glVectorFromVector(getLinearVelocity());
                item.avel = glVectorFromVector(getAngularVelocity());
                items.push_back(item);
            }
            else if(dm == DisplayMode::PHYSICAL)
            {
                Transform oTrans = oCompoundTrans * parts.at(partId).origin * parts.at(partId).solid->getO2CTransform();
                item.objectId = parts.at(partId).solid->getPhysicalObject();
                item.lookId = -1;
                item.model = glMatrixFromTransform(oTrans);
                item.vel = glVectorFromVector(getLinearVelocity());
                item.avel = glVectorFromVector(getAngularVelocity());
                items.push_back(item);
            }
        }

#ifndef DEBUG_HYDRO
        GeometryApproxType atype;
        std::vector<Scalar> aparams;
        parts.at(partId).solid->getGeometryApprox(atype, aparams);
        item.model = glMatrixFromTransform(oCompoundTrans * parts.at(partId).origin * parts.at(partId).solid->getO2HTransform());
        
        switch(atype)
        {
            case  GeometryApproxType::AUTO:
                break;
            
            case  GeometryApproxType::SPHERE:
                item.type = RenderableType::HYDRO_ELLIPSOID;
                item.points.push_back(glm::vec3((GLfloat)aparams[0], (GLfloat)aparams[0], (GLfloat)aparams[0]));
                items.push_back(item);
                break;
            
            case  GeometryApproxType::CYLINDER:
                item.type = RenderableType::HYDRO_CYLINDER;
                item.points.push_back(glm::vec3((GLfloat)aparams[0], (GLfloat)aparams[0], (GLfloat)aparams[1]));
                items.push_back(item);
                break;
            
            case  GeometryApproxType::ELLIPSOID:
                item.type = RenderableType::HYDRO_ELLIPSOID;
                item.points.push_back(glm::vec3((GLfloat)aparams[0], (GLfloat)aparams[1], (GLfloat)aparams[2]));
                items.push_back(item);
                break;
        }   
#endif
    }
    catch(const std::exception& e)
    {
        //Error finding part id
    }      

    return items;
}

std::vector<Renderable> Compound::Render()
{
    std::vector<Renderable> items(0);
    
    if(isRenderable())
    {
        Renderable item;
        item.type = RenderableType::SOLID_CS;
        item.model = glMatrixFromTransform(getCGTransform());
        items.push_back(item);
        
        Vector3 cbWorld = getCGTransform() * P_CB;
        item.type = RenderableType::HYDRO_CS;
        item.model = glMatrixFromTransform(Transform(Quaternion::getIdentity(), cbWorld));
        item.points.push_back(glm::vec3(volume, volume, volume));
        items.push_back(item);
        item.points.clear();
        
        Transform oCompoundTrans = getOTransform();
        
        for(size_t i=0; i<parts.size(); ++i)
        {
            std::vector<Renderable> partItems = Render(i);
            items.insert(items.end(), partItems.begin(), partItems.end());
        }

        //Forces
        Vector3 cg = getCGTransform().getOrigin();
        glm::vec3 cgv((GLfloat)cg.x(), (GLfloat)cg.y(), (GLfloat)cg.z());
        item.points.clear();
        item.points.push_back(cgv);
        item.model = glm::mat4(1.f);
        
        item.type = RenderableType::FORCE_BUOYANCY;
        item.points.push_back(cgv + glm::vec3((GLfloat)Fb.x(), (GLfloat)Fb.y(), (GLfloat)Fb.z())/1000.f);
        items.push_back(item);
        
        item.points.pop_back();
        item.type = RenderableType::FORCE_LINEAR_DRAG;
        item.points.push_back(cgv + glm::vec3((GLfloat)Fdf.x(), (GLfloat)Fdf.y(), (GLfloat)Fdf.z()));
        items.push_back(item);
        
        item.points.pop_back();
        item.type = RenderableType::FORCE_QUADRATIC_DRAG;
        item.points.push_back(cgv + glm::vec3((GLfloat)Fdq.x(), (GLfloat)Fdq.y(), (GLfloat)Fdq.z()));
        items.push_back(item);

#ifdef DEBUG_HYDRO
        items.push_back(submerged);

        item.type = RenderableType::HYDRO_LINES;
        item.model = glm::mat4(1.f);
        
        Vector3 min, max;
        getAABB(min, max);

        item.points.push_back(glm::vec3((GLfloat)min.x(), (GLfloat)min.y(), (GLfloat)min.z()));
        item.points.push_back(glm::vec3((GLfloat)max.x(), (GLfloat)min.y(), (GLfloat)min.z()));

        item.points.push_back(glm::vec3((GLfloat)min.x(), (GLfloat)min.y(), (GLfloat)min.z()));
        item.points.push_back(glm::vec3((GLfloat)min.x(), (GLfloat)max.y(), (GLfloat)min.z()));

        item.points.push_back(glm::vec3((GLfloat)min.x(), (GLfloat)min.y(), (GLfloat)min.z()));
        item.points.push_back(glm::vec3((GLfloat)min.x(), (GLfloat)min.y(), (GLfloat)max.z()));

        item.points.push_back(glm::vec3((GLfloat)min.x(), (GLfloat)max.y(), (GLfloat)min.z()));
        item.points.push_back(glm::vec3((GLfloat)min.x(), (GLfloat)max.y(), (GLfloat)max.z()));

        item.points.push_back(glm::vec3((GLfloat)min.x(), (GLfloat)min.y(), (GLfloat)max.z()));
        item.points.push_back(glm::vec3((GLfloat)min.x(), (GLfloat)max.y(), (GLfloat)max.z()));

        item.points.push_back(glm::vec3((GLfloat)min.x(), (GLfloat)min.y(), (GLfloat)max.z()));
        item.points.push_back(glm::vec3((GLfloat)max.x(), (GLfloat)min.y(), (GLfloat)max.z()));

        item.points.push_back(glm::vec3((GLfloat)max.x(), (GLfloat)min.y(), (GLfloat)min.z()));
        item.points.push_back(glm::vec3((GLfloat)max.x(), (GLfloat)min.y(), (GLfloat)max.z()));

        item.points.push_back(glm::vec3((GLfloat)max.x(), (GLfloat)min.y(), (GLfloat)min.z()));
        item.points.push_back(glm::vec3((GLfloat)max.x(), (GLfloat)max.y(), (GLfloat)min.z()));

        item.points.push_back(glm::vec3((GLfloat)min.x(), (GLfloat)max.y(), (GLfloat)min.z()));
        item.points.push_back(glm::vec3((GLfloat)max.x(), (GLfloat)max.y(), (GLfloat)min.z()));

        item.points.push_back(glm::vec3((GLfloat)max.x(), (GLfloat)max.y(), (GLfloat)min.z()));
        item.points.push_back(glm::vec3((GLfloat)max.x(), (GLfloat)max.y(), (GLfloat)max.z()));

        item.points.push_back(glm::vec3((GLfloat)max.x(), (GLfloat)min.y(), (GLfloat)max.z()));
        item.points.push_back(glm::vec3((GLfloat)max.x(), (GLfloat)max.y(), (GLfloat)max.z()));

        item.points.push_back(glm::vec3((GLfloat)min.x(), (GLfloat)max.y(), (GLfloat)max.z()));
        item.points.push_back(glm::vec3((GLfloat)max.x(), (GLfloat)max.y(), (GLfloat)max.z()));

        items.push_back(item);
#endif
    }
        
    return items;
}

}
