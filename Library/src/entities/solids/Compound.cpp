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
//  Copyright (c) 2017-2026 Patryk Cieslak. All rights reserved.
//

#include "entities/solids/Compound.h"

#include "core/SimulationApp.h"
#include "core/SimulationManager.h"
#include "utils/GeometryFileUtil.h"

namespace sf
{

Compound::Compound(const std::string& uniqueName, PhysicsSettings phy, std::unique_ptr<SolidEntity> firstExternalPart, const Transform& origin)
    : SolidEntity(uniqueName, phy, "", "", Scalar(-1))
{
    //All transformations are zero -> transforming the origin of a compound body doesn't make sense...
    phyMesh_ = NULL; // There is no single mesh
    volume_ = 0;
    mass_ = 0;
    Ipri_ = Vector3(0,0,0);
    displayInternals_ = false;
    
    AddExternalPart(std::move(firstExternalPart), origin);
}

void Compound::setDisplayInternalParts(bool enabled)
{
    displayInternals_ = enabled;
}

bool Compound::isDisplayingInternalParts()
{
    return displayInternals_;
}

Scalar Compound::getAugmentedMass() const
{
    return mass_ + aMass_.x();
}
        
Vector3 Compound::getAugmentedInertia() const
{
    return Ipri_;
}
    
Material Compound::getMaterial(size_t partId) const
{
    if(partId < parts_.size())
        return parts_[partId].solid->getMaterial();
    else
        return Material();
}

size_t Compound::getPartId(size_t collisionShapeId) const
{
    if(collisionShapeId < collisionPartId_.size())
        return collisionPartId_[collisionShapeId];
    else
        return 0;
}

const CompoundPart& Compound::getPart(size_t partId) const
{
    try
    {
        return parts_.at(partId);
    }
    catch(const std::out_of_range& e)
    {
        cCritical("Part of compound body with id %d does not exist!", partId);
    }
}
    
SolidType Compound::getSolidType()
{
    return SolidType::COMPOUND;
}

std::vector<Vector3> Compound::getMeshVertices() const
{
    std::vector<Vector3> pVert;
        
    for(size_t i=0; i<parts_.size(); ++i)
    {
        if(parts_[i].isExternal)
        {
            std::vector<Vector3> pPartVert = parts_[i].solid->getMeshVertices();
            Transform phyMeshTrans = parts_[i].origin * parts_[i].solid->getO2GTransform();
            for(size_t h=0; h < pPartVert.size(); ++h)
            {
                Vector3 v = phyMeshTrans * pPartVert[h];
                pVert.push_back(v);
            }
        }
    }
        
    return pVert;
}
    
void Compound::AddInternalPart(std::unique_ptr<SolidEntity> solid, const Transform& origin, bool alwaysVisible)
{
    if(solid != nullptr)
    {
        parts_.push_back({std::move(solid), origin, false, alwaysVisible});   
        RecalculatePhysicalProperties();
    }
}

void Compound::AddExternalPart(std::unique_ptr<SolidEntity> solid, const Transform& origin)
{
    if(solid != nullptr)
    {
        parts_.push_back({std::move(solid), origin, true, false});
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
    
    T_CG2O_ = T_CG2C_ = T_CG2G_ = Transform::getIdentity();
    P_CB_ = Vector3(0,0,0);
        
    for(size_t i=0; i<parts_.size(); ++i)
    {
        //Mechanical parameters
        compoundMass += parts_[i].solid->getMass();
        compoundAugmentedMass += parts_[i].isExternal ? parts_[i].solid->getAugmentedMass() : parts_[i].solid->getMass();
        compoundCG += (parts_[i].origin * parts_[i].solid->getCG2OTransform().inverse()).getOrigin() * parts_[i].solid->getMass();
        
        if(parts_[i].solid->isBuoyant())
        {
            compoundVolume += parts_[i].solid->getVolume();
            compoundCB += parts_[i].origin * parts_[i].solid->getCG2OTransform().inverse() * parts_[i].solid->getCB() * parts_[i].solid->getVolume();
        }

        if(parts_[i].isExternal)
            compoundSurface += parts_[i].solid->getSurface();
    }
    
    //Set transform origin
    compoundCG /= compoundMass;
    if(compoundVolume > Scalar(0)) compoundCB /= compoundVolume;
    T_CG2O_.setOrigin(-compoundCG);
    
    //2. Calculate compound inertia matrix
    Matrix3 I = Matrix3(0,0,0,0,0,0,0,0,0);
        
    for(unsigned int i=0; i<parts_.size(); ++i)
    {
        //Calculate inertia matrix 3x3 of solid in the compound body origin frame and transform to CB
        Vector3 solidPriInertia = parts_[i].isExternal ? parts_[i].solid->getAugmentedInertia() : parts_[i].solid->getInertia();
        Matrix3 solidInertia = Matrix3(solidPriInertia.x(), 0, 0, 0, solidPriInertia.y(), 0, 0, 0, solidPriInertia.z());
            
        //Rotate inertia tensor from part CG to compound CG
        Transform compToPart = T_CG2O_ * parts_[i].origin * parts_[i].solid->getCG2OTransform().inverse();
        solidInertia = compToPart.getBasis() * solidInertia * compToPart.getBasis().transpose();
            
        //Translate inertia tensor from part CG to compound CG
        Vector3 t = compToPart.getOrigin();
        Scalar m = parts_[i].isExternal ? parts_[i].solid->getAugmentedMass() : parts_[i].solid->getMass();
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
        T_CG2O_ = Transform(rotMat, Vector3(0,0,0)).inverse() * T_CG2O_;
    }
    
    T_CG2C_ = T_CG2G_ = T_CG2O_;
    
    //Move CB to compound CG frame
    P_CB_ = T_CG2O_ * compoundCB;
    
    mass_ = compoundMass;
    aMass_.setX(compoundAugmentedMass - compoundMass);
    aMass_.setY(compoundAugmentedMass - compoundMass);
    aMass_.setZ(compoundAugmentedMass - compoundMass);
    volume_ = compoundVolume;
    surface_ = compoundSurface;
    Ipri_ = compoundPriInertia;
}

btCollisionShape* Compound::BuildCollisionShape()
{
    //Build collision shape from external parts
    btCompoundShape* colShape = new btCompoundShape();
    for(size_t i = 0; i<parts_.size(); ++i)
    {
        if(parts_[i].isExternal)
        {
            Transform childTrans = parts_[i].origin * parts_[i].solid->getCG2OTransform().inverse() * parts_[i].solid->getCG2CTransform();
            btCollisionShape* partColShape = parts_[i].solid->BuildCollisionShape();
            colShape->addChildShape(childTrans, partColShape);
            collisionPartId_.push_back(i);
        }
    }
    return colShape;
}

void Compound::ComputeHydrodynamicForces(HydrodynamicsSettings settings, Ocean* ocn)
{
    if(phy_.mode != PhysicsMode::FLOATING && phy_.mode != PhysicsMode::SUBMERGED) return;
    
    auto points = submerged_.getDataAsPoints();
    if (points != nullptr)
        points->clear();

    BodyFluidPosition bf = CheckBodyFluidPosition(ocn);
     
    //If completely outside fluid just set all torques and forces to 0
    if(bf == BodyFluidPosition::OUTSIDE)
    {
        Fb_.setZero();
        Tb_.setZero();
        Fdq_.setZero();
        Tdq_.setZero();
        Fdf_.setZero();
        Tdf_.setZero();
        Swet_ = Scalar(0);
        Vsub_ = Scalar(0);
        return;
    }
    
    if(bf == BodyFluidPosition::INSIDE)
    {
        //Compute buoyancy based on CB position
        if(isBuoyant())
        {
            Fb_ = -volume_*ocn->getLiquid().density * SimulationApp::getApp()->getSimulationManager()->getGravity();
            Tb_ = (getCGTransform() * P_CB_ - getCGTransform().getOrigin()).cross(Fb_);
        }
        
        if(settings.dampingForces)
        {
            //Set zero
            Fdq_.setZero();
            Tdq_.setZero();
            Fdf_.setZero();
            Tdf_.setZero();
            
            //Get velocity data
            Vector3 v = getLinearVelocity();
            Vector3 omega = getAngularVelocity();
            
            //Create temporary vectors for summing
            Vector3 Fdqp(0,0,0);
            Vector3 Tdqp(0,0,0);
            Vector3 Fdfp(0,0,0);
            Vector3 Tdfp(0,0,0);
            
            for(size_t i=0; i<parts_.size(); ++i) //Go through all parts
                if(parts_[i].isExternal 
                    && (parts_[i].solid->getPhysicsMode() == PhysicsMode::SUBMERGED
                    || parts_[i].solid->getPhysicsMode() == PhysicsMode::FLOATING)) //Compute drag only for external parts
                {
                    Transform T_C_part = getOTransform() * parts_[i].origin * parts_[i].solid->getO2CTransform();
                    Transform T_O_part = getOTransform() * parts_[i].origin;

                    ComputeHydrodynamicForcesSubmerged(parts_[i].solid->getPhysicsMesh(), ocn, getCGTransform(), T_C_part, v, omega, Fdqp, Tdqp, Fdfp, Tdfp);
                    Vector3 Cd, Cf;
                    parts_[i].solid->getHydrodynamicCoefficients(Cd, Cf);
                    CorrectHydrodynamicForces(ocn, Fdqp, Tdqp, Fdfp, Tdfp, Cd, Cf, T_O_part);
                    Fdq_ += Fdqp;
                    Tdq_ += Tdqp;
                    Fdf_ += Fdfp;
                    Tdf_ += Tdfp;
                }
        }

        Swet_ = surface_;
        Vsub_ = volume_;
    }
    else //CROSSING FLUID SURFACE (compound body but not necessarily all parts!)
    {
        if(settings.reallisticBuoyancy || settings.dampingForces)
        {
            //Clear forces that will be recomputed
            if(settings.reallisticBuoyancy)
            {
                Fb_.setZero();
                Tb_.setZero();
            }
        
            if(settings.dampingForces)
            {
                Fdq_.setZero();
                Tdq_.setZero();
                Fdf_.setZero();
                Tdf_.setZero();
            }

            Swet_ = Scalar(0);
            Vsub_ = Scalar(0);
        
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

            for(size_t i=0; i<parts_.size(); ++i) //Loop through all parts
            {
                if(parts_[i].solid->getPhysicsMode() != PhysicsMode::SUBMERGED
                    && parts_[i].solid->getPhysicsMode() != PhysicsMode::FLOATING)
                    continue;

                Transform T_C_part = getOTransform() * parts_[i].origin * parts_[i].solid->getO2CTransform();
                Transform T_O_part = getOTransform() * parts_[i].origin;
                HydrodynamicsSettings pSettings = settings;
                pSettings.reallisticBuoyancy &= parts_[i].solid->isBuoyant();

                if(parts_[i].isExternal) //Compute buoyancy and drag
                {
                    ComputeHydrodynamicForcesSurface(pSettings, parts_[i].solid->getPhysicsMesh(), ocn, getCGTransform(), T_C_part, v, omega, Fbp, Tbp, Fdqp, Tdqp, Fdfp, Tdfp, Swetp, Vsubp, submerged_);
                    Vector3 Cd, Cf;
                    parts_[i].solid->getHydrodynamicCoefficients(Cd, Cf);
                    CorrectHydrodynamicForces(ocn, Fdqp, Tdqp, Fdfp, Tdfp, Cd, Cf, T_O_part);
                    Fb_ += Fbp;
                    Tb_ += Tbp;
                    Fdq_ += Fdqp;
                    Tdq_ += Tdqp;
                    Fdf_ += Fdfp;
                    Tdf_ += Tdfp;
                    Swet_ += Swetp;
                    Vsub_ += Vsubp;
                }
                else if(pSettings.reallisticBuoyancy) //Compute only buoyancy
                {
                    pSettings.dampingForces = false;
                    ComputeHydrodynamicForcesSurface(pSettings, parts_[i].solid->getPhysicsMesh(), ocn, getCGTransform(), T_C_part, v, omega, Fbp, Tbp, Fdqp, Tdqp, Fdfp, Tdfp, Swetp, Vsubp, submerged_);
                    Fb_ += Fbp;
                    Tb_ += Tbp;
                    Vsub_ += Vsubp;
                }
            }
        }
    }
}

void Compound::ComputeAerodynamicForces(Atmosphere* atm)
{
    if(phy_.mode != PhysicsMode::AERODYNAMIC) return;
    
    //Set zero
    Fda_.setZero();
    Tda_.setZero();
            
    //Get velocity data
    Vector3 v = getLinearVelocity();
    Vector3 omega = getAngularVelocity();
            
    //Create temporary vectors for summing
    Vector3 Fdap(0,0,0);
    Vector3 Tdap(0,0,0);
            
    for(size_t i=0; i<parts_.size(); ++i) //Go through all parts
        if(parts_[i].isExternal) //Compute drag only for external parts
        {
            Transform T_C_part = getOTransform() * parts_[i].origin * parts_[i].solid->getO2CTransform();
            SolidEntity::ComputeAerodynamicForces(parts_[i].solid->getPhysicsMesh(), atm, getCGTransform(), T_C_part, v, omega, Fdap, Tdap);
            parts_[i].solid->CorrectAerodynamicForces(atm, Fdap, Tdap);
            Fda_ += Fdap;
            Tda_ += Tdap;
        }
}

void Compound::BuildGraphicalObject()
{
    for(unsigned int i=0; i<parts_.size(); ++i)
        parts_[i].solid->BuildGraphicalObject();
}

std::vector<Renderable> Compound::Render(size_t partId)
{
    std::vector<Renderable> items(0);
    Transform oCompoundTrans = getOTransform();

    try
    {
        Renderable item1;

        if((parts_.at(partId).isExternal && !displayInternals_)  
            || (!parts_.at(partId).isExternal && displayInternals_)
            || (parts_.at(partId).alwaysVisible))
        {
            item1.type = RenderableType::SOLID;
            item1.materialName = parts_.at(partId).solid->getMaterial().name;
                
            if(dm_ == DisplayMode::GRAPHICAL)
            {
                Transform oTrans = oCompoundTrans * parts_.at(partId).origin * parts_.at(partId).solid->getO2GTransform();
                item1.objectId = parts_.at(partId).solid->getGraphicalObject();
                item1.lookId = parts_.at(partId).solid->getLook();
                item1.model = glMatrixFromTransform(oTrans);
                item1.cor = glVectorFromVector(getCGTransform().getOrigin());
                item1.vel = glVectorFromVector(getLinearVelocity());
                item1.avel = glVectorFromVector(getAngularVelocity());
                items.push_back(item1);
            }
            else if(dm_ == DisplayMode::PHYSICAL)
            {
                Transform oTrans = oCompoundTrans * parts_.at(partId).origin * parts_.at(partId).solid->getO2CTransform();
                item1.objectId = parts_.at(partId).solid->getPhysicalObject();
                item1.lookId = -1;
                item1.model = glMatrixFromTransform(oTrans);
                item1.cor = glVectorFromVector(getCGTransform().getOrigin());
                item1.vel = glVectorFromVector(getLinearVelocity());
                item1.avel = glVectorFromVector(getAngularVelocity());
                items.push_back(item1);
            }
        }

#ifndef DEBUG_HYDRO
        GeometryApproxType atype;
        std::vector<Scalar> aparams;
        parts_.at(partId).solid->getGeometryApprox(atype, aparams);

        Renderable item2;
        item2.model = glMatrixFromTransform(oCompoundTrans * parts_.at(partId).origin * parts_.at(partId).solid->getO2HTransform());
        item2.data = std::make_shared<std::vector<glm::vec3>>();

        switch(atype)
        {
            case  GeometryApproxType::AUTO:
                break;
            
            case  GeometryApproxType::SPHERE:
                item2.type = RenderableType::HYDRO_ELLIPSOID;
                item2.getDataAsPoints()->push_back(glm::vec3((GLfloat)aparams[0], (GLfloat)aparams[0], (GLfloat)aparams[0]));
                items.push_back(item2);
                break;
            
            case  GeometryApproxType::CYLINDER:
                item2.type = RenderableType::HYDRO_CYLINDER;
                item2.getDataAsPoints()->push_back(glm::vec3((GLfloat)aparams[0], (GLfloat)aparams[0], (GLfloat)aparams[1]));
                items.push_back(item2);
                break;
            
            case  GeometryApproxType::ELLIPSOID:
                item2.type = RenderableType::HYDRO_ELLIPSOID;
                item2.getDataAsPoints()->push_back(glm::vec3((GLfloat)aparams[0], (GLfloat)aparams[1], (GLfloat)aparams[2]));
                items.push_back(item2);
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
        Renderable item1;
        item1.type = RenderableType::SOLID_CS;
        item1.model = glMatrixFromTransform(getCGTransform());
        items.push_back(item1);
        
        Vector3 cbWorld = getCGTransform() * P_CB_;
        Renderable item2;
        item2.type = RenderableType::HYDRO_CS;
        item2.model = glMatrixFromTransform(Transform(Quaternion::getIdentity(), cbWorld));
        item2.data = std::make_shared<std::vector<glm::vec3>>();
        item2.getDataAsPoints()->push_back(glm::vec3(volume_, volume_, volume_));
        items.push_back(item2);
        
        //Parts
        for(size_t i=0; i<parts_.size(); ++i)
        {
            std::vector<Renderable> partItems = Render(i);
            items.insert(items.end(), partItems.begin(), partItems.end());
        }

        //Forces
        Vector3 cg = getCGTransform().getOrigin();
        glm::vec3 cgv((GLfloat)cg.x(), (GLfloat)cg.y(), (GLfloat)cg.z());

        //--- Buoyancy
        Renderable item3;
        item3.type = RenderableType::FORCE_BUOYANCY;
        item3.model = glm::mat4(1.f);
        item3.data = std::make_shared<std::vector<glm::vec3>>();
        item3.getDataAsPoints()->push_back(cgv);
        item3.getDataAsPoints()->push_back(cgv + glm::vec3((GLfloat)Fb_.x(), (GLfloat)Fb_.y(), (GLfloat)Fb_.z())/1000.f);
        items.push_back(item3);
        
        //--- Linear drag
        Renderable item4;
        item4.type = RenderableType::FORCE_LINEAR_DRAG;
        item4.model = glm::mat4(1.f);
        item4.data = std::make_shared<std::vector<glm::vec3>>();
        item4.getDataAsPoints()->push_back(cgv);
        item4.getDataAsPoints()->push_back(cgv + glm::vec3((GLfloat)Fdf_.x(), (GLfloat)Fdf_.y(), (GLfloat)Fdf_.z()));
        items.push_back(item4);
        
        //--- Quadratic drag
        Renderable item5;
        item5.type = RenderableType::FORCE_QUADRATIC_DRAG;
        item5.model = glm::mat4(1.f);
        item5.data = std::make_shared<std::vector<glm::vec3>>();
        item5.getDataAsPoints()->push_back(cgv);
        item5.getDataAsPoints()->push_back(cgv + glm::vec3((GLfloat)Fdq_.x(), (GLfloat)Fdq_.y(), (GLfloat)Fdq_.z()));
        items.push_back(item5);

#ifdef DEBUG_HYDRO
        items.push_back(submerged);

        Renderable debugItem;
        debugItem.type = RenderableType::HYDRO_LINES;
        debugItem.model = glm::mat4(1.f);
        debugItem.data = std::make_shared<std::vector<glm::vec3>>();
        auto points = debugItem.getDataAsPoints();
        
        Vector3 min, max;
        getAABB(min, max);

        points->push_back(glm::vec3((GLfloat)min.x(), (GLfloat)min.y(), (GLfloat)min.z()));
        points->push_back(glm::vec3((GLfloat)max.x(), (GLfloat)min.y(), (GLfloat)min.z()));

        points->push_back(glm::vec3((GLfloat)min.x(), (GLfloat)min.y(), (GLfloat)min.z()));
        points->push_back(glm::vec3((GLfloat)min.x(), (GLfloat)max.y(), (GLfloat)min.z()));

        points->push_back(glm::vec3((GLfloat)min.x(), (GLfloat)min.y(), (GLfloat)min.z()));
        points->push_back(glm::vec3((GLfloat)min.x(), (GLfloat)min.y(), (GLfloat)max.z()));

        points->push_back(glm::vec3((GLfloat)min.x(), (GLfloat)max.y(), (GLfloat)min.z()));
        points->push_back(glm::vec3((GLfloat)min.x(), (GLfloat)max.y(), (GLfloat)max.z()));

        points->push_back(glm::vec3((GLfloat)min.x(), (GLfloat)min.y(), (GLfloat)max.z()));
        points->push_back(glm::vec3((GLfloat)min.x(), (GLfloat)max.y(), (GLfloat)max.z()));

        points->push_back(glm::vec3((GLfloat)min.x(), (GLfloat)min.y(), (GLfloat)max.z()));
        points->push_back(glm::vec3((GLfloat)max.x(), (GLfloat)min.y(), (GLfloat)max.z()));

        points->push_back(glm::vec3((GLfloat)max.x(), (GLfloat)min.y(), (GLfloat)min.z()));
        points->push_back(glm::vec3((GLfloat)max.x(), (GLfloat)min.y(), (GLfloat)max.z()));

        points->push_back(glm::vec3((GLfloat)max.x(), (GLfloat)min.y(), (GLfloat)min.z()));
        points->push_back(glm::vec3((GLfloat)max.x(), (GLfloat)max.y(), (GLfloat)min.z()));

        points->push_back(glm::vec3((GLfloat)min.x(), (GLfloat)max.y(), (GLfloat)min.z()));
        points->push_back(glm::vec3((GLfloat)max.x(), (GLfloat)max.y(), (GLfloat)min.z()));

        points->push_back(glm::vec3((GLfloat)max.x(), (GLfloat)max.y(), (GLfloat)min.z()));
        points->push_back(glm::vec3((GLfloat)max.x(), (GLfloat)max.y(), (GLfloat)max.z()));

        points->push_back(glm::vec3((GLfloat)max.x(), (GLfloat)min.y(), (GLfloat)max.z()));
        points->push_back(glm::vec3((GLfloat)max.x(), (GLfloat)max.y(), (GLfloat)max.z()));

        points->push_back(glm::vec3((GLfloat)min.x(), (GLfloat)max.y(), (GLfloat)max.z()));
        points->push_back(glm::vec3((GLfloat)max.x(), (GLfloat)max.y(), (GLfloat)max.z()));

        items.push_back(debugItem);
#endif
    }
        
    return items;
}

}
