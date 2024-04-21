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
//  SolidEntity.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 29/12/12.
//  Copyright (c) 2012-2024 Patryk Cieslak. All rights reserved.
//

#include "entities/SolidEntity.h"

#include "core/GraphicalSimulationApp.h"
#include "core/SimulationManager.h"
#include "graphics/OpenGLPipeline.h"
#include "graphics/OpenGLContent.h"
#include "utils/SystemUtil.hpp"
#include "entities/forcefields/Ocean.h"
#include "entities/forcefields/Atmosphere.h"
#include <iostream>
#include <algorithm>

namespace sf
{

SolidEntity::SolidEntity(std::string uniqueName, BodyPhysicsSettings phy, std::string material, std::string look, Scalar thickness) 
    : MovingEntity(uniqueName, material, look), phy(phy), thick(thickness)
{
    //Check if ocean is enabled and change physics mode accordingly
    if((phy.mode == BodyPhysicsMode::SUBMERGED || phy.mode == BodyPhysicsMode::FLOATING) && !SimulationApp::getApp()->getSimulationManager()->isOceanEnabled())
        this->phy.mode = BodyPhysicsMode::SURFACE;
    
    //Get material
    mat = SimulationApp::getApp()->getSimulationManager()->getMaterialManager()->getMaterial(material);
    
    //Get Look
    if(SimulationApp::getApp()->hasGraphics())
        lookId = ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->getLookId(look);
    else
        lookId = -1;
    
    //Set transformations to identity
    T_O2G = I4();
    T_O2C = I4();
    T_O2H = I4();
    T_CG2C = I4();
    T_CG2G = I4();
    T_CG2O = I4();
    P_CB.setZero();
    
    //Set properties
    mass = Scalar(0);
    aMass.setZero();
    surface = Scalar(0);
    aI.setZero();
    Ipri.setZero();
    contactK = Scalar(-1);
    contactD = Scalar(0);
    volume = Scalar(0);
    fdApproxType =  GeometryApproxType::AUTO;
    fdApproxParams = std::vector<Scalar>(0);
    fdCd = Vector3(-1.0, -1.0, -1.0);
    fdCf = Vector3(-1.0, -1.0, -1.0);
    T_CG2H = Transform::getIdentity();
    
    //Set vectors to zero
    Fb.setZero();
    Tb.setZero();
    Fdq.setZero();
    Tdq.setZero();
    Fdf.setZero();
    Tdf.setZero();
    Fda.setZero();
    Tda.setZero();
    Swet = Scalar(0);
    Vsub = Scalar(0);
    lastV.setZero();
    lastOmega.setZero();
    linearAcc.setZero();
    angularAcc.setZero();
    
    //Set pointers
    multibodyCollider = nullptr;
    phyMesh = nullptr;
    graObjectId = -1;
    phyObjectId = -1;
    dm = DisplayMode::GRAPHICAL;
    submerged.type = RenderableType::HYDRO_LINES;
    submerged.model = glm::mat4(1.f);
}

SolidEntity::~SolidEntity()
{
    if(phyMesh != nullptr) 
        delete phyMesh;
}

EntityType SolidEntity::getType() const
{
    return EntityType::SOLID;
}

void SolidEntity::ScalePhysicalPropertiesToArbitraryMass(Scalar mass)
{
    if(rigidBody != nullptr || multibodyCollider != nullptr)
    {
        cWarning("Physical properties of bodies cannot be changed after adding to simulation!");
        return;
    }
    
    Scalar oldMass = this->mass;
    this->mass = mass;
    Ipri *= this->mass/oldMass;
}

void SolidEntity::SetArbitraryPhysicalProperties(Scalar mass, const Vector3& inertia, const Transform& CG)
{
    if(rigidBody != nullptr || multibodyCollider != nullptr)
    {
        cWarning("Physical properties of bodies cannot be changed after adding to simulation!");
        return;
    }
    
    this->mass = mass;
    Ipri = inertia;
    Transform T_CG_old_new = T_CG2O * CG; //Transform from old CG to new CG
    T_CG2O = CG.inverse(); //Set new CG in body origin frame
    T_CG2C = T_CG2O * T_O2C;
    T_CG2G = T_CG2O * T_O2G;
    T_CG2H = T_CG_old_new.inverse() * T_CG2H;
    P_CB = T_CG_old_new.inverse() * P_CB;
}

void SolidEntity::SetContactProperties(bool soft, Scalar stiffness, Scalar damping)
{
    if(soft)
    {
        contactK = stiffness > Scalar(0) ? stiffness : Scalar(1000);
        contactD = damping >= Scalar(0) ? damping : Scalar(0); 
    }
    else
    {
        contactK = Scalar(-1);
        contactD = Scalar(0);
    }
    
    if(rigidBody != nullptr)
    {
        if(soft)
            rigidBody->setContactStiffnessAndDamping(contactK, contactD);
        else
        {
            int cflags = rigidBody->getCollisionFlags();
            cflags &= ~(btCollisionObject::CollisionFlags::CF_HAS_CONTACT_STIFFNESS_DAMPING);
            rigidBody->setCollisionFlags(cflags);
        }
    }
    else if(multibodyCollider != nullptr)
    {
        if(soft)
            multibodyCollider->setContactStiffnessAndDamping(contactK, contactD);
        else
        {
            int cflags = multibodyCollider->getCollisionFlags();
            cflags &= ~(btCollisionObject::CollisionFlags::CF_HAS_CONTACT_STIFFNESS_DAMPING);
            multibodyCollider->setCollisionFlags(cflags);
        }
    }
}

void SolidEntity::SetHydrodynamicCoefficients(const Vector3& Cd, const Vector3& Cf)
{
    if(Cd.getX() >= Scalar(0) && Cd.getY() >= Scalar(0) && Cd.getZ() >= Scalar(0))
        fdCd = Cd;
    if(Cf.getX() >= Scalar(0) && Cf.getY() >= Scalar(0) && Cf.getZ() >= Scalar(0))
        fdCf = Cf;
}

int SolidEntity::getPhysicalObject() const
{
    return phyObjectId;
}

bool SolidEntity::isBuoyant() const
{
    return (phy.mode == BodyPhysicsMode::SUBMERGED || phy.mode == BodyPhysicsMode::FLOATING) && phy.buoyancy;
}
    
BodyPhysicsMode SolidEntity::getBodyPhysicsMode() const
{
    return phy.mode;
}

void SolidEntity::getAABB(Vector3& min, Vector3& max)
{
    if(rigidBody != nullptr)
        rigidBody->getAabb(min, max);
    else if(multibodyCollider != nullptr)
        multibodyCollider->getCollisionShape()->getAabb(getCGTransform(), min, max);
    else
    {
        min.setValue(BT_LARGE_FLOAT, BT_LARGE_FLOAT, BT_LARGE_FLOAT);
        max.setValue(-BT_LARGE_FLOAT, -BT_LARGE_FLOAT, -BT_LARGE_FLOAT);
    }
}

std::vector<Renderable> SolidEntity::Render()
{
    std::vector<Renderable> items(0);
    
    if( (rigidBody != nullptr || multibodyCollider != nullptr)  && isRenderable() )
    {
        Renderable item;
        item.type = RenderableType::SOLID;
        item.materialName = mat.name;
        
        if(dm == DisplayMode::GRAPHICAL && graObjectId >= 0)
        {
            item.objectId = graObjectId;
            item.lookId = lookId;
            item.model = glMatrixFromTransform(getGTransform());
            item.vel = glVectorFromVector(getLinearVelocity());
            item.avel = glVectorFromVector(getAngularVelocity());
            items.push_back(item);
        }
        else if(dm == DisplayMode::PHYSICAL && phyObjectId >= 0)
        {
            item.objectId = phyObjectId;
            item.lookId = -1;
            item.model = glMatrixFromTransform(getCTransform());
            item.vel = glVectorFromVector(getLinearVelocity());
            item.avel = glVectorFromVector(getAngularVelocity());
            items.push_back(item);
        }
        
        item.type = RenderableType::SOLID_CS;
        item.model = glMatrixFromTransform(getCGTransform());
        items.push_back(item);
        
        //Hydrodynamics
        Vector3 cbWorld = getCGTransform() * P_CB;
        item.type = RenderableType::HYDRO_CS;
        item.model = glMatrixFromTransform(Transform(Quaternion::getIdentity(), cbWorld));
        items.push_back(item);

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

        //Surface crossing debug
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
#else
        //Geometry approximation
        switch(fdApproxType)
        {
            case  GeometryApproxType::AUTO:
                break;
                
            case  GeometryApproxType::SPHERE:
                item.type = RenderableType::HYDRO_ELLIPSOID;
                item.model = glMatrixFromTransform(getHTransform());
                item.points.push_back(glm::vec3((GLfloat)fdApproxParams[0], (GLfloat)fdApproxParams[0], (GLfloat)fdApproxParams[0]));
                items.push_back(item);
                break;
                
            case  GeometryApproxType::CYLINDER:
                item.type = RenderableType::HYDRO_CYLINDER;
                item.model = glMatrixFromTransform(getHTransform());
                item.points.push_back(glm::vec3((GLfloat)fdApproxParams[0], (GLfloat)fdApproxParams[0], (GLfloat)fdApproxParams[1]));
                items.push_back(item);
                break;
                
            case  GeometryApproxType::ELLIPSOID:
                item.type = RenderableType::HYDRO_ELLIPSOID;
                item.model = glMatrixFromTransform(getHTransform());
                item.points.push_back(glm::vec3((GLfloat)fdApproxParams[0], (GLfloat)fdApproxParams[1], (GLfloat)fdApproxParams[2]));
                items.push_back(item);
                break;
        }
#endif
    }
    
    return items;
}
    
Transform SolidEntity::getCG2GTransform() const
{
    return T_CG2G;
}
    
Transform SolidEntity::getCG2CTransform() const
{
    return T_CG2C;
}
    
Transform SolidEntity::getCG2OTransform() const
{
    return T_CG2O;
}
    
Vector3 SolidEntity::getCB() const
{
    return P_CB;
}

Transform SolidEntity::getCGTransform() const
{
    if(rigidBody != nullptr)
    {
        Transform trans;
        rigidBody->getMotionState()->getWorldTransform(trans);
        return trans;
    }
    else if(multibodyCollider != nullptr)
    {
        return multibodyCollider->getWorldTransform();
    }
    else
        return Transform::getIdentity();
}

Transform SolidEntity::getO2CTransform() const
{
    return T_O2C;
}
    
Transform SolidEntity::getO2GTransform() const
{
    return T_O2G;
}

Transform SolidEntity::getO2HTransform() const
{
    return T_O2H;
}
    
Transform SolidEntity::getGTransform() const
{
    return getCGTransform() * T_CG2G;
}
    
Transform SolidEntity::getCTransform() const
{
    return getCGTransform() * T_CG2C;
}
    
Transform SolidEntity::getHTransform() const
{
    return getCGTransform() * T_CG2H;
}
    
Transform SolidEntity::getOTransform() const
{
    return getCGTransform() * T_CG2O;
}

void SolidEntity::setCGTransform(const Transform& trans)
{
    if(rigidBody != nullptr)
    {
        rigidBody->getMotionState()->setWorldTransform(trans);
    }
    else if(multibodyCollider != nullptr)
    {
        multibodyCollider->setWorldTransform(trans);
    }
}

Vector3 SolidEntity::getLinearVelocity() const
{
    if(rigidBody != nullptr)
    {
        return rigidBody->getLinearVelocity();
    }
    else if(multibodyCollider != nullptr)
    {
        //Get multibody and link id
        btMultiBody* multiBody = multibodyCollider->m_multiBody;
        int index = multibodyCollider->m_link;
        
        //Start with base velocity
        Vector3 linVelocity = multiBody->getBaseVel(); //Global
        Vector3 angVelocity = multiBody->getBaseOmega(); //Global
        
        if(index >= 0) //If collider is not base
        {
            for(int i = 0; i <= index; ++i) //Accumulate velocity resulting from joints
            {
                //Add velocity resulting from rotation of previous links
                linVelocity += angVelocity.cross(multiBody->localDirToWorld(i, multiBody->getRVector(i)));
                
                if(multiBody->getLink(i).m_jointType == btMultibodyLink::ePrismatic) //Just add linear velocity
                {
                    Vector3 axis = multiBody->getLink(i).getAxisBottom(0); //Local axis
                    Vector3 vel = multiBody->getJointVel(i) * axis; //Local velocity
                    Vector3 gvel = multiBody->localDirToWorld(i, vel); //Global velocity
                    linVelocity += gvel;
                }
                else if(multiBody->getLink(i).m_jointType == btMultibodyLink::eRevolute) //Add linear velocity due to rotation
                {
                    //Vector3 axis = multiBody->getLink(i).getAxisBottom(0); //Local linear motion
                    //Vector3 vel = multiBody->getJointVel(i) * axis; //Local velocity
                    
                    Vector3 axis = multiBody->getLink(i).getAxisTop(0); //Axis of joint
                    Vector3 aVel = multiBody->getJointVel(i) * axis;
                    Vector3 vel = aVel.cross(multiBody->getLink(i).m_dVector); //Local velocity
                    Vector3 gvel = multiBody->localDirToWorld(i, vel); //Global linear velocity
                    linVelocity += gvel;
                    angVelocity += multiBody->localDirToWorld(i, aVel); //Global angular velocity
                }
            }
        }
        
        return linVelocity;
    }
    else
        return Vector3(0,0,0);
}

Vector3 SolidEntity::getAngularVelocity() const
{
    if(rigidBody != nullptr)
    {
        return rigidBody->getAngularVelocity();
    }
    else if(multibodyCollider != nullptr)
    {
        //Get multibody and link id
        btMultiBody* multiBody = multibodyCollider->m_multiBody;
        int index = multibodyCollider->m_link;
        
        //Start with base velocity
        Vector3 angVelocity = multiBody->getBaseOmega(); //Global
        
        if(index >= 0)
        {
            for(int i = 0; i <= index; ++i) //Accumulate velocity resulting from joints
                if(multiBody->getLink(i).m_jointType == btMultibodyLink::eRevolute) //Only revolute joints can change angular velocity
                {
                    Vector3 axis = multiBody->getLink(i).getAxisTop(0); //Local axis
                    Vector3 vel = multiBody->getJointVel(i) * axis; //Local velocity
                    Vector3 gvel = multiBody->localDirToWorld(i, vel); //Global velocity
                    angVelocity += gvel;
                }
        }
        
        return angVelocity;
    }
    else
        return Vector3(0,0,0);
}

Vector3 SolidEntity::getLinearVelocityInLocalPoint(const Vector3& relPos) const
{
    if(rigidBody != nullptr)
    {
        return rigidBody->getVelocityInLocalPoint(relPos);
    }
    else if(multibodyCollider != nullptr)
    {
        return getLinearVelocity() + getAngularVelocity().cross(relPos);
    }
    else
        return Vector3(0,0,0);
}

Vector3 SolidEntity::getAppliedForce()
{
    if(rigidBody != nullptr)
    {
        return rigidBody->getTotalForce();
    }
    else if(multibodyCollider != nullptr)
    {
        btMultiBody* multiBody = multibodyCollider->m_multiBody;
        int index = multibodyCollider->m_link;

        if(index >= 0)
            return multiBody->getLinkForce(index);
        else
            return multiBody->getBaseForce();
    }
    else
        return V0();
}

void SolidEntity::getHydrodynamicForces(Vector3& Fb, Vector3& Tb, Vector3& Fd, Vector3& Td, Vector3& Ff, Vector3& Tf)
{
    Fb = this->Fb;
    Tb = this->Tb;
    Fd = this->Fdq;
    Td = this->Tdq;
    Ff = this->Fdf;
    Tf = this->Tdf;
}

void SolidEntity::getHydrodynamicCoefficients(Vector3& Cd, Vector3& Cf) const
{
    Cd = fdCd;
    Cf = fdCf;
}

Scalar SolidEntity::getWettedSurface() const
{
    return Swet;
}

Scalar SolidEntity::getSubmergedVolume() const
{
    return Vsub;
}

Vector3 SolidEntity::getLinearAcceleration() const
{
    return linearAcc;
}

Vector3 SolidEntity::getAngularAcceleration() const
{
    return angularAcc;
}

Scalar SolidEntity::getVolume() const
{
    return volume;
}
    
Vector3 SolidEntity::getInertia() const
{
    return Ipri;
}

Scalar SolidEntity::getMass() const
{
    return mass;
}

Scalar SolidEntity::getSurface() const
{
    return surface;
}

Vector3 SolidEntity::getAddedMass() const
{
    return aMass;
}

Vector3 SolidEntity::getAddedInertia() const
{
    return aI;
}

Scalar SolidEntity::getAugmentedMass() const
{
    if(phy.mode == BodyPhysicsMode::SUBMERGED)
        return mass + (aMass.x() + aMass.y() + aMass.z())/Scalar(3);
    else
        return mass;
}

Vector3 SolidEntity::getAugmentedInertia() const
{
    if(phy.mode == BodyPhysicsMode::SUBMERGED)
        return Ipri + aI;
    else
        return Ipri;
}

void SolidEntity::getGeometryApprox(GeometryApproxType& type, std::vector<Scalar>& params) const
{
    type = fdApproxType;
    params = fdApproxParams;
}

const Mesh* SolidEntity::getPhysicsMesh()
{
    return phyMesh;
}

std::vector<Vector3>* SolidEntity::getMeshVertices() const
{
    std::vector<Vector3>* vertices = new std::vector<Vector3>(0);
    if(phyMesh != nullptr)
    {
        for(size_t i=0; i<phyMesh->getNumOfVertices(); ++i)
        {
            glm::vec3 pos = phyMesh->getVertexPos(i);
            vertices->push_back(Vector3(pos.x, pos.y, pos.z));
        }
    }
    return vertices;
}

void SolidEntity::ComputeFluidDynamicsApprox(GeometryApproxType t)
{
    switch(t)
    {
        case  GeometryApproxType::SPHERE:
            ComputeSphericalApprox();
            break;
            
        case  GeometryApproxType::CYLINDER:
            ComputeCylindricalApprox();
            break;
            
        case  GeometryApproxType::AUTO:
        case  GeometryApproxType::ELLIPSOID:
            ComputeEllipsoidalApprox();
            break;
    }
#ifdef DEBUG
    cInfo("Added mass: %lf, %lf, %lf, %lf, %lf, %lf", aMass.x(), aMass.y(), aMass.z(), aI.x(), aI.y(), aI.z());
#endif
}

void SolidEntity::ComputeSphericalApprox()
{
    std::vector<Vector3>* x = getMeshVertices();
    if(x->size() < 2)
        return;
    for(size_t i=0; i<x->size(); ++i)
        x->at(i) = T_CG2C * x->at(i) - P_CB;
    
    Scalar r(0);
    for(size_t i=0; i<x->size(); ++i)
    {
        Scalar rc = x->at(i).length2();
        if(rc > r)
            r = rc;
    }
    
    fdApproxType =  GeometryApproxType::SPHERE;
    fdApproxParams.resize(1);
    fdApproxParams[0] = btSqrt(r);
    
    Scalar rho = Scalar(1000);
    Ocean* ocn;
    if((ocn = SimulationApp::getApp()->getSimulationManager()->getOcean()) != nullptr)
        rho = ocn->getLiquid().density;

    Scalar m = Scalar(2)*M_PI*rho*r*r*r/Scalar(3);
    aMass = Vector3(m,m,m);
    aI = V0();
    
    //Set transform with respect to geometry
    Transform sphereTransform = I4();
    sphereTransform.setOrigin(P_CB);
    T_CG2H = sphereTransform;

    Vector3 Cd(1,1,1);
    SetHydrodynamicCoefficients(Cd, Scalar(0.1)*Cd); //No need to trasform (all equal)

    delete x;
}

void SolidEntity::ComputeCylindricalApprox()
{
    std::vector<Vector3>* x = getMeshVertices();
    if(x->size() < 2)
        return;
    for(size_t i=0; i<x->size(); ++i)
        x->at(i) = T_CG2C * x->at(i) - P_CB;
        
    //Radius
    Scalar r[3] = {0,0,0};
    for(size_t i=0; i<x->size(); ++i)
    {
        Scalar d;
        
        //X
        d = btSqrt(x->at(i).y()*x->at(i).y() + x->at(i).z()*x->at(i).z());
        r[0] = d > r[0] ? d : r[0];
        
        //Y
        d = btSqrt(x->at(i).x()*x->at(i).x() + x->at(i).z()*x->at(i).z());
        r[1] = d > r[1] ? d : r[1];
        
        //Z
        d = btSqrt(x->at(i).x()*x->at(i).x() + x->at(i).y()*x->at(i).y());
        r[2] = d > r[2] ? d : r[2];
    }
    
    unsigned int axis = 0;
    
    if(r[0] <= r[1] && r[0] <= r[2]) //X cylinder
        axis = 0;
    else if(r[1] <= r[0] && r[1] <= r[2]) //Y cylinder
        axis = 1;
    else //Z cylinder
        axis = 2;
    
    Scalar l_2 = 0;
    for(size_t i=0; i<x->size(); ++i)
    {
        Scalar d = btFabs(x->at(i).m_floats[axis]);
        l_2 = d > l_2 ? d : l_2;
    }
    
    fdApproxType =  GeometryApproxType::CYLINDER;
    fdApproxParams.resize(2);
    
    if(axis == 0) //X axis
    {
        fdApproxParams[0] = r[0]; 
        fdApproxParams[1] = l_2*Scalar(2);
        T_CG2H = Transform(Quaternion(0,M_PI_2,0), P_CB);
    }
    else if(axis == 1) //Y axis
    {
        fdApproxParams[0] = r[1];
        fdApproxParams[1] = l_2*Scalar(2);
        T_CG2H = Transform(Quaternion(0,0,M_PI_2), P_CB);
    }
    else
    {
        fdApproxParams[0] = r[2];
        fdApproxParams[1] = l_2*Scalar(2);
        T_CG2H = Transform(IQ(), P_CB);
    }
    
    //Added mass and inertia
    Scalar rho = Scalar(1000);
    Ocean* ocn;
    if((ocn = SimulationApp::getApp()->getSimulationManager()->getOcean()) != nullptr)
        rho = ocn->getLiquid().density;

    Scalar m1 = rho*M_PI*fdApproxParams[0]*fdApproxParams[0]; //Parallel to axis
    Scalar m2 = rho*M_PI*fdApproxParams[0]*fdApproxParams[0]*fdApproxParams[1]; //Perpendicular to axis
    Scalar I1 = Scalar(0);
    Scalar I2 = Scalar(1)/Scalar(12)*M_PI*rho*fdApproxParams[1]*fdApproxParams[1]*btPow(fdApproxParams[0], Scalar(3));
    
    aMass = T_CG2H.getBasis() * Vector3(m2, m2, m1);
    aMass = Vector3(btFabs(aMass.getX()), btFabs(aMass.getY()), btFabs(aMass.getZ()));
    aI = T_CG2H.getBasis() * Vector3(I2, I2, I1);
    aI = Vector3(btFabs(aI.getX()), btFabs(aI.getY()), btFabs(aI.getZ()));

    Vector3 Cd(0.5, 0.5, 1.0);
    Cd = T_CG2O.getBasis().inverse() * T_CG2H.getBasis() * Cd; // To origin frame
    Cd = Vector3(btFabs(Cd.getX()), btFabs(Cd.getY()), btFabs(Cd.getZ()));
    SetHydrodynamicCoefficients(Cd, Scalar(0.1)*Cd);

    delete x;
}

void SolidEntity::ComputeEllipsoidalApprox()
{
#ifdef DEBUG
    cInfo("---- Computing ellipsoidal approximation of geometry for %s ----", getName().c_str());
#endif
    std::vector<Vector3>* x = getMeshVertices();
    if(x->size() < 2)
        return;
    for(size_t i=0; i<x->size(); ++i)
        x->at(i) = T_CG2C * x->at(i) - P_CB;
    
    //Initial volume approximation algorithm
    std::vector<Vector3> x0;
    for(size_t k=0; k<3; ++k) //3 dimensions
    {
        std::vector<Scalar> x_k(x->size());
        for(size_t i=0; i<x->size(); ++i)
            x_k[i] = x->at(i).m_floats[k];

        auto result = std::minmax_element(x_k.begin(), x_k.end());
        x0.push_back(x->at(result.first - x_k.begin()));
        x0.push_back(x->at(result.second - x_k.begin()));
    }

    //Minimum-volume enclosing axis-aligned ellipsoid algorithm
    std::vector<Scalar> sigma(x->size());
    for(size_t i=0; i<x->size(); ++i)
    {
        std::vector<Vector3>::iterator it;
        it = std::find(x0.begin(), x0.end(), x->at(i));
        if(it != x0.end())
            sigma[i] = Scalar(1)/Scalar(x0.size());
        else
            sigma[i] = Scalar(0);
    }
    
    int k = 0;
    
    auto u = [](auto j, auto& x, auto& sigma)
    {
        auto sum = Scalar(0);
        for(size_t i=0; i<x->size(); ++i)
            sum += sigma[i] * x->at(i).m_floats[j] * x->at(i).m_floats[j]; 
        return sum;
    };
    
    auto v = [](auto j, auto& x, auto& sigma)
    {
        auto sum = Scalar(0);
        for(size_t i=0; i<x->size(); ++i)
            sum += sigma[i] * x->at(i).m_floats[j]; 
        return sum;	
    };
    
    /*
    auto lambda = [&x, &sigma, &u, &v](int i)
    {
        auto sum = Scalar(0);
        for(int j=0; j<3; ++j)
        {
            auto vj = v(j, x, sigma);
            auto uj = u(j, x, sigma);
            sum += (x[i].m_floats[j] - vj)*(x[i].m_floats[j] - vj)/(3*(uj - vj*vj));
        }
        return sum;
    };
    
    Scalar epsilon0 = btPow(Scalar(1) + Scalar(0.1), Scalar(2)/Scalar(3)) - Scalar(1);
    Scalar epsilon;
    
    while(1)
    {
        std::vector<Scalar> xk(x.size());
        for(size_t i=0; i<xk.size(); ++i) xk[i] = lambda(i);
    
        std::vector<Scalar>::iterator ik = std::max_element(xk.begin(), xk.end());
        size_t max_id = ik - xk.begin();
        Scalar epsilon_upper = xk[max_id] - Scalar(1);
    
        size_t min_id = 0;
        Scalar min_v(10e9);
        for(size_t i=0; i<xk.size(); ++i)
        {
            if(xk[i] < min_v && sigma[i] > Scalar(0))
            {
                min_v = xk[i];
                min_id = i;
            }
        }
        Scalar epsilon_lower = Scalar(1) - min_v;
        
        Scalar epsilon = std::max(epsilon_upper, epsilon_lower);
        epsilon = epsilon_upper;
        
        if(epsilon <= epsilon0)
            break;
            
        ++k;
            
        if(epsilon == epsilon_upper)
        {
#ifdef DEBUG
            cInfo("Iteration: %d  Epsilon(+): %1.3lf", k, epsilon);
#endif
            x0.push_back(x[max_id]);
            
            Scalar sum = Scalar(0);
            for(int j=0; j<3; ++j)
            {
                auto vj = v(j, x, sigma);
                auto uj = u(j, x, sigma);
                sum += btPow((x[max_id].m_floats[j] - vj)*(x[max_id].m_floats[j] - vj)/(3*(uj - vj*vj)), Scalar(2));
            }
            
            Scalar beta = epsilon/(Scalar(1) + Scalar(3)*sum);
            
            std::vector<Scalar> e(sigma.size(), Scalar(0));
            e[max_id] = Scalar(1);
        
            for(size_t i=0; i<sigma.size(); ++i)
                sigma[i] = (Scalar(1) - beta)*sigma[i] + beta*e[i];  
        }
        else
        {
#ifdef DEBUG
            cInfo("Iteration: %d  Epsilon(-): %1.3lf", k, epsilon);
#endif
            Scalar maxwj(-10e9);
            for(int j=0; j<3; ++j)
            {
                auto vj = v(j, x, sigma);
                auto uj = u(j, x, sigma);
                Scalar wj = (x[min_id].m_floats[j] - vj)*(x[min_id].m_floats[j] - vj)/(3*(uj - vj*vj));
                if(wj > maxwj)
                    maxwj = wj;
            }
            
            Scalar beta = std::min(epsilon/(Scalar(1)-epsilon+Scalar(3)*maxwj), sigma[min_id]/(Scalar(1)-sigma[min_id])                    );
            */
            /*if(beta == sigma[min_id]/(Scalar(1)-sigma[min_id]))
            {
                std::vector<Vector3>::iterator it;
                it = std::find(x0.begin(), x0.end(), x[min_id]);
                x0.erase(it);
            }*/
            /*
            std::vector<Scalar> e(sigma.size(), Scalar(0));
            e[min_id] = Scalar(1);
        
            for(size_t i=0; i<sigma.size(); ++i)
                sigma[i] = (Scalar(1) + beta)*sigma[i] - beta*e[i];
        }
    }
    */
    
    Vector3 c;
    Vector3 d;
    
    if(k == 0)
    {
        c.setX((x0[0].x() + x0[1].x())/Scalar(2)); 
        c.setY((x0[2].y() + x0[3].y())/Scalar(2)); 
        c.setZ((x0[4].z() + x0[5].z())/Scalar(2));

        d.setX(btFabs(x0[0].x()-c.x()));
        d.setY(btFabs(x0[2].y()-c.y()));
        d.setZ(btFabs(x0[4].z()-c.z()));
    }
    else
    {
        c.setX(v(0,x,sigma));
        c.setY(v(1,x,sigma));
        c.setZ(v(2,x,sigma));
    
        for(int j=0; j<3; ++j)
        {
            d.m_floats[j] = Scalar(1)/(Scalar(3)*(u(j,x,sigma) - v(j,x,sigma)*v(j,x,sigma)));
            d.m_floats[j] = Scalar(1)/btSqrt(d.m_floats[j]);
        }
    }
    
#ifdef DEBUG
    cInfo("Ellipsoid center: %1.3lf %1.3lf %1.3lf", c.x(), c.y(), c.z());
    cInfo("Ellipsoid axis: %1.3lf %1.3lf %1.3lf", d.x(), d.y(), d.z());
    cInfo("Ellipsoid core points: %d", x0.size());
#endif
    
    fdApproxType =  GeometryApproxType::ELLIPSOID;
    fdApproxParams.resize(3);
    fdApproxParams[0] = d.getX();
    fdApproxParams[1] = d.getY();
    fdApproxParams[2] = d.getZ();
    
    //Compute added mass
    Scalar rho = Scalar(1000);
    Ocean* ocn;
    if((ocn = SimulationApp::getApp()->getSimulationManager()->getOcean()) != nullptr)
        rho = ocn->getLiquid().density;

    Scalar r12 = (fdApproxParams[1] + fdApproxParams[2])/Scalar(2);
    aMass.setX(LambKFactor(fdApproxParams[0], r12)*Scalar(4)/Scalar(3)*M_PI*rho*fdApproxParams[0]*r12*r12);
    aMass.setY(Scalar(4)/Scalar(3)*M_PI*rho*fdApproxParams[2]*fdApproxParams[2]*fdApproxParams[0]);
    aMass.setZ(Scalar(4)/Scalar(3)*M_PI*rho*fdApproxParams[1]*fdApproxParams[1]*fdApproxParams[0]);
    aI.setX(0); //THIS SHOULD BE > 0
    aI.setY(Scalar(1)/Scalar(12)*M_PI*rho*fdApproxParams[1]*fdApproxParams[1]*btPow(fdApproxParams[0], Scalar(3)));
    aI.setZ(Scalar(1)/Scalar(12)*M_PI*rho*fdApproxParams[2]*fdApproxParams[2]*btPow(fdApproxParams[0], Scalar(3)));
    
    //Set transform with respect to geometry
    Transform ellipsoidTransform;
    ellipsoidTransform.getBasis().setIdentity(); //Aligned with CG frame (for now)
    ellipsoidTransform.setOrigin(P_CB);
    T_CG2H = ellipsoidTransform;

    Vector3 Cd(Scalar(1)/fdApproxParams[0] , Scalar(1)/fdApproxParams[1], Scalar(1)/fdApproxParams[2]);
    Scalar maxCd = btMax(btMax(Cd.x(), Cd.y()), Cd.z());
    Cd /= maxCd;
    Cd = T_CG2O.getBasis().inverse() * Cd; // To origin frame
    Cd = Vector3(btFabs(Cd.getX()), btFabs(Cd.getY()), btFabs(Cd.getZ()));
    SetHydrodynamicCoefficients(Cd, Scalar(0.1)*Cd);

#ifdef DEBUG
    cInfo("--------------------------------------------------------------------");
#endif
    delete x;
}

Scalar SolidEntity::LambKFactor(Scalar r1, Scalar r2)
{
    Scalar e = Scalar(1) - r2*r2/r1;
    Scalar elog = (Scalar(1)+e)/(Scalar(1)-e);
    
    if(elog > Scalar(0))
    {
        Scalar alpha0 = Scalar(2)*(Scalar(1)-e*e)/(e*e) * (Scalar(0.5)*btLog((Scalar(1)+e)/(Scalar(1)-e)) - e);
        return alpha0/(Scalar(2)-alpha0);
    }
    else 
        return Scalar(1);
}

void SolidEntity::BuildGraphicalObject()
{
    if(phyMesh == nullptr || !SimulationApp::getApp()->hasGraphics())
        return;
        
    graObjectId = ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->BuildObject(phyMesh);
    phyObjectId = graObjectId;
}

void SolidEntity::BuildRigidBody()
{
    if(rigidBody == nullptr)
    {
        btDefaultMotionState* motionState = new btDefaultMotionState();
        
        //Generate collision shape
        btCollisionShape* colShape0 = BuildCollisionShape();
        btCompoundShape* colShape;
        
        if(colShape0->getShapeType() == COMPOUND_SHAPE_PROXYTYPE) //For a compound shape just move the children to avoid additional level
        {
            colShape = (btCompoundShape*)colShape0;
            for(int i=0; i < colShape->getNumChildShapes(); ++i)
                colShape->updateChildTransform(i, T_CG2C * colShape->getChildTransform(i), true);
        }
        else //For other shapes, create compound shape which allow for the shift against gravity centre
        {
            colShape = new btCompoundShape();
            colShape->addChildShape(T_CG2C, colShape0);
        }
        
        //Construct Bullet rigid body
        Scalar M = getAugmentedMass();
        Vector3 I = getAugmentedInertia();
        
        btRigidBody::btRigidBodyConstructionInfo rigidBodyCI(M, motionState, colShape, I);
        rigidBodyCI.m_friction = rigidBodyCI.m_rollingFriction = rigidBodyCI.m_restitution = Scalar(0.); //not used
        rigidBodyCI.m_linearDamping = rigidBodyCI.m_angularDamping = Scalar(0.); //not used
        rigidBodyCI.m_additionalDamping = false;
        
        rigidBody = new btRigidBody(rigidBodyCI);
        rigidBody->setUserPointer(this);
        rigidBody->setFlags(rigidBody->getFlags() | BT_ENABLE_GYROSCOPIC_FORCE_IMPLICIT_BODY);
        rigidBody->setCollisionFlags(rigidBody->getCollisionFlags() | btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);
        //rigidBody->setContactProcessingThreshold(0.002);
        //rigidBody->setCcdMotionThreshold(0.01);
        //rigidBody->setCcdSweptSphereRadius(0.9);
        
        // Soft contact
        if(contactK > Scalar(0))
            rigidBody->setContactStiffnessAndDamping(contactK, contactD);

        cInfo("Built rigid body %s [mass: %1.3lf; inertia: %1.3lf, %1.3lf, %1.3lf; volume: %1.1lf]", getName().c_str(), mass, Ipri.x(), Ipri.y(), Ipri.z(), volume*1e6);
    }
}

void SolidEntity::BuildMultibodyLinkCollider(btMultiBody *mb, unsigned int child, btMultiBodyDynamicsWorld *world)
{
    if(multibodyCollider == nullptr)
    {
        //Generate collision shape
        btCollisionShape* colShape0 = BuildCollisionShape();
        btCompoundShape* colShape;
        if(colShape0->getShapeType() == COMPOUND_SHAPE_PROXYTYPE) //For a compound shape just move the children to avoid additional level
        {
            colShape = (btCompoundShape*)colShape0;
            for(int i=0; i < colShape->getNumChildShapes(); ++i)
            {
                colShape->getChildShape(i)->setMargin(Scalar(0));
                colShape->updateChildTransform(i, T_CG2C * colShape->getChildTransform(i), true);
            }
        }
        else //For other shapes, create compound shape which allow for the shift against gravity centre
        {
            colShape = new btCompoundShape();
            colShape0->setMargin(Scalar(0));
            colShape->addChildShape(T_CG2C, colShape0);
            
        }
        colShape->setMargin(Scalar(0));
        
        //Construct Bullet multi-body link
        multibodyCollider = new btMultiBodyLinkCollider(mb, child - 1);
        multibodyCollider->setCollisionShape(colShape);
        multibodyCollider->setUserPointer(this); //HAS TO BE AFTER SETTING COLLISION SHAPE TO PROPAGATE TO ALL OF COMPOUND SUBSHAPES!!!!!
        multibodyCollider->setFriction(Scalar(0));
        multibodyCollider->setRestitution(Scalar(0));
        multibodyCollider->setRollingFriction(Scalar(0));
        multibodyCollider->setSpinningFriction(Scalar(0));
        multibodyCollider->setCollisionFlags(multibodyCollider->getCollisionFlags() | btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);
        multibodyCollider->setActivationState(DISABLE_DEACTIVATION);
        
        if(child > 0)
            mb->getLink(child - 1).m_collider = multibodyCollider;
        else
            mb->setBaseCollider(multibodyCollider);
        
        world->addCollisionObject(multibodyCollider, MASK_DYNAMIC, MASK_GHOST | MASK_STATIC | MASK_DYNAMIC | MASK_ANIMATED_COLLIDING);
        
        if(contactK > Scalar(0))
            multibodyCollider->setContactStiffnessAndDamping(contactK, contactD);
        
        //Graphics
        BuildGraphicalObject();
        
        cInfo("Built multibody link %s (mass[kg]: %1.3lf; inertia[kgm2]: %1.3lf, %1.3lf, %1.3lf; volume[cm3]: %1.1lf)", getName().c_str(), mass, Ipri.x(), Ipri.y(), Ipri.z(), volume*1e6);
    }
}

void SolidEntity::AddToSimulation(SimulationManager* sm)
{
    AddToSimulation(sm, Transform::getIdentity());
}

void SolidEntity::AddToSimulation(SimulationManager* sm, const Transform& origin)
{
    if(rigidBody == nullptr)
    {
        // Build
        BuildRigidBody();
        BuildGraphicalObject();
        
        // Setup sleeping
        Scalar linSleep, angSleep;
        sm->getSleepingThresholds(linSleep, angSleep);
        if(linSleep <= Scalar(0) || angSleep <= Scalar(0))        
            rigidBody->setActivationState(DISABLE_DEACTIVATION);
        else
        {
            rigidBody->setSleepingThresholds(linSleep, angSleep);
        }

        // Add to world
        Transform Tcg = origin * T_CG2O.inverse();
        rigidBody->setMotionState(new btDefaultMotionState(Tcg));
        sm->getDynamicsWorld()->addRigidBody(rigidBody, MASK_DYNAMIC, MASK_GHOST | MASK_STATIC | MASK_DYNAMIC | MASK_ANIMATED_COLLIDING);
    }
}

void SolidEntity::RemoveFromSimulation(SimulationManager* sm)
{
    sm->getDynamicsWorld()->removeRigidBody(rigidBody);
    rigidBody = nullptr;
}

void SolidEntity::UpdateAcceleration(Scalar dt)
{
    Vector3 currentV = getLinearVelocity();
    Vector3 currentOmega = getAngularVelocity();
    linearAcc = (currentV - lastV)/dt;
    angularAcc = (currentOmega - lastOmega)/dt;
    lastV = currentV;
    lastOmega = currentOmega;
}

void SolidEntity::ApplyGravity(const Vector3& g)
{
    if(rigidBody != nullptr)
    {
        rigidBody->applyCentralForce(g * mass);
    }
}

void SolidEntity::ApplyCentralForce(const Vector3& force)
{
    if(rigidBody != nullptr)
        rigidBody->applyCentralForce(force);
    else if(multibodyCollider != nullptr)
    {
        btMultiBody* multiBody = multibodyCollider->m_multiBody;
        int index = multibodyCollider->m_link;
        
        if(index == -1) //base
            multiBody->addBaseForce(force);
        else
            multiBody->addLinkForce(index, force);
    }
}

void SolidEntity::ApplyTorque(const Vector3& torque)
{
    if(rigidBody != nullptr)
        rigidBody->applyTorque(torque);
    else if(multibodyCollider != nullptr)
    {
        btMultiBody* multiBody = multibodyCollider->m_multiBody;
        int index = multibodyCollider->m_link;
        
        if(index == -1) //base
            multiBody->addBaseTorque(torque);
        else
            multiBody->addLinkTorque(index, torque);
    }
}

BodyFluidPosition SolidEntity::CheckBodyFluidPosition(Ocean* ocn)
{
    Vector3 aabbMin, aabbMax;
    getAABB(aabbMin, aabbMax);
    Vector3 d = aabbMax-aabbMin;
    
    unsigned int underwater = 0;
    if(ocn->GetDepth(aabbMin) > Scalar(0)) ++underwater;
    if(ocn->GetDepth(aabbMax) > Scalar(0)) ++underwater;
    if(ocn->GetDepth(aabbMin + Vector3(d.x(), 0, 0)) > Scalar(0)) ++underwater;
    if(ocn->GetDepth(aabbMin + Vector3(0, d.y(), 0)) > Scalar(0)) ++underwater;
    if(ocn->GetDepth(aabbMin + Vector3(d.x(), d.y(), 0)) > Scalar(0)) ++underwater;
    if(ocn->GetDepth(aabbMin + Vector3(0, 0, d.z())) > Scalar(0)) ++underwater;
    if(ocn->GetDepth(aabbMin + Vector3(d.x(), 0, d.z())) > Scalar(0)) ++underwater;
    if(ocn->GetDepth(aabbMin + Vector3(0, d.y(), d.z())) > Scalar(0)) ++underwater;
    
    if(underwater == 0)
        return BodyFluidPosition::OUTSIDE;
    else if(underwater == 8)
        return BodyFluidPosition::INSIDE;
    else
        return BodyFluidPosition::CROSSING_SURFACE;
}

void SolidEntity::CorrectHydrodynamicForces(Ocean* ocn, Vector3& _Fdq, Vector3& _Tdq, Vector3& _Fdf, Vector3& _Tdf)
{
    Vector3 Fdq = getOTransform().getBasis().inverse() * _Fdq; // In origin frame
    Fdq = Fdq.safeNormalize();
    Scalar Fdqc = btFabs(Fdq.getX()) * fdCd.getX() + btFabs(Fdq.getY()) * fdCd.getY() + btFabs(Fdq.getZ()) * fdCd.getZ();
    //Fdqc = 1.0;
    _Fdq = Scalar(0.5) * ocn->getLiquid().density * Fdqc * _Fdq; //0.5*rho*Cd*S*v2 from drag equation    

    Vector3 Tdq = getOTransform().getBasis().inverse() * _Tdq; // In origin frame
    Tdq = Tdq.safeNormalize();
    Scalar Tdqc = btFabs(Tdq.getX()) * fdCd.getX() + btFabs(Tdq.getY()) * fdCd.getY() + btFabs(Tdq.getZ()) * fdCd.getZ();
    //Tdqc = 1.0;
    _Tdq = Scalar(0.5) * ocn->getLiquid().density * Tdqc * _Tdq; //0.5*rho*Cd*S*v2 from drag equation

    Vector3 Fdf = getOTransform().getBasis().inverse() * _Fdf; // In origin frame
    Fdf = Fdf.safeNormalize();
    Scalar Fdfc = btFabs(Fdf.getX()) * fdCf.getX() + btFabs(Fdf.getY()) * fdCf.getY() + btFabs(Fdf.getZ()) * fdCf.getZ(); 
    //Fdfc = 1.0;
    _Fdf = ocn->getLiquid().density * Fdfc * _Fdf; //rho*Cf*S*v from viscous drag equation
    
    Vector3 Tdf = getOTransform().getBasis().inverse() * _Tdf; // In origin frame
    Tdf = Tdf.safeNormalize();
    Scalar Tdfc = btFabs(Tdf.getX()) * fdCf.getX() + btFabs(Tdf.getY()) * fdCf.getY() + btFabs(Tdf.getZ()) * fdCf.getZ();
    //Tdfc = 1.0;
    _Tdf = ocn->getLiquid().density * Tdfc * _Tdf; //rho*S*v from viscous drag equation
}

void SolidEntity::ComputeHydrodynamicForcesSurface(const HydrodynamicsSettings& settings, const Mesh* mesh, Ocean* ocn, const Transform& T_CG, const Transform& T_C,
                                            const Vector3& _v, const Vector3& _omega, Vector3& _Fb, Vector3& _Tb, Vector3& _Fdq, Vector3& _Tdq, Vector3& _Fdf, Vector3& _Tdf, 
                                            Scalar& _Swet, Scalar& _Vsub, Renderable& debug)
{
    if(mesh == nullptr)
    {
        if(settings.reallisticBuoyancy)
        {
            _Fb.setZero();
            _Tb.setZero();
        }

        if(settings.dampingForces)
        {
            _Fdq.setZero();
            _Tdq.setZero();
            _Fdf.setZero();
            _Tdf.setZero();
        }

        _Swet = Scalar(0);
        _Vsub = Scalar(0);
        return;
    }

    //Computation with floats (geometry has float precision)
    glm::vec3 Fb(0.f);
    glm::vec3 Tb(0.f);
    glm::vec3 Fdq(0.f);
    glm::vec3 Tdq(0.f);
    glm::vec3 Fdf(0.f);
    glm::vec3 Tdf(0.f);
    GLfloat Swet(0.f);
    GLfloat Vsub(0.f);
    glm::mat4 TCG = glMatrixFromTransform(T_CG);
    glm::mat4 TC = glMatrixFromTransform(T_C);
    glm::vec3 v = glVectorFromVector(_v);
    glm::vec3 omega = glVectorFromVector(_omega);
    glm::vec3 CBsub(0.f);
   
    //Calculate fluid dynamics forces and torques
    glm::vec3 p = glm::vec3(TCG[3]);
    glm::vec3 p0 = p; //Point used as a center of mesh for volume calculation.
    p0.z = 0.f;       //When the robot is far from the world origin numerical erros would explode without translating the mesh data!
    
    //Loop through all faces...
    for(size_t i=0; i<mesh->faces.size(); ++i)
    {
        //Global coordinates
        glm::vec3 p1gl = mesh->getVertexPos(i, 0);
        glm::vec3 p2gl = mesh->getVertexPos(i, 1);
        glm::vec3 p3gl = mesh->getVertexPos(i, 2);
        glm::vec3 p1 = glm::vec3(TC * glm::vec4(p1gl, 1.f));
        glm::vec3 p2 = glm::vec3(TC * glm::vec4(p2gl, 1.f));
        glm::vec3 p3 = glm::vec3(TC * glm::vec4(p3gl, 1.f));
        
        //Check if face underwater
        GLfloat depth[3];
        depth[0] = ocn->GetDepth(p1);
        depth[1] = ocn->GetDepth(p2);
        depth[2] = ocn->GetDepth(p3);
        
        if(depth[0] < 0.f && depth[1] < 0.f && depth[2] < 0.f)
            continue;
        
        //Calculate face properties
        glm::vec3 fc;
        glm::vec3 fn;
        glm::vec3 fn1;
        GLfloat A;
        
        if(depth[0] < 0.f) //Vertex 1 above water
        {
            if(depth[1] < 0.f) //Two vertices above water (triangle)
            {
                p1 = p3 + (p1-p3) * (depth[2]/(fabsf(depth[0]) + depth[2]));
                p2 = p3 + (p2-p3) * (depth[2]/(fabsf(depth[1]) + depth[2]));
                //p3 without change
                
                //Volume properties
                glm::vec3 p01 = p1-p0;
                glm::vec3 p02 = p2-p0;
                glm::vec3 p03 = p3-p0;
                glm::vec3 tetraCG = (p01+p02+p03)/4.f;
                GLfloat tetraV6 = glm::dot(p01, glm::cross(p02, p03));
                CBsub += tetraCG * tetraV6;
                Vsub += tetraV6;
                
                //Face properties
                glm::vec3 fv1 = p2-p1; //One side of the face (triangle)
                glm::vec3 fv2 = p3-p1; //Another side of the face (triangle)
                fc = (p1+p2+p3)/3.f; //Face centroid
        
                fn = glm::cross(fv1, fv2); //Normal of the face (length != 1)
                GLfloat len = glm::length2(fn); //Double area
                if(len < 1e-12f) continue;
                len = glm::sqrt(len);
                fn1 = fn/len; //Normalised normal (length = 1)
                A = len/2.f; //Area of the face (triangle)         
#ifdef DEBUG_HYDRO
                debug.points.push_back(p1);
                debug.points.push_back(p2);
                debug.points.push_back(p2);
                debug.points.push_back(p3);
                debug.points.push_back(p3);
                debug.points.push_back(p1);
#endif
            }
            else if(depth[2] < 0.f) //Two vertices above water (triangle)
            {
                p1 = p2 + (p1-p2) * (depth[1]/(fabsf(depth[0]) + depth[1]));
                //p2 without change
                p3 = p2 + (p3-p2) * (depth[1]/(fabsf(depth[2]) + depth[1]));
                
                //Volume properties
                glm::vec3 p01 = p1-p0;
                glm::vec3 p02 = p2-p0;
                glm::vec3 p03 = p3-p0;
                glm::vec3 tetraCG = (p01+p02+p03)/4.f;
                GLfloat tetraV6 = glm::dot(p01, glm::cross(p02, p03));
                CBsub += tetraCG * tetraV6;
                Vsub += tetraV6;
                
                //Face properties
                glm::vec3 fv1 = p2-p1; //One side of the face (triangle)
                glm::vec3 fv2 = p3-p1; //Another side of the face (triangle)
                fc = (p1+p2+p3)/3.f; //Face centroid
        
                fn = glm::cross(fv1, fv2); //Normal of the face (length != 1)
                GLfloat len = glm::length2(fn);
                if(len < 1e-12f) continue;
                len = glm::sqrt(len);
                fn1 = fn/len; //Normalised normal (length = 1)
                A = len/2.f; //Area of the face (triangle)         
#ifdef DEBUG_HYDRO
                debug.points.push_back(p1);
                debug.points.push_back(p2);
                debug.points.push_back(p2);
                debug.points.push_back(p3);
                debug.points.push_back(p3);
                debug.points.push_back(p1);
#endif
            }
            else //depth[1] >= 0 && depth[2] >= 0 --> Two vertices under water (quad = two triangles)
            {
                //Quad!!!!
                glm::vec3 p4 = p3 + (p1-p3) * (depth[2]/(fabsf(depth[0]) + depth[2]));
                p1 = p2 + (p1-p2) * (depth[1]/(fabsf(depth[0]) + depth[1]));
                //p2 without change
                //p3 without change
                
                //Volume properties
                //Tetra 1
                glm::vec3 p01 = p1-p0;
                glm::vec3 p02 = p2-p0;
                glm::vec3 p03 = p3-p0;
                glm::vec3 tetraCG = (p01+p02+p03)/4.f;
                GLfloat tetraV6 = glm::dot(p01, glm::cross(p02, p03));
                CBsub += tetraCG * tetraV6;
                Vsub += tetraV6;
                //Tetra 2
                glm::vec3 p04 = p4-p0;
                tetraCG = (p01+p03+p04)/4.f;
                tetraV6 = glm::dot(p01, glm::cross(p03, p04));
                CBsub += tetraCG * tetraV6;
                Vsub += tetraV6;
                
                //Face properties
                glm::vec3 fv1 = p2-p1;
                glm::vec3 fv2 = p4-p1;
                glm::vec3 fv3 = p2-p3;
                glm::vec3 fv4 = p4-p3;
                fc = (p1 + p2 + p3 + p4)/4.f;
                
                fn = glm::cross(fv1, fv2);
                GLfloat len = glm::length2(fn);
                if(len < 1e-12f) continue;
                len = glm::sqrt(len);
                fn1 = fn/len;
                A = (len + glm::length(glm::cross(fv3, fv4)))/2.f; //Quad
                fn = fn1 * A;
#ifdef DEBUG_HYDRO
                debug.points.push_back(p1);
                debug.points.push_back(p2);
                debug.points.push_back(p2);
                debug.points.push_back(p3);
                debug.points.push_back(p3);
                debug.points.push_back(p4);
                debug.points.push_back(p4);
                debug.points.push_back(p1);
#endif  
            }
        }
        else if(depth[1] < 0.f)
        {
            if(depth[2] < 0.f)
            {
                //p1 without change
                p2 = p1 + (p2-p1) * (depth[0]/(fabsf(depth[1]) + depth[0]));
                p3 = p1 + (p3-p1) * (depth[0]/(fabsf(depth[2]) + depth[0]));
                
                //Volume properties
                glm::vec3 p01 = p1-p0;
                glm::vec3 p02 = p2-p0;
                glm::vec3 p03 = p3-p0;
                glm::vec3 tetraCG = (p01+p02+p03)/4.f;
                GLfloat tetraV6 = glm::dot(p01, glm::cross(p02, p03));
                CBsub += tetraCG * tetraV6;
                Vsub += tetraV6;

                //Face properties
                glm::vec3 fv1 = p2-p1; //One side of the face (triangle)
                glm::vec3 fv2 = p3-p1; //Another side of the face (triangle)
                fc = (p1+p2+p3)/3.f; //Face centroid
        
                fn = glm::cross(fv1, fv2); //Normal of the face (length != 1)
                GLfloat len = glm::length2(fn);
                if(len < 1e-12f) continue;
                len = glm::sqrt(len);
                fn1 = fn/len; //Normalised normal (length = 1)
                A = len/2.f; //Area of the face (triangle)
#ifdef DEBUG_HYDRO
                debug.points.push_back(p1);
                debug.points.push_back(p2);
                debug.points.push_back(p2);
                debug.points.push_back(p3);
                debug.points.push_back(p3);
                debug.points.push_back(p1);
#endif                
            }
            else
            {
                //Quad!!!!
                glm::vec3 p4 = p3 + (p2-p3) * (depth[2]/(fabsf(depth[1]) + depth[2]));
                //p1 without change
                p2 = p1 + (p2-p1) * (depth[0]/(fabsf(depth[1]) + depth[0]));
                //p3 without change
                
                //Volume properties
                //Tetra 1
                glm::vec3 p01 = p1-p0;
                glm::vec3 p02 = p2-p0;
                glm::vec3 p03 = p3-p0;
                glm::vec3 tetraCG = (p01+p02+p03)/4.f;
                GLfloat tetraV6 = glm::dot(p01, glm::cross(p02, p03));
                CBsub += tetraCG * tetraV6;
                Vsub += tetraV6;
                //Tetra 2
                glm::vec3 p04 = p4-p0;
                tetraCG = (p02+p04+p03)/4.f;
                tetraV6 = glm::dot(p02, glm::cross(p04, p03));
                CBsub += tetraCG * tetraV6;
                Vsub += tetraV6;              

                //Face properties
                glm::vec3 fv1 = p2-p1;
                glm::vec3 fv2 = p3-p1;
                glm::vec3 fv3 = p2-p3;
                glm::vec3 fv4 = p4-p3;
                fc = (p1 + p2 + p3 + p4)/4.f;
                fn = glm::cross(fv1, fv2); //Triangle 1
                GLfloat len = glm::length2(fn);
                if(len < 1e-12f) continue;    
                len = glm::sqrt(len);
                fn1 = fn/len;
                A = (len + glm::length(glm::cross(fv3, fv4)))/2.f; //Quad
                fn = fn1 * A;
#ifdef DEBUG_HYDRO
                debug.points.push_back(p1);
                debug.points.push_back(p2);
                debug.points.push_back(p2);
                debug.points.push_back(p4);
                debug.points.push_back(p4);
                debug.points.push_back(p3);
                debug.points.push_back(p3);
                debug.points.push_back(p1);
#endif                 
            }
        }
        else if(depth[2] < 0.f)
        {
            //Quad!!!!
            glm::vec3 p4 = p1 + (p3-p1) * (depth[0]/(fabsf(depth[2]) + depth[0]));
            //p1 without change
            //p2 without change
            p3 = p2 + (p3-p2) * (depth[1]/(fabsf(depth[2]) + depth[1]));
                
            //Volume properties
            //Tetra 1
            glm::vec3 p01 = p1-p0;
            glm::vec3 p02 = p2-p0;
            glm::vec3 p03 = p3-p0;
            glm::vec3 tetraCG = (p01+p02+p03)/4.f;
            GLfloat tetraV6 = glm::dot(p01, glm::cross(p02, p03));
            CBsub += tetraCG * tetraV6;
            Vsub += tetraV6;
            //Tetra 2
            glm::vec3 p04 = p4-p0;
            tetraCG = (p01+p03+p04)/4.f;
            tetraV6 = glm::dot(p01, glm::cross(p03, p04));
            CBsub += tetraCG * tetraV6;
            Vsub += tetraV6;
            
            //Face properties
            glm::vec3 fv1 = p2-p1;
            glm::vec3 fv2 = p4-p1;
            glm::vec3 fv3 = p2-p3;
            glm::vec3 fv4 = p4-p3;
            fc = (p1 + p2 + p3 + p4)/4.f;
            fn = glm::cross(fv1, fv2);
            GLfloat len = glm::length2(fn);
            if(len < 1e-12f) continue;
            len = glm::sqrt(len);
            fn1 = fn/len;
            A = (len + glm::length(glm::cross(fv3, fv4)))/2.f; //Quad
            fn = fn1 * A;
#ifdef DEBUG_HYDRO
            debug.points.push_back(p1);
            debug.points.push_back(p2);
            debug.points.push_back(p2);
            debug.points.push_back(p3);
            debug.points.push_back(p3);
            debug.points.push_back(p4);
            debug.points.push_back(p4);
            debug.points.push_back(p1);
#endif             
        }
        else //All underwater
        {
            //Volume properties
            glm::vec3 p01 = p1-p0;
            glm::vec3 p02 = p2-p0;
            glm::vec3 p03 = p3-p0;
            glm::vec3 tetraCG = (p01+p02+p03)/4.f;
            GLfloat tetraV6 = glm::dot(p01, glm::cross(p02, p03));
            CBsub += tetraCG * tetraV6;
            Vsub += tetraV6;

            //Face properties
            glm::vec3 fv1 = p2-p1; //One side of the face (triangle)
            glm::vec3 fv2 = p3-p1; //Another side of the face (triangle)
            fn = glm::cross(fv1, fv2); //Normal of the face (length != 1)
            GLfloat len = glm::length2(fn);
            if(len < 1e-12f) continue;
            len = glm::sqrt(len);
            fn1 = fn/len; //Normalised normal (length = 1)
            A = len/2.f; //Area of the face (triangle)
            fc = (p1+p2+p3)/3.f; //Face centroid
#ifdef DEBUG_HYDRO
            debug.points.push_back(p1);
            debug.points.push_back(p2);
            debug.points.push_back(p2);
            debug.points.push_back(p3);
            debug.points.push_back(p3);
            debug.points.push_back(p1);
#endif             
        }

        //Buoyancy force
        if(settings.reallisticBuoyancy && ocn->hasWaves())
        {
            GLfloat depthc = ocn->GetDepth(fc);
            glm::vec3 Fbi = -fn1 * A * depthc; //Buoyancy force per face (based on pressure)        
            
            //Accumulate
            Fb += Fbi;
            Tb += glm::cross(fc-p, Fbi);
        }
        
        //Damping force
        if(settings.dampingForces)
        {
            glm::vec3 vc = ocn->GetFluidVelocity(fc) - (v + glm::cross(omega, fc-p));
            GLfloat vc_n = glm::dot(vc, fn1);
            glm::vec3 vn = vc_n  * fn1; //Normal velocity
            glm::vec3 vt = vc - vn; //Tangent velocity
            
            if(vc_n < -1e-12f) //If liquid is approaching the surface
            {
                GLfloat vmag2 = glm::length2(vc);
                glm::vec3 quadratic = vc * sqrtf(vmag2) * -vc_n * A;
                Fdq += quadratic;
                Tdq += glm::cross(fc - p, quadratic);
            }

            GLfloat vmag2 = glm::length2(vt);
            if(vmag2 > 1e-9f)
            {
                glm::vec3 skin = vt * A;
                Fdf += skin;
                Tdf += glm::cross(fc - p, skin);
            }
        }

        //Wetted surface area
        Swet += A;
    }

    //Buoyancy
    if(settings.reallisticBuoyancy && Vsub > 1e-9f)
    {
        _Vsub = Vsub/6.f;
        
        if(ocn->hasWaves())
        {
            Fb *= ocn->getLiquid().density * SimulationApp::getApp()->getSimulationManager()->getGravity().getZ();
            Tb *= ocn->getLiquid().density * SimulationApp::getApp()->getSimulationManager()->getGravity().getZ();
            _Fb = Vector3(Fb.x, Fb.y, Fb.z);
            _Tb = Vector3(Tb.x, Tb.y, Tb.z);
        }
        else
        {
            CBsub = CBsub/Vsub + p0;
            Vector3 _CBsub(CBsub.x, CBsub.y, CBsub.z);
            _Fb = -_Vsub * ocn->getLiquid().density * SimulationApp::getApp()->getSimulationManager()->getGravity();
            _Tb = (_CBsub - T_CG.getOrigin()).cross(_Fb);
        }        
    }
    
    //Damping forces
    if(settings.dampingForces)
    {
        _Fdq = Vector3(Fdq.x, Fdq.y, Fdq.z);
        _Tdq = Vector3(Tdq.x, Tdq.y, Tdq.z);
        _Fdf = Vector3(Fdf.x, Fdf.y, Fdf.z);
        _Tdf = Vector3(Tdf.x, Tdf.y, Tdf.z);
    }

    //Wetted surface area
    _Swet = Swet;
}

void SolidEntity::ComputeHydrodynamicForcesSubmerged(const Mesh* mesh, Ocean* ocn, const Transform& T_CG, const Transform& T_C,
                                              const Vector3& _v, const Vector3& _omega, Vector3& _Fdq, Vector3& _Tdq, Vector3& _Fdf, Vector3& _Tdf)
{
    if(mesh == nullptr)
    {
        _Fdq.setZero();
        _Tdq.setZero();
        _Fdf.setZero();
        _Tdf.setZero();
        return;
    }

    //Computation with floats (geometry has float precision)
    glm::vec3 Fdq(0.f);
    glm::vec3 Tdq(0.f);
    glm::vec3 Fdf(0.f);
    glm::vec3 Tdf(0.f);
    glm::mat4 TCG = glMatrixFromTransform(T_CG);
    glm::mat4 TC = glMatrixFromTransform(T_C);
    glm::vec3 v = glVectorFromVector(_v);
    glm::vec3 omega = glVectorFromVector(_omega);
    
    //Calculate fluid dynamics forces and torques
    glm::vec3 p = glm::vec3(TCG[3]);

    //Loop through all faces...
    for(size_t i=0; i<mesh->faces.size(); ++i)
    {
        //Global coordinates
        glm::vec3 p1gl = mesh->getVertexPos(i, 0);
        glm::vec3 p2gl = mesh->getVertexPos(i, 1);
        glm::vec3 p3gl = mesh->getVertexPos(i, 2);
        glm::vec3 p1 = glm::vec3(TC * glm::vec4(p1gl, 1.f));
        glm::vec3 p2 = glm::vec3(TC * glm::vec4(p2gl, 1.f));
        glm::vec3 p3 = glm::vec3(TC * glm::vec4(p3gl, 1.f));
        
        //Face properties
        glm::vec3 fv1 = p2-p1; //One side of the face (triangle)
        glm::vec3 fv2 = p3-p1; //Another side of the face (triangle)
        glm::vec3 fn = glm::cross(fv1, fv2); //Normal of the face (length != 1)
        GLfloat len = glm::length2(fn);
        if(len < 1e-12f) continue;
        len = glm::sqrt(len);
        glm::vec3 fn1 = fn/len; //Normalised normal (length = 1)
        GLfloat A = len/2.f; //Area of the face (triangle)
        glm::vec3 fc = (p1+p2+p3)/3.f; //Face centroid
     
        //Forces
        glm::vec3 vc = ocn->GetFluidVelocity(fc) - (v + glm::cross(omega, fc-p));
        GLfloat vc_n = glm::dot(vc, fn1);
        glm::vec3 vn = vc_n  * fn1; //Normal velocity
        glm::vec3 vt = vc - vn; //Tangent velocity
        
        if(vc_n < -1e-12f) //If liquid is approaching the surface
        {
            GLfloat vmag2 = glm::length2(vc);
            glm::vec3 quadratic = vc * sqrtf(vmag2) * -vc_n * A;
            Fdq += quadratic;
            Tdq += glm::cross(fc - p, quadratic);
        }

        GLfloat vmag2 = glm::length2(vt);
        if(vmag2 > 1e-9f)
        {
            glm::vec3 skin = vt * A;
            Fdf += skin;
            Tdf += glm::cross(fc - p, skin);
        }
    }

    _Fdq = Vector3(Fdq.x, Fdq.y, Fdq.z);
    _Tdq = Vector3(Tdq.x, Tdq.y, Tdq.z);
    _Fdf = Vector3(Fdf.x, Fdf.y, Fdf.z);
    _Tdf = Vector3(Tdf.x, Tdf.y, Tdf.z);
}

void SolidEntity::ComputeHydrodynamicForces(HydrodynamicsSettings settings, Ocean* ocn)
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
    
    //Get velocities and transformations
    Vector3 v = getLinearVelocity();
    Vector3 omega = getAngularVelocity();
    
    //Check if fully submerged --> simplifies buoyancy calculation
    if(bf == BodyFluidPosition::INSIDE)
    {
        //Compute buoyancy based on CB position
        if(isBuoyant())
        {
            Fb = -volume*ocn->getLiquid().density * SimulationApp::getApp()->getSimulationManager()->getGravity();
            Tb = (getCGTransform() * P_CB - getCGTransform().getOrigin()).cross(Fb);
        }
        
        if(settings.dampingForces)
            ComputeHydrodynamicForcesSubmerged(getPhysicsMesh(), ocn, getCGTransform(), getCTransform(), v, omega, Fdq, Tdq, Fdf, Tdf);

        Swet = surface;
    }
    else //CROSSING_FLUID_SURFACE
    {
        if(!isBuoyant()) settings.reallisticBuoyancy = false;
        ComputeHydrodynamicForcesSurface(settings, getPhysicsMesh(), ocn, getCGTransform(), getCTransform(), v, omega, Fb, Tb, Fdq, Tdq, Fdf, Tdf, Swet, Vsub, submerged);
    }
    
    if(settings.dampingForces)
        CorrectHydrodynamicForces(ocn, Fdq, Tdq, Fdf, Tdf);
}

void SolidEntity::ComputeAerodynamicForces(Atmosphere* atm)
{
    if(phy.mode != BodyPhysicsMode::AERODYNAMIC) return;
    
    //Get velocities and transformations
    Vector3 v = getLinearVelocity();
    Vector3 omega = getAngularVelocity();
    
    //Compute drag
    ComputeAerodynamicForces(getPhysicsMesh(), atm, getCGTransform(), getCTransform(), v, omega, Fda, Tda);
    CorrectAerodynamicForces(atm, Fda, Tda);
}

void SolidEntity::ComputeAerodynamicForces(const Mesh* mesh, Atmosphere* atm, const Transform& T_CG, const Transform& T_C,
                                           const Vector3& _v, const Vector3& _omega, Vector3& _Fda, Vector3& _Tda)
{
    if(mesh == nullptr) 
    {
        _Fda.setZero();
        _Tda.setZero();
        return;
    }
    
    glm::vec3 Fda(0.f);
    glm::vec3 Tda(0.f);
    glm::mat4 TCG = glMatrixFromTransform(T_CG);
    glm::mat4 TC = glMatrixFromTransform(T_C);
    glm::vec3 v = glVectorFromVector(_v);
    glm::vec3 omega = glVectorFromVector(_omega);
    
    //Calculate fluid dynamics forces and torques
    glm::vec3 p = glm::vec3(TCG[3]);

    //Loop through all faces...
    for(size_t i=0; i<mesh->faces.size(); ++i)
    {
        //Global coordinates
        glm::vec3 p1gl = mesh->getVertexPos(i, 0);
        glm::vec3 p2gl = mesh->getVertexPos(i, 1);
        glm::vec3 p3gl = mesh->getVertexPos(i, 2);
        glm::vec3 p1 = glm::vec3(TC * glm::vec4(p1gl, 1.f));
        glm::vec3 p2 = glm::vec3(TC * glm::vec4(p2gl, 1.f));
        glm::vec3 p3 = glm::vec3(TC * glm::vec4(p3gl, 1.f));
        
        //Calculate face properties
        glm::vec3 fv1 = p2-p1; //One side of the face (triangle)
        glm::vec3 fv2 = p3-p1; //Another side of the face (triangle)
        glm::vec3 fn = glm::cross(fv1, fv2); //Normal of the face (length != 1)
        GLfloat len = glm::length2(fn);
        if(len < 1e-12f) continue;
        len = glm::sqrt(len);
        glm::vec3 fn1 = fn/len; //Normalised normal (length = 1)
        GLfloat A = len/2.f; //Area of the face (triangle)
        glm::vec3 fc = (p1+p2+p3)/3.f; //Face centroid

        //Damping forces
        glm::vec3 vc = atm->GetFluidVelocity(fc) - (v + glm::cross(omega, fc-p));
        glm::vec3 vn = glm::dot(vc, fn1) * fn1; //Normal velocity
        
        if(glm::dot(fn1, vn) < -1e-12f)
        {
            glm::vec3 quadratic = vn * vn.length() * A;
            //Accumulate
            Fda += quadratic;
            Tda += glm::cross(fc - p, quadratic);
        }
    }

    Scalar density = atm->getGas().density;
    _Fda = Scalar(0.5) * density * Vector3(Fda.x, Fda.y, Fda.z);
    _Tda = Scalar(0.5) * density * Vector3(Tda.x, Tda.y, Tda.z);
}

void SolidEntity::CorrectAerodynamicForces(Atmosphere* atm, Vector3& _Fda, Vector3& _Tda)
{
    //Correct forces
    Vector3 Fdan = (T_CG2H.getBasis().inverse() * _Fda).safeNormalize(); //Transform force to proxy frame
    Scalar corFactor(1.0);
    
    switch(fdApproxType)
    {
        case  GeometryApproxType::AUTO:
        case  GeometryApproxType::SPHERE:
            break;
                
        case  GeometryApproxType::CYLINDER:
        {
            Vector3 Cd(0.5, 0.5, 1.0);
            corFactor = Cd.dot(Fdan);
        }
            break;
                
        case  GeometryApproxType::ELLIPSOID:
        {
            Vector3 Cd(Scalar(1)/fdApproxParams[0] , Scalar(1)/fdApproxParams[1], Scalar(1)/fdApproxParams[2]);
            Scalar maxCd = btMax(btMax(Cd.x(), Cd.y()), Cd.z());
            Cd /= maxCd;
            corFactor = Cd.dot(Fdan);
        }
            break;
    }
    
    _Fda *= btFabs(corFactor);
    _Tda *= btFabs(corFactor);
}

void SolidEntity::ApplyHydrodynamicForces()
{
    ApplyCentralForce(Fb + Fdq + Fdf);
    ApplyTorque(Tb + Tdq + Tdf);
}

void SolidEntity::ApplyAerodynamicForces()
{
    ApplyCentralForce(Fda);
    ApplyTorque(Tda);
}

}
