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
//  Created by Patryk Cieslak on 12/29/12.
//  Copyright (c) 2012-2019 Patryk Cieslak. All rights reserved.
//

#include "entities/SolidEntity.h"

#include "core/GraphicalSimulationApp.h"
#include "core/SimulationManager.h"
#include "core/Console.h"
#include "graphics/OpenGLPipeline.h"
#include "graphics/OpenGLContent.h"
#include "utils/SystemUtil.hpp"
#include "entities/forcefields/Ocean.h"
#include "entities/forcefields/Atmosphere.h"

namespace sf
{

SolidEntity::SolidEntity(std::string uniqueName, Material m, BodyPhysicsType bpt, int _lookId, Scalar thickness, bool isBuoyant) : Entity(uniqueName)
{
	mat = m;
    thick = thickness;
    
    if((bpt == BodyPhysicsType::SUBMERGED_BODY || bpt == BodyPhysicsType::FLOATING_BODY) && !SimulationApp::getApp()->getSimulationManager()->isOceanEnabled())
        phyType = BodyPhysicsType::SURFACE_BODY;
    else
        phyType = bpt;
    
    buoyant = isBuoyant;
    lookId = _lookId;
    
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
    aMass = Matrix6Eigen::Zero();
    Ipri.setZero();
    contactK = Scalar(-1);
    contactD = Scalar(0);
    volume = Scalar(0);
    fdApproxType = FD_APPROX_AUTO;
    fdApproxParams = std::vector<Scalar>(0);
    T_CG2H = Transform::getIdentity();
    
    //Set vectors to zero
    Fb.setZero();
    Tb.setZero();
    Fdl.setZero();
    Tdl.setZero();
    Fdq.setZero();
    Tdq.setZero();
    Fda.setZero();
    Tda.setZero();
	filteredLinearVel.setZero();
	filteredAngularVel.setZero();
    linearAcc.setZero();
    angularAcc.setZero();
	
    //Set pointers
    rigidBody = NULL;
    multibodyCollider = NULL;
	phyMesh = NULL;
    graObjectId = -1;
    phyObjectId = -1;
    dm = DisplayMode::DISPLAY_GRAPHICAL;
    submerged.type = RenderableType::HYDRO_LINES;
}

SolidEntity::~SolidEntity()
{
    if(phyMesh != NULL) delete phyMesh;
}

EntityType SolidEntity::getType()
{
    return ENTITY_SOLID;
}

void SolidEntity::ScalePhysicalPropertiesToArbitraryMass(Scalar mass)
{
    if(rigidBody != NULL || multibodyCollider != NULL)
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
    if(rigidBody != NULL || multibodyCollider != NULL)
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
    
    if(rigidBody != NULL)
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
    else if(multibodyCollider != NULL)
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

void SolidEntity::setDisplayMode(DisplayMode m)
{
    dm = m;
}

void SolidEntity::setLook(int newLookId)
{
    lookId = newLookId;
}

int SolidEntity::getLook() const
{
    return lookId;
}

int SolidEntity::getGraphicalObject() const
{
	return graObjectId;
}

int SolidEntity::getPhysicalObject() const
{
    return phyObjectId;
}

bool SolidEntity::isBuoyant() const
{
    return buoyant;
}
    
BodyPhysicsType SolidEntity::getBodyPhysicsType() const
{
    return phyType;
}

void SolidEntity::getAABB(Vector3& min, Vector3& max)
{
    if(rigidBody != NULL)
        rigidBody->getAabb(min, max);
    else if(multibodyCollider != NULL)
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
	
	if( (rigidBody != NULL || multibodyCollider != NULL)  && isRenderable() )
	{
        Renderable item;
        
        if(dm == DisplayMode::DISPLAY_GRAPHICAL && graObjectId >= 0)
        { 
            item.type = RenderableType::SOLID;
            item.objectId = graObjectId;
            item.lookId = lookId;
            item.model = glMatrixFromTransform(getGTransform());
            items.push_back(item);
        }
        else if(dm == DisplayMode::DISPLAY_PHYSICAL && phyObjectId >= 0)
        {
            item.type = RenderableType::SOLID;
            item.objectId = phyObjectId;
            item.lookId = -1;
            item.model = glMatrixFromTransform(getCTransform());
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
#ifdef DEBUG
        //Surface crossing debug
        submerged.model = glMatrixFromTransform(sf::I4());
        items.push_back(submerged);
#else
        //Geometry approximation
        switch(fdApproxType)
        {
            case FD_APPROX_AUTO:
                break;
                
            case FD_APPROX_SPHERE:
                item.type = RenderableType::HYDRO_ELLIPSOID;
                item.model = glMatrixFromTransform(getHTransform());
                item.points.push_back(glm::vec3((GLfloat)fdApproxParams[0], (GLfloat)fdApproxParams[0], (GLfloat)fdApproxParams[0]));
                items.push_back(item);
                break;
                
            case FD_APPROX_CYLINDER:
                item.type = RenderableType::HYDRO_CYLINDER;
                item.model = glMatrixFromTransform(getHTransform());
                item.points.push_back(glm::vec3((GLfloat)fdApproxParams[0], (GLfloat)fdApproxParams[0], (GLfloat)fdApproxParams[1]));
                items.push_back(item);
                break;
                
            case FD_APPROX_ELLIPSOID:
                item.type = RenderableType::HYDRO_ELLIPSOID;
                item.model = glMatrixFromTransform(getHTransform());
                item.points.push_back(glm::vec3((GLfloat)fdApproxParams[0], (GLfloat)fdApproxParams[1], (GLfloat)fdApproxParams[2]));
                items.push_back(item);
                break;
        }
#endif
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
        item.points.push_back(cgv + glm::vec3((GLfloat)Fdl.x(), (GLfloat)Fdl.y(), (GLfloat)Fdl.z()));
        items.push_back(item);
        
        item.points.pop_back();
        item.type = RenderableType::FORCE_QUADRATIC_DRAG;
        item.points.push_back(cgv + glm::vec3((GLfloat)Fdq.x(), (GLfloat)Fdq.y(), (GLfloat)Fdq.z()));
        items.push_back(item);
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
    if(rigidBody != NULL)
    {
        Transform trans;
        rigidBody->getMotionState()->getWorldTransform(trans);
        return trans;
    }
    else if(multibodyCollider != NULL)
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
    if(rigidBody != NULL)
    {
        rigidBody->getMotionState()->setWorldTransform(trans);
    }
    else if(multibodyCollider != NULL)
    {
        multibodyCollider->setWorldTransform(trans);
    }
}

Vector3 SolidEntity::getLinearVelocity() const
{
    if(rigidBody != NULL)
    {
        return rigidBody->getLinearVelocity();
    }
    else if(multibodyCollider != NULL)
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
    if(rigidBody != NULL)
    {
        return rigidBody->getAngularVelocity();
    }
    else if(multibodyCollider != NULL)
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
    if(rigidBody != NULL)
    {
        return rigidBody->getVelocityInLocalPoint(relPos);
    }
    else if(multibodyCollider != NULL)
    {
        return getLinearVelocity() + getAngularVelocity().cross(relPos);
    }
    else
        return Vector3(0,0,0);
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

Matrix6Eigen SolidEntity::getAddedMass() const
{
    return aMass;
}

Scalar SolidEntity::getAugmentedMass() const
{
    if(phyType == BodyPhysicsType::SUBMERGED_BODY)
        return mass + (aMass(0,0) + aMass(1,1) + aMass(2,2))/Scalar(3);
    else
        return mass;
}

Vector3 SolidEntity::getAugmentedInertia() const
{
    if(phyType == BodyPhysicsType::SUBMERGED_BODY)
        return Ipri + Vector3(aMass(3,3), aMass(4,4), aMass(5,5)); 
    else
        return Ipri;
}

void SolidEntity::getGeometryApprox(GeometryApproxType& type, std::vector<Scalar>& params) const
{
    type = fdApproxType;
    params = fdApproxParams;
}

Material SolidEntity::getMaterial() const
{
    return mat;
}

std::vector<Vertex>* SolidEntity::getMeshVertices()
{
    std::vector<Vertex>* pVert = new std::vector<Vertex>(0);
    
    if(phyMesh != NULL)
        pVert->insert(pVert->end(), phyMesh->vertices.begin(), phyMesh->vertices.end());
        
    return pVert;
}

const Mesh* SolidEntity::getPhysicsMesh()
{
    return phyMesh;
}

void SolidEntity::ComputeFluidDynamicsApprox(GeometryApproxType t)
{
    switch(t)
    {
        case FD_APPROX_SPHERE:
            ComputeSphericalApprox();
            break;
            
        case FD_APPROX_CYLINDER:
            ComputeCylindricalApprox();
            break;
            
        case FD_APPROX_AUTO:
        case FD_APPROX_ELLIPSOID:
            ComputeEllipsoidalApprox();
            break;
    }
}

void SolidEntity::ComputeSphericalApprox()
{
    //Get vertices of solid
	std::vector<Vertex>* vertices = getMeshVertices();
    if(vertices->size() < 9)
    {
        delete vertices;
        return;
    }
    
	//Fill points matrix
	MatrixXEigen P(vertices->size(), 3);
	for(unsigned int i=0; i<vertices->size(); ++i)
    {
        Vector3 vpos((*vertices)[i].pos.x, (*vertices)[i].pos.y, (*vertices)[i].pos.z);
        vpos = T_CG2C * vpos;
		P.row(i) << vpos.x(), vpos.y(), vpos.z();
    }
    delete vertices; //Clear allocated memory so it doesn't leak!
    
    //Ellipsoid fit
    //Compute contraints -> axis aligned, three radii
	MatrixXEigen A(P.rows(), 4);
	A.col(0) = 2 * P.col(0);
	A.col(1) = 2 * P.col(1);
	A.col(2) = 2 * P.col(2);
	A.col(3) = MatrixXEigen::Ones(P.rows(), 1);
		
	//Solve Least-Squares problem Ax=b
	MatrixXEigen b(P.rows(), 1);
	MatrixXEigen x(A.cols(), 1);	
	//squared norm
	b = P.col(0).array() * P.col(0).array() + P.col(1).array() * P.col(1).array() + P.col(2).array() * P.col(2).array();
	//solution
	x = A.jacobiSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(b); 
    
	//Find ellipsoid parameters
	MatrixXEigen p(10, 1);
    p(0) = -1.0;
    p(1) = -1.0;
    p(2) = -1.0;
    p(3) = 0.0;
    p(4) = 0.0;
    p(5) = 0.0;
    p(6) = x(0);
    p(7) = x(1);
    p(8) = x(2);
    p(9) = x(3);
	
	MatrixXEigen E(4, 4);
	E << p(0), p(3), p(4), p(6),
		 p(3), p(1), p(5), p(7),
		 p(4), p(5), p(2), p(8),
		 p(6), p(7), p(8), p(9);
		 
	//Compute center
	MatrixXEigen c(3, 1);
	c = -E.block(0, 0, 3, 3).jacobiSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(p.block(6, 0, 3, 1));
	
	//Compute transform matrix
	Matrix4Eigen T;
	T.setIdentity();
	T.block(3, 0, 1, 3) = c.transpose();
	T = T * E * T.transpose();
	
	//Compute axes
	Eigen::SelfAdjointEigenSolver<MatrixXEigen> eigenSolver(T.block(0, 0, 3, 3)/(-T(3,3)));
	if(eigenSolver.info() != Eigen::Success) 
	{
		cError("Error computing sphere for %s!", getName().c_str());
		return;
	}
	
    //Ellipsoid radii
	MatrixXEigen r(3, 1);
	r = Eigen::sqrt(1.0/Eigen::abs(eigenSolver.eigenvalues().array()));
    
    fdApproxType = FD_APPROX_SPHERE;
    fdApproxParams.resize(1);
    fdApproxParams[0] = r(0);
    
    //Added mass and inertia
    Scalar rho = Scalar(1000);
    Scalar m = Scalar(2)*M_PI*rho*r(0)*r(0)*r(0)/Scalar(3);
    Scalar I = Scalar(0);
    
    aMass(0,0) = m;
    aMass(1,1) = m;
    aMass(2,2) = m;
    aMass(3,3) = I;
    aMass(4,4) = I;
    aMass(5,5) = I;
    
    //Set transform with respect to geometry
    Transform sphereTransform = I4();
    sphereTransform.setOrigin(Vector3(c(0), c(1), c(2)));
    T_CG2H = sphereTransform;
    
#ifdef DEBUG
    //std::cout << getName() << " added mass: " << aMass << std::endl << std::endl;
#endif
}

void SolidEntity::ComputeCylindricalApprox()
{
    //Get vertices of solid
	std::vector<Vertex>* vertices = getMeshVertices(); //Get a copy of vertices
    if(vertices->size() < 2)
    {
        delete vertices;
        return;
    }
    
    //Fill points matrix
	MatrixXEigen P(vertices->size(), 3);
	for(unsigned int i=0; i<vertices->size(); ++i)
    {
        Vector3 vpos((*vertices)[i].pos.x, (*vertices)[i].pos.y, (*vertices)[i].pos.z);
        vpos = T_CG2C * vpos;
		P.row(i) << vpos.x(), vpos.y(), vpos.z();
    }
    delete vertices; //Clear allocated memory so it doesn't leak!
    
    //Radius
    Scalar r[3] = {0,0,0};
    for(unsigned int i=0; i < P.rows(); ++i)
    {
        Scalar d;
        
        //X
        d = sqrt(P(i,1)*P(i,1) + P(i,2)*P(i,2));
        r[0] = d > r[0] ? d : r[0];
        
        //Y
        d = sqrt(P(i,0)*P(i,0) + P(i,2)*P(i,2));
        r[1] = d > r[1] ? d : r[1];
        
        //Z
        d = sqrt(P(i,0)*P(i,0) + P(i,1)*P(i,1));
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
    for(unsigned int i=0; i<P.rows(); ++i)
    {
        Scalar d = fabs(P(i,axis));
        l_2 = d > l_2 ? d : l_2;
    }
    
    fdApproxType = FD_APPROX_CYLINDER;
    fdApproxParams.resize(2);
    
    if(axis == 0) //X axis
    {
        fdApproxParams[0] = r[0]; 
        fdApproxParams[1] = l_2*Scalar(2);
        T_CG2H = Transform(Quaternion(0,M_PI_2,0), Vector3(0,0,0));
    }
    else if(axis == 1) //Y axis
    {
        fdApproxParams[0] = r[1];
        fdApproxParams[1] = l_2*Scalar(2);
        T_CG2H = Transform(Quaternion(0,0,M_PI_2), Vector3(0,0,0));
    }
    else
    {
        fdApproxParams[0] = r[2];
        fdApproxParams[1] = l_2*Scalar(2);
        T_CG2H = I4();
    }
    
    //Added mass and inertia
    Scalar rho = Scalar(1000);
    Scalar m1 = rho*M_PI*fdApproxParams[0]*fdApproxParams[0]; //Parallel to axis
    Scalar m2 = rho*M_PI*fdApproxParams[0]*fdApproxParams[0]*Scalar(2)*fdApproxParams[1]; //Perpendicular to axis
    Scalar I1 = Scalar(0);
    Scalar I2 = Scalar(1)/Scalar(12)*M_PI*rho*fdApproxParams[1]*fdApproxParams[1]*btPow(fdApproxParams[0], Scalar(3));
    
    Vector3 M = T_CG2H.getBasis() * Vector3(m2, m2, m1);
    Vector3 I = T_CG2H.getBasis() * Vector3(I2, I2, I1);
    
    aMass(0,0) = btFabs(M.x());
    aMass(1,1) = btFabs(M.y());
    aMass(2,2) = btFabs(M.z());
    aMass(3,3) = btFabs(I.x());
    aMass(4,4) = btFabs(I.y());
    aMass(5,5) = btFabs(I.z());
    
#ifdef DEBUG
    //std::cout << getName() << " added mass: " << aMass << std::endl << std::endl;
#endif
}

void SolidEntity::ComputeEllipsoidalApprox()
{
    //Get vertices of solid
	std::vector<Vertex>* vertices = getMeshVertices();
    if(vertices->size() < 9)
    {
        delete vertices;
        return;
    }
    
	//Fill points matrix
	MatrixXEigen P(vertices->size(), 3);
	for(unsigned int i=0; i<vertices->size(); ++i)
    {
        Vector3 vpos((*vertices)[i].pos.x, (*vertices)[i].pos.y, (*vertices)[i].pos.z);
        vpos = T_CG2C * vpos;
		P.row(i) << vpos.x(), vpos.y(), vpos.z();
    }
    delete vertices; //Clear allocated memory so it doesn't leak!
    
    //Ellipsoid fit
    //Compute contraints -> axis aligned, three radii
	MatrixXEigen A(P.rows(), 6);
	A.col(0) = P.col(0).array() * P.col(0).array() + P.col(1).array() * P.col(1).array() - 2 * P.col(2).array() * P.col(2).array();
	A.col(1) = P.col(0).array() * P.col(0).array() + P.col(2).array() * P.col(2).array() - 2 * P.col(1).array() * P.col(1).array();
	A.col(2) = 2 * P.col(0);
	A.col(3) = 2 * P.col(1);
	A.col(4) = 2 * P.col(2);
	A.col(5) = MatrixXEigen::Ones(P.rows(), 1);
	
	//Solve Least-Squares problem Ax=b
	MatrixXEigen b(P.rows(), 1);
	MatrixXEigen x(A.cols(), 1);	
	//squared norm
	b = P.col(0).array() * P.col(0).array() + P.col(1).array() * P.col(1).array() + P.col(2).array() * P.col(2).array();
	//solution
	//x = (A.transpose() * A).ldlt().solve(A.transpose() * b); //normal equations
	x = A.jacobiSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(b); 
    
	//Find ellipsoid parameters
	MatrixXEigen p(10, 1);
    p(0) = x(0) + x(1) - 1.0;
    p(1) = x(0) - 2*x(1) - 1.0;
    p(2) = x(1) - 2*x(0) - 1.0;
    p(3) = 0.0;
    p(4) = 0.0;
    p(5) = 0.0;
    p(6) = x(2);
    p(7) = x(3);
    p(8) = x(4);
    p(9) = x(5);
	
	MatrixXEigen E(4, 4);
	E << p(0), p(3), p(4), p(6),
		 p(3), p(1), p(5), p(7),
		 p(4), p(5), p(2), p(8),
		 p(6), p(7), p(8), p(9);
		 
	//Compute center
	MatrixXEigen c(3, 1);
	c = -E.block(0, 0, 3, 3).jacobiSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(p.block(6, 0, 3, 1));
	
	//Compute transform matrix
	Matrix4Eigen T;
	T.setIdentity();
	T.block(3, 0, 1, 3) = c.transpose();
	T = T * E * T.transpose();
	
	//Compute axes
	Eigen::SelfAdjointEigenSolver<MatrixXEigen> eigenSolver(T.block(0, 0, 3, 3)/(-T(3,3)));
	if(eigenSolver.info() != Eigen::Success) 
	{
		cError("Error computing ellipsoid for %s!", getName().c_str());
		return;
	}
	
    //Ellipsoid radii
	MatrixXEigen r(3, 1);
	r = Eigen::sqrt(1.0/Eigen::abs(eigenSolver.eigenvalues().array()));
    Vector3 ellipsoidR(r(0), r(1), r(2));
    
    //Ellipsoid axes
	MatrixXEigen axes(3, 3);
	axes = eigenSolver.eigenvectors().array();
    
    //Reorder radii
    Transform ellipsoidTransform;
    ellipsoidTransform.setIdentity();
    ellipsoidTransform.setBasis(Matrix3(axes(0,0), axes(0,1), axes(0,2), axes(1,0), axes(1,1), axes(1,2), axes(2,0), axes(2,1), axes(2,2)));
    ellipsoidR = ellipsoidTransform.getBasis() * ellipsoidR;
    
    fdApproxType = FD_APPROX_ELLIPSOID;
    fdApproxParams.resize(3);
    fdApproxParams[0] = ellipsoidR.getX();
    fdApproxParams[1] = ellipsoidR.getY();
    fdApproxParams[2] = ellipsoidR.getZ();
    
    //Compute added mass
    //Search for the longest semiaxis
    Scalar rho = Scalar(1000); //Fluid density
    Scalar r12 = (r(1) + r(2))/Scalar(2);
    Scalar m1 = LambKFactor(r(0), r12)*Scalar(4)/Scalar(3)*M_PI*rho*r(0)*r12*r12;
    Scalar m2 = Scalar(4)/Scalar(3)*M_PI*rho*r(2)*r(2)*r(0);
    Scalar m3 = Scalar(4)/Scalar(3)*M_PI*rho*r(1)*r(1)*r(0);
    Scalar I1 = Scalar(0); //THIS SHOULD BE > 0
    Scalar I2 = Scalar(1)/Scalar(12)*M_PI*rho*r(1)*r(1)*btPow(r(0), Scalar(3));
    Scalar I3 = Scalar(1)/Scalar(12)*M_PI*rho*r(2)*r(2)*btPow(r(0), Scalar(3));
    
    Vector3 M = ellipsoidTransform.getBasis() * Vector3(m1,m2,m3);
    Vector3 I = ellipsoidTransform.getBasis() * Vector3(I1,I2,I3);
    
    aMass(0,0) = M.x();
    aMass(1,1) = M.y();
    aMass(2,2) = M.z();
    aMass(3,3) = I.x();
    aMass(4,4) = I.y();
    aMass(5,5) = I.z();
    
    //Set transform with respect to geometry
    ellipsoidTransform.getBasis().setIdentity(); //Aligned with CG frame (for now)
    ellipsoidTransform.setOrigin(Vector3(c(0), c(1), c(2)));
    T_CG2H = ellipsoidTransform;
    
#ifdef DEBUG
    //std::cout << getName() << " added mass: " << aMass << std::endl << std::endl;
#endif
}

Scalar SolidEntity::LambKFactor(Scalar r1, Scalar r2)
{
    Scalar e = Scalar(1) - r2*r2/r1;
    Scalar alpha0 = Scalar(2)*(Scalar(1)-e*e)/(e*e) * (Scalar(0.5)*btLog((Scalar(1)+e)/(Scalar(1)-e)) - e);
    return alpha0/(Scalar(2)-alpha0);
}

void SolidEntity::BuildGraphicalObject()
{
	if(phyMesh == NULL || !SimulationApp::getApp()->hasGraphics())
		return;
		
	graObjectId = ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->BuildObject(phyMesh);
    phyObjectId = graObjectId;
}

void SolidEntity::BuildRigidBody()
{
    if(rigidBody == NULL)
    {
        btDefaultMotionState* motionState = new btDefaultMotionState();
        
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
        
        //Construct Bullet rigid body
        Scalar M = getAugmentedMass();
        Vector3 I = getAugmentedInertia();
        
        btRigidBody::btRigidBodyConstructionInfo rigidBodyCI(M, motionState, colShape, I);
        rigidBodyCI.m_friction = rigidBodyCI.m_rollingFriction = rigidBodyCI.m_restitution = Scalar(0.); //not used
        rigidBodyCI.m_linearDamping = rigidBodyCI.m_angularDamping = Scalar(0.); //not used
		rigidBodyCI.m_linearSleepingThreshold = rigidBodyCI.m_angularSleepingThreshold = Scalar(0.); //not used
        rigidBodyCI.m_additionalDamping = false;
        
        rigidBody = new btRigidBody(rigidBodyCI);
        rigidBody->setUserPointer(this);
        rigidBody->setFlags(rigidBody->getFlags() | BT_ENABLE_GYROSCOPIC_FORCE_IMPLICIT_BODY);
		rigidBody->setCollisionFlags(rigidBody->getCollisionFlags() | btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);
        rigidBody->setActivationState(DISABLE_DEACTIVATION);
        //rigidBody->setCcdMotionThreshold(0.01);
        //rigidBody->setCcdSweptSphereRadius(0.9);
        
        if(contactK > Scalar(0))
            rigidBody->setContactStiffnessAndDamping(contactK, contactD);
        
        cInfo("Built rigid body %s [mass: %1.3lf; inertia: %1.3lf, %1.3lf, %1.3lf; volume: %1.1lf]", getName().c_str(), mass, Ipri.x(), Ipri.y(), Ipri.z(), volume*1e6);
    }
}

void SolidEntity::BuildMultibodyLinkCollider(btMultiBody *mb, unsigned int child, btMultiBodyDynamicsWorld *world)
{
    if(multibodyCollider == NULL)
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
        
        world->addCollisionObject(multibodyCollider, MASK_DEFAULT, MASK_STATIC | MASK_DEFAULT);
        
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

void SolidEntity::AddToSimulation(SimulationManager *sm, const Transform& origin)
{
    if(rigidBody == NULL)
    {
        BuildRigidBody();
		BuildGraphicalObject();
        
        rigidBody->setMotionState(new btDefaultMotionState(origin * T_CG2O.inverse()));
        //rigidBody->setCenterOfMassTransform(origin * T_CG2O.inverse());
        sm->getDynamicsWorld()->addRigidBody(rigidBody, MASK_DEFAULT, MASK_STATIC | MASK_DEFAULT);
    }
}

void SolidEntity::RemoveFromSimulation(SimulationManager* sm)
{
    if(rigidBody != NULL)
    {
        sm->getDynamicsWorld()->removeRigidBody(rigidBody);
        delete rigidBody;
        rigidBody = NULL;
    }
}

void SolidEntity::UpdateAcceleration(Scalar dt)
{
	//Filter velocity
	Scalar alpha = 0.5;
	Vector3 currentLinearVel = alpha * getLinearVelocity() + (Scalar(1)-alpha) * filteredLinearVel;
	Vector3 currentAngularVel = alpha * getAngularVelocity() + (Scalar(1)-alpha) * filteredAngularVel;
		
	//Compute derivative
	linearAcc = (currentLinearVel - filteredLinearVel)/dt;
	angularAcc = (currentAngularVel - filteredAngularVel)/dt;
		
	//Update filtered
	filteredLinearVel = currentLinearVel;
	filteredAngularVel = currentAngularVel;
}

void SolidEntity::ApplyGravity(const Vector3& g)
{
    if(rigidBody != NULL)
    {
        rigidBody->applyCentralForce(g * mass);
    }
}

void SolidEntity::ApplyCentralForce(const Vector3& force)
{
    if(rigidBody != NULL)
        rigidBody->applyCentralForce(force);
    else if(multibodyCollider != NULL)
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
    if(rigidBody != NULL)
        rigidBody->applyTorque(torque);
    else if(multibodyCollider != NULL)
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
    
    unsigned int submerged = 0;
    if(ocn->GetDepth(aabbMin) > Scalar(0)) ++submerged;
    if(ocn->GetDepth(aabbMax) > Scalar(0)) ++submerged;
    if(ocn->GetDepth(aabbMin + Vector3(d.x(), 0, 0)) > Scalar(0)) ++submerged;
    if(ocn->GetDepth(aabbMin + Vector3(0, d.y(), 0)) > Scalar(0)) ++submerged;
    if(ocn->GetDepth(aabbMin + Vector3(d.x(), d.y(), 0)) > Scalar(0)) ++submerged;
    if(ocn->GetDepth(aabbMin + Vector3(0, 0, d.z())) > Scalar(0)) ++submerged;
    if(ocn->GetDepth(aabbMin + Vector3(d.x(), 0, d.z())) > Scalar(0)) ++submerged;
    if(ocn->GetDepth(aabbMin + Vector3(0, d.y(), d.z())) > Scalar(0)) ++submerged;
    
    if(submerged == 0)
        return BodyFluidPosition::OUTSIDE_FLUID;
    else if(submerged == 8)
        return BodyFluidPosition::INSIDE_FLUID;
    else
        return BodyFluidPosition::CROSSING_FLUID_SURFACE;
}
    
void SolidEntity::ComputeDampingForces(Vector3 vc, Vector3 fn, Scalar A, Vector3& linear, Vector3& quadratic, Vector3& skin)
{
    Vector3 vn = vc.dot(fn) * fn; //Normal velocity
    Vector3 vt = vc - vn; //Tangent velocity
    //linear = mu * vt * A / Scalar(0.0001);
    
    if(fn.dot(vn) < Scalar(0))
    {
        Scalar vmag = vn.safeNorm();
        linear = vn * btExp(-0.5*vmag*vmag) * A; //0.5*rho in CorrectDampingForces
        quadratic = vn * vmag * A; // 0.5*rho in CorrectDampingForces
    }
    else
    {
        linear = Vector3(0,0,0);
        quadratic = Vector3(0,0,0);
    }
    
    Scalar vmag = vt.safeNorm();
    skin = vt * vmag * A;
}

void SolidEntity::CorrectHydrodynamicForces(Ocean* ocn, Vector3& _Fdl, Vector3& _Tdl, Vector3& _Fdq, Vector3& _Tdq, Vector3& _Fds, Vector3& _Tds)
{
    Vector3 Fdqn = (T_CG2H.getBasis().inverse() * _Fdq).safeNormalize(); //Transform force to approximate geometry frame
    Scalar corFactor(1.0);
    
    switch(fdApproxType)
    {
        case FD_APPROX_AUTO:
        case FD_APPROX_SPHERE:
            break;
                
        case FD_APPROX_CYLINDER:
        {
            Vector3 Cd(0.5, 0.5, 1.0);
            corFactor = Cd.dot(Fdqn);
        }
            break;
                
        case FD_APPROX_ELLIPSOID:
        {
            Vector3 Cd(Scalar(1)/fdApproxParams[0] , Scalar(1)/fdApproxParams[1], Scalar(1)/fdApproxParams[2]);
            Scalar maxCd = btMax(btMax(Cd.x(), Cd.y()), Cd.z());
            Cd /= maxCd;
            corFactor = Cd.dot(Fdqn);
        }
            break;
    }
    
    //corFactor *= 2.0;
    
    _Fdl *= 0.1 * btFabs(corFactor) * 0.5 * ocn->getLiquid().density;
    _Tdl *= 0.1 * btFabs(corFactor) * 0.5 * ocn->getLiquid().density;
    _Fdq *= btFabs(corFactor) * 0.5 * ocn->getLiquid().density;
    _Tdq *= btFabs(corFactor) * 0.5 * ocn->getLiquid().density;
    _Fds *= 0.1 * 0.5 * ocn->getLiquid().density;
    _Tds *= 0.1 * 0.5 * ocn->getLiquid().density;
}

void SolidEntity::ComputeHydrodynamicForcesSurface(const HydrodynamicsSettings& settings, const Mesh* mesh, Ocean* ocn, const Transform& T_CG, const Transform& T_C,
                                            const Vector3& v, const Vector3& omega, Vector3& _Fb, Vector3& _Tb, Vector3& _Fdl, Vector3& _Tdl, Vector3& _Fdq, Vector3& _Tdq, Vector3& _Fds, Vector3& _Tds, Renderable& debug)
{
    //Buoyancy
    if(settings.reallisticBuoyancy)
    {
        _Fb.setZero();
        _Tb.setZero();
    }
    
    //Damping forces
    if(settings.dampingForces)
    {
        _Fdl.setZero();
        _Tdl.setZero();
        _Fdq.setZero();
        _Tdq.setZero();
        _Fds.setZero();
        _Tds.setZero();
    }
	
    //Set zeros
	if(mesh == NULL) return;
    
#ifdef DEBUG    
    uint64_t start = GetTimeInMicroseconds();
#endif
      
    //Calculate fluid dynamics forces and torques
    Vector3 p = T_CG.getOrigin();
 
	//Loop through all faces...
    for(size_t i=0; i<mesh->faces.size(); ++i)
    {
        //Global coordinates
		glm::vec3 p1gl = mesh->vertices[mesh->faces[i].vertexID[0]].pos;
		glm::vec3 p2gl = mesh->vertices[mesh->faces[i].vertexID[1]].pos;
		glm::vec3 p3gl = mesh->vertices[mesh->faces[i].vertexID[2]].pos;
        Vector3 p1 = T_C * Vector3(p1gl.x,p1gl.y,p1gl.z);
        Vector3 p2 = T_C * Vector3(p2gl.x,p2gl.y,p2gl.z);
        Vector3 p3 = T_C * Vector3(p3gl.x,p3gl.y,p3gl.z);
        
        //Check if face underwater
        Scalar depth[3];
        depth[0] = ocn->GetDepth(p1);
        depth[1] = ocn->GetDepth(p2);
        depth[2] = ocn->GetDepth(p3);
        
        if(depth[0] < Scalar(0) && depth[1] < Scalar(0) && depth[2] < Scalar(0))
            continue;
        
        //Calculate face properties
        Vector3 fc;
        Vector3 fn;
        Vector3 fn1;
        Scalar A;
        
        if(depth[0] < Scalar(0)) //Vertex 1 above water
        {
            if(depth[1] < Scalar(0)) //Two vertices above water (triangle)
            {
                p1 = p3 + (p1-p3) * (depth[2]/(btFabs(depth[0]) + depth[2]));
                p2 = p3 + (p2-p3) * (depth[2]/(btFabs(depth[1]) + depth[2]));
                //p3 without change
                
                //Calculate
                Vector3 fv1 = p2-p1; //One side of the face (triangle)
                Vector3 fv2 = p3-p1; //Another side of the face (triangle)
                fc = (p1+p2+p3)/Scalar(3); //Face centroid
        
                fn = fv1.cross(fv2); //Normal of the face (length != 1)
                Scalar len = fn.length();
                
                if(btFuzzyZero(len)) //Check for invalid triangle
                    continue;
                
                fn1 = fn/len; //Normalised normal (length = 1)
                A = len/Scalar(2); //Area of the face (triangle)                
#ifdef DEBUG
                debug.points.push_back(glm::vec3((GLfloat)p1.x(), (GLfloat)p1.y(), (GLfloat)p1.z()));
                debug.points.push_back(glm::vec3((GLfloat)p2.x(), (GLfloat)p2.y(), (GLfloat)p2.z()));
                debug.points.push_back(glm::vec3((GLfloat)p2.x(), (GLfloat)p2.y(), (GLfloat)p2.z()));
                debug.points.push_back(glm::vec3((GLfloat)p3.x(), (GLfloat)p3.y(), (GLfloat)p3.z()));
                debug.points.push_back(glm::vec3((GLfloat)p3.x(), (GLfloat)p3.y(), (GLfloat)p3.z()));
                debug.points.push_back(glm::vec3((GLfloat)p1.x(), (GLfloat)p1.y(), (GLfloat)p1.z()));
#endif
            }
            else if(depth[2] < Scalar(0)) //Two vertices above water (triangle)
            {
                p1 = p2 + (p1-p2) * (depth[1]/(btFabs(depth[0]) + depth[1]));
                //p2 without change
                p3 = p2 + (p3-p2) * (depth[1]/(btFabs(depth[2]) + depth[1]));
                
                //Calculate
                Vector3 fv1 = p2-p1; //One side of the face (triangle)
                Vector3 fv2 = p3-p1; //Another side of the face (triangle)
                fc = (p1+p2+p3)/Scalar(3); //Face centroid
        
                fn = fv1.cross(fv2); //Normal of the face (length != 1)
                Scalar len = fn.length();
                
                if(btFuzzyZero(len)) //Check for invalid triangle
                    continue;
                
                fn1 = fn/len; //Normalised normal (length = 1)
                A = len/Scalar(2); //Area of the face (triangle)         
#ifdef DEBUG
                debug.points.push_back(glm::vec3((GLfloat)p1.x(), (GLfloat)p1.y(), (GLfloat)p1.z()));
                debug.points.push_back(glm::vec3((GLfloat)p2.x(), (GLfloat)p2.y(), (GLfloat)p2.z()));
                debug.points.push_back(glm::vec3((GLfloat)p2.x(), (GLfloat)p2.y(), (GLfloat)p2.z()));
                debug.points.push_back(glm::vec3((GLfloat)p3.x(), (GLfloat)p3.y(), (GLfloat)p3.z()));
                debug.points.push_back(glm::vec3((GLfloat)p3.x(), (GLfloat)p3.y(), (GLfloat)p3.z()));
                debug.points.push_back(glm::vec3((GLfloat)p1.x(), (GLfloat)p1.y(), (GLfloat)p1.z()));
#endif
            }
            else //depth[1] >= 0 && depth[2] >= 0 --> Two vertices under water (quad = two triangles)
            {
                //Quad!!!!
                Vector3 p1temp = p2 + (p1-p2) * (depth[1]/(btFabs(depth[0]) + depth[1]));
                //p2 without change
                //p3 without change
                Vector3 p4 = p3 + (p1-p3) * (depth[2]/(btFabs(depth[0]) + depth[2]));
                p1 = p1temp;
                
                //Calculate
                Vector3 fv1 = p2-p1;
                Vector3 fv2 = p4-p1;
                Vector3 fv3 = p2-p3;
                Vector3 fv4 = p4-p3;
                
                fc = (p1 + p2 + p3 + p4)/Scalar(4);
                fn = fv1.cross(fv2);
                Scalar len = fn.length();
                
                if(btFuzzyZero(len)) //Check validity
                    continue;
                
                fn1 = fn/len;
                A = (len + fv3.cross(fv4).length())/Scalar(2); //Quad
                fn = fn1 * A;
#ifdef DEBUG
                debug.points.push_back(glm::vec3((GLfloat)p1.x(), (GLfloat)p1.y(), (GLfloat)p1.z()));
                debug.points.push_back(glm::vec3((GLfloat)p2.x(), (GLfloat)p2.y(), (GLfloat)p2.z()));
                debug.points.push_back(glm::vec3((GLfloat)p2.x(), (GLfloat)p2.y(), (GLfloat)p2.z()));
                debug.points.push_back(glm::vec3((GLfloat)p3.x(), (GLfloat)p3.y(), (GLfloat)p3.z()));
                debug.points.push_back(glm::vec3((GLfloat)p3.x(), (GLfloat)p3.y(), (GLfloat)p3.z()));
                debug.points.push_back(glm::vec3((GLfloat)p4.x(), (GLfloat)p4.y(), (GLfloat)p4.z()));
                debug.points.push_back(glm::vec3((GLfloat)p4.x(), (GLfloat)p4.y(), (GLfloat)p4.z()));
                debug.points.push_back(glm::vec3((GLfloat)p1.x(), (GLfloat)p1.y(), (GLfloat)p1.z()));
#endif  
            }
        }
        else if(depth[1] < Scalar(0))
        {
            if(depth[2] < Scalar(0))
            {
                //p1 without change
                p2 = p1 + (p2-p1) * (depth[0]/(btFabs(depth[1]) + depth[0]));
                p3 = p1 + (p3-p1) * (depth[0]/(btFabs(depth[2]) + depth[0]));
                
                //Calculate
                Vector3 fv1 = p2-p1; //One side of the face (triangle)
                Vector3 fv2 = p3-p1; //Another side of the face (triangle)
                fc = (p1+p2+p3)/Scalar(3); //Face centroid
        
                fn = fv1.cross(fv2); //Normal of the face (length != 1)
                Scalar len = fn.length();
                
                if(btFuzzyZero(len))
                    continue;
                
                fn1 = fn/len; //Normalised normal (length = 1)
                A = len/Scalar(2); //Area of the face (triangle)
#ifdef DEBUG
                debug.points.push_back(glm::vec3((GLfloat)p1.x(), (GLfloat)p1.y(), (GLfloat)p1.z()));
                debug.points.push_back(glm::vec3((GLfloat)p2.x(), (GLfloat)p2.y(), (GLfloat)p2.z()));
                debug.points.push_back(glm::vec3((GLfloat)p2.x(), (GLfloat)p2.y(), (GLfloat)p2.z()));
                debug.points.push_back(glm::vec3((GLfloat)p3.x(), (GLfloat)p3.y(), (GLfloat)p3.z()));
                debug.points.push_back(glm::vec3((GLfloat)p3.x(), (GLfloat)p3.y(), (GLfloat)p3.z()));
                debug.points.push_back(glm::vec3((GLfloat)p1.x(), (GLfloat)p1.y(), (GLfloat)p1.z()));
#endif                
            }
            else
            {
                //Quad!!!!
                //p1 without change
                Vector3 p2temp = p1 + (p2-p1) * (depth[0]/(btFabs(depth[1]) + depth[0]));
                //p3 without change
                Vector3 p4 = p3 + (p2-p3) * (depth[2]/(btFabs(depth[1]) + depth[2]));
                p2 = p2temp;
                
                //Calculate
                Vector3 fv1 = p2-p1;
                Vector3 fv2 = p3-p1;
                Vector3 fv3 = p2-p3;
                Vector3 fv4 = p4-p3;
                
                fc = (p1 + p2 + p3 + p4)/Scalar(4);
                fn = fv1.cross(fv2); //Triangle 1
                Scalar len = fn.length();
                
                if(btFuzzyZero(len)) //Check validity
                    continue;
                
                fn1 = fn/len;
                A = (len + fv3.cross(fv4).length())/Scalar(2); //Quad
                fn = fn1 * A;
#ifdef DEBUG
                debug.points.push_back(glm::vec3((GLfloat)p1.x(), (GLfloat)p1.y(), (GLfloat)p1.z()));
                debug.points.push_back(glm::vec3((GLfloat)p2.x(), (GLfloat)p2.y(), (GLfloat)p2.z()));
                debug.points.push_back(glm::vec3((GLfloat)p2.x(), (GLfloat)p2.y(), (GLfloat)p2.z()));
                debug.points.push_back(glm::vec3((GLfloat)p4.x(), (GLfloat)p4.y(), (GLfloat)p4.z()));
                debug.points.push_back(glm::vec3((GLfloat)p4.x(), (GLfloat)p4.y(), (GLfloat)p4.z()));
                debug.points.push_back(glm::vec3((GLfloat)p3.x(), (GLfloat)p3.y(), (GLfloat)p3.z()));
                debug.points.push_back(glm::vec3((GLfloat)p3.x(), (GLfloat)p3.y(), (GLfloat)p3.z()));
                debug.points.push_back(glm::vec3((GLfloat)p1.x(), (GLfloat)p1.y(), (GLfloat)p1.z()));
#endif                 
            }
        }
        else if(depth[2] < Scalar(0))
        {
            //Quad!!!!
            //p1 without change
            //p2 without change
            Vector3 p3temp = p2 + (p3-p2) * (depth[1]/(btFabs(depth[2]) + depth[1]));
            Vector3 p4 = p1 + (p3-p1) * (depth[0]/(btFabs(depth[2]) + depth[0]));
            p3 = p3temp;
                
            //Calculate
            Vector3 fv1 = p2-p1;
            Vector3 fv2 = p4-p1;
            Vector3 fv3 = p2-p3;
            Vector3 fv4 = p4-p3;
                
            fc = (p1 + p2 + p3 + p4)/Scalar(4);
            fn = fv1.cross(fv2);
            Scalar len = fn.length();
            
            if(btFuzzyZero(len)) //Check validity
                continue;
            
            fn1 = fn/len;
            A = (len + fv3.cross(fv4).length())/Scalar(2); //Quad
            fn = fn1 * A;
#ifdef DEBUG
            debug.points.push_back(glm::vec3((GLfloat)p1.x(), (GLfloat)p1.y(), (GLfloat)p1.z()));
            debug.points.push_back(glm::vec3((GLfloat)p2.x(), (GLfloat)p2.y(), (GLfloat)p2.z()));
            debug.points.push_back(glm::vec3((GLfloat)p2.x(), (GLfloat)p2.y(), (GLfloat)p2.z()));
            debug.points.push_back(glm::vec3((GLfloat)p3.x(), (GLfloat)p3.y(), (GLfloat)p3.z()));
            debug.points.push_back(glm::vec3((GLfloat)p3.x(), (GLfloat)p3.y(), (GLfloat)p3.z()));
            debug.points.push_back(glm::vec3((GLfloat)p4.x(), (GLfloat)p4.y(), (GLfloat)p4.z()));
            debug.points.push_back(glm::vec3((GLfloat)p4.x(), (GLfloat)p4.y(), (GLfloat)p4.z()));
            debug.points.push_back(glm::vec3((GLfloat)p1.x(), (GLfloat)p1.y(), (GLfloat)p1.z()));
#endif             
        }
        else //All underwater
        {
            Vector3 fv1 = p2-p1; //One side of the face (triangle)
            Vector3 fv2 = p3-p1; //Another side of the face (triangle)
            fc = (p1+p2+p3)/Scalar(3); //Face centroid
        
            fn = fv1.cross(fv2); //Normal of the face (length != 1)
            Scalar len = fn.length();
            
            if(btFuzzyZero(len))
                continue;
            
            fn1 = fn/len; //Normalised normal (length = 1)
            A = len/Scalar(2); //Area of the face (triangle)
#ifdef DEBUGG
            debug.points.push_back(glm::vec3((GLfloat)p1.x(), (GLfloat)p1.y(), (GLfloat)p1.z()));
            debug.points.push_back(glm::vec3((GLfloat)p2.x(), (GLfloat)p2.y(), (GLfloat)p2.z()));
            debug.points.push_back(glm::vec3((GLfloat)p2.x(), (GLfloat)p2.y(), (GLfloat)p2.z()));
            debug.points.push_back(glm::vec3((GLfloat)p3.x(), (GLfloat)p3.y(), (GLfloat)p3.z()));
            debug.points.push_back(glm::vec3((GLfloat)p3.x(), (GLfloat)p3.y(), (GLfloat)p3.z()));
            debug.points.push_back(glm::vec3((GLfloat)p1.x(), (GLfloat)p1.y(), (GLfloat)p1.z()));
#endif             
        }
        
        Scalar pressure = ocn->GetPressure(fc);
        
        //Buoyancy force
        if(settings.reallisticBuoyancy)
        {
            Vector3 Fbi = -fn1 * A * pressure; //Buoyancy force per face (based on pressure)
            
            //Accumulate
            _Fb += Fbi;
            _Tb += (fc - p).cross(Fbi);
        }
        
        //Damping force
        if(settings.dampingForces)
		{
            Vector3 vc = ocn->GetFluidVelocity(fc) - (v + omega.cross(fc - p)); //Water velocity at face center
            Vector3 Fdlf;
            Vector3 Fdqf;
            Vector3 Fdsf;
            ComputeDampingForces(vc, fn1, A, Fdlf, Fdqf, Fdsf);
            
            //Accumulate
            _Fdl += Fdlf;
            _Tdl += (fc - p).cross(Fdlf);
            _Fdq += Fdqf;
            _Tdq += (fc - p).cross(Fdqf);
            _Fds += Fdsf;
            _Tds += (fc - p).cross(Fdsf);
        }
    }
    
#ifdef DEBUG
    uint64_t elapsed = GetTimeInMicroseconds() - start;
    //std::cout << getName() << ": " <<  elapsed << std::endl; 
#endif
}

void SolidEntity::ComputeHydrodynamicForcesSubmerged(const Mesh* mesh, Ocean* ocn, const Transform& T_CG, const Transform& T_C,
                                              const Vector3& v, const Vector3& omega, Vector3& _Fdl, Vector3& _Tdl, Vector3& _Fdq, Vector3& _Tdq, Vector3& _Fds, Vector3& _Tds)
{
    if(mesh == NULL) return;
    
#ifdef DEBUG
    uint64_t start = GetTimeInMicroseconds();
#endif
    
    //Damping forces
    _Fdl.setZero();
    _Tdl.setZero();
    _Fdq.setZero();
    _Tdq.setZero();
    _Fds.setZero();
    _Tds.setZero();
    
    //Calculate fluid dynamics forces and torques
    Vector3 p = T_CG.getOrigin();
    
    //Loop through all faces...
    for(unsigned int i=0; i<mesh->faces.size(); ++i)
    {
        //Global coordinates
        glm::vec3 p1gl = mesh->vertices[mesh->faces[i].vertexID[0]].pos;
        glm::vec3 p2gl = mesh->vertices[mesh->faces[i].vertexID[1]].pos;
        glm::vec3 p3gl = mesh->vertices[mesh->faces[i].vertexID[2]].pos;
        Vector3 p1 = T_C * Vector3(p1gl.x,p1gl.y,p1gl.z);
        Vector3 p2 = T_C * Vector3(p2gl.x,p2gl.y,p2gl.z);
        Vector3 p3 = T_C * Vector3(p3gl.x,p3gl.y,p3gl.z);
        
        //Calculate face properties
        Vector3 fv1 = p2-p1; //One side of the face (triangle)
        Vector3 fv2 = p3-p1; //Another side of the face (triangle)
        Vector3 fn = fv1.cross(fv2); //Normal of the face (length != 1)
        Scalar len = fn.safeNorm();
        
        if(len == Scalar(0)) continue; //If triangle is incorrect (two sides parallel)
        
        Vector3 fc = (p1+p2+p3)/Scalar(3); //Face centroid
        Vector3 fn1 = fn/len; //Normalised normal (length = 1)
        Scalar A = len/Scalar(2); //Area of the face (triangle)
        
        //Damping forces
        Vector3 vc = ocn->GetFluidVelocity(fc) - (v + omega.cross(fc - p)); //Water velocity at face center
        Vector3 Fdlf;
        Vector3 Fdqf;
        Vector3 Fdsf;
        ComputeDampingForces(vc, fn1, A, Fdlf, Fdqf, Fdsf);
        
        //Accumulate
        _Fdl += Fdlf;
        _Tdl += (fc - p).cross(Fdlf);
        _Fdq += Fdqf;
        _Tdq += (fc - p).cross(Fdqf);
        _Fds += Fdsf;
        _Tds += (fc - p).cross(Fdsf);
        
    }
    
#ifdef DEBUG
    uint64_t elapsed = GetTimeInMicroseconds() - start;
    //std::cout << getName() << ": " <<  elapsed << std::endl;
#endif
}

void SolidEntity::ComputeHydrodynamicForces(HydrodynamicsSettings settings, Ocean* ocn)
{
    if(phyType != BodyPhysicsType::FLOATING_BODY && phyType != BodyPhysicsType::SUBMERGED_BODY) return;
    
#ifdef DEBUG
    submerged.points.clear();
#endif
    
    BodyFluidPosition bf = CheckBodyFluidPosition(ocn);
    
    //If completely outside fluid just set all torques and forces to 0
    if(bf == BodyFluidPosition::OUTSIDE_FLUID)
    {
        Fb.setZero();
        Tb.setZero();
        Fdl.setZero();
        Tdl.setZero();
        Fdq.setZero();
        Tdq.setZero();
        Fds.setZero();
        Tds.setZero();
        return;
    }
    
    //Get velocities and transformations
    Vector3 v = getLinearVelocity();
    Vector3 omega = getAngularVelocity();
	
    //Check if fully submerged --> simplifies buoyancy calculation
    if(bf == BodyFluidPosition::INSIDE_FLUID)
    {
        //Compute buoyancy based on CB position
        if(isBuoyant())
        {
            Fb = -volume*ocn->getLiquid().density * SimulationApp::getApp()->getSimulationManager()->getGravity();
            Tb = (getCGTransform() * P_CB - getCGTransform().getOrigin()).cross(Fb);
        }
        
        if(settings.dampingForces)
            ComputeHydrodynamicForcesSubmerged(getPhysicsMesh(), ocn, getCGTransform(), getCTransform(), v, omega, Fdl, Tdl, Fdq, Tdq, Fds, Tds);
    }
    else //CROSSING_FLUID_SURFACE
    {
        if(!isBuoyant()) settings.reallisticBuoyancy = false;
        ComputeHydrodynamicForcesSurface(settings, getPhysicsMesh(), ocn, getCGTransform(), getCTransform(), v, omega, Fb, Tb, Fdl, Tdl, Fdq, Tdq, Fds, Tds, submerged);
    }
    
    if(settings.dampingForces)
        CorrectHydrodynamicForces(ocn, Fdl, Tdl, Fdq, Tdq, Fds, Tds);
}

void SolidEntity::ComputeAerodynamicForces(Atmosphere* atm)
{
    if(phyType != BodyPhysicsType::AERODYNAMIC_BODY) return;
    
    //Get velocities and transformations
    Vector3 v = getLinearVelocity();
    Vector3 omega = getAngularVelocity();
	
    //Compute drag
    ComputeAerodynamicForces(getPhysicsMesh(), atm, getCGTransform(), getCTransform(), v, omega, Fda, Tda);
    CorrectAerodynamicForces(atm, Fda, Tda);
}

void SolidEntity::ComputeAerodynamicForces(const Mesh* mesh, Atmosphere* atm, const Transform& T_CG, const Transform& T_C,
                                           const Vector3& v, const Vector3& omega, Vector3& _Fda, Vector3& _Tda)
{
    if(mesh == NULL) return;
        
    _Fda.setZero();
    _Tda.setZero();
    
    //Calculate fluid dynamics forces and torques
    Vector3 p = T_CG.getOrigin();
    Scalar density = atm->getGas().density;
  
    //Loop through all faces...
    for(size_t i=0; i<mesh->faces.size(); ++i)
    {
        //Global coordinates
        glm::vec3 p1gl = mesh->vertices[mesh->faces[i].vertexID[0]].pos;
        glm::vec3 p2gl = mesh->vertices[mesh->faces[i].vertexID[1]].pos;
        glm::vec3 p3gl = mesh->vertices[mesh->faces[i].vertexID[2]].pos;
        Vector3 p1 = T_C * Vector3(p1gl.x,p1gl.y,p1gl.z);
        Vector3 p2 = T_C * Vector3(p2gl.x,p2gl.y,p2gl.z);
        Vector3 p3 = T_C * Vector3(p3gl.x,p3gl.y,p3gl.z);
        
        //Calculate face properties
        Vector3 fv1 = p2-p1; //One side of the face (triangle)
        Vector3 fv2 = p3-p1; //Another side of the face (triangle)
        Vector3 fn = fv1.cross(fv2); //Normal of the face (length != 1)
        Scalar len = fn.safeNorm();
        
        if(len == Scalar(0)) continue; //If triangle is incorrect (two sides parallel)
        
        Vector3 fc = (p1+p2+p3)/Scalar(3); //Face centroid
        Vector3 fn1 = fn/len; //Normalised normal (length = 1)
        Scalar A = len/Scalar(2); //Area of the face (triangle)
        
        //Damping forces
        Vector3 vc = atm->GetFluidVelocity(fc) - (v + omega.cross(fc - p)); //Air velocity at face center
        Vector3 vn = vc.dot(fn1) * fn1; //Normal velocity
        Vector3 Fdaf(0,0,0);
        
        if(fn1.dot(vn) < Scalar(0))
            Fdaf = vn * vn.safeNorm() * A;
        
        //Accumulate
        _Fda += Fdaf;
        _Tda += (fc - p).cross(Fdaf);
    }
    
    _Fda *= Scalar(0.5) * density;
    _Tda *= Scalar(0.5) * density;
}

void SolidEntity::CorrectAerodynamicForces(Atmosphere* atm, Vector3& _Fda, Vector3& _Tda)
{
     //Correct forces
    Vector3 Fdan = (T_CG2H.getBasis().inverse() * _Fda).safeNormalize(); //Transform force to proxy frame
    Scalar corFactor(1.0);
    
    switch(fdApproxType)
    {
        case FD_APPROX_AUTO:
        case FD_APPROX_SPHERE:
            break;
                
        case FD_APPROX_CYLINDER:
        {
            Vector3 Cd(0.5, 0.5, 1.0);
            corFactor = Cd.dot(Fdan);
        }
            break;
                
        case FD_APPROX_ELLIPSOID:
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
    ApplyCentralForce(Fb + Fdl + Fdq + Fds);
    //printf("Fb: %1.3lf, %1.3lf, %1.3lf\n", Fb.x(), Fb.y(), Fb.z());
    ApplyTorque(Tb + Tdq + Tdl + Tds);
}

void SolidEntity::ApplyAerodynamicForces()
{
    ApplyCentralForce(Fda);
    ApplyTorque(Tda);
}
    
void SolidEntity::ComputePhysicalProperties(Mesh *mesh, Scalar wallThickness, Material mat, Vector3& CG, Scalar& volume, Vector3& Ipri, Matrix3& Irot)
{
    //1.Calculate mesh volume and CG
    CG = Vector3(0,0,0);
    volume = 0;
    
    if(wallThickness > Scalar(0)) //Shell
    {
        for(size_t i=0; i<mesh->faces.size(); ++i)
        {
            //Get triangle, convert from OpenGL to physics
            glm::vec3 v1gl = mesh->vertices[mesh->faces[i].vertexID[0]].pos;
            glm::vec3 v2gl = mesh->vertices[mesh->faces[i].vertexID[1]].pos;
            glm::vec3 v3gl = mesh->vertices[mesh->faces[i].vertexID[2]].pos;
            Vector3 v1(v1gl.x,v1gl.y,v1gl.z);
            Vector3 v2(v2gl.x,v2gl.y,v2gl.z);
            Vector3 v3(v3gl.x,v3gl.y,v3gl.z);
            
            //Calculate volume of shell triangle
            Scalar A = (v2-v1).cross(v3-v1).length()/Scalar(2);
            Vector3 triCG = (v1+v2+v3)/Scalar(3);
            Scalar triVolume = A * wallThickness;
            CG += triCG * triVolume;
            volume += triVolume;
        }
        
        //Compute mesh CG
        if(volume > Scalar(0))
            CG /= volume;
        else
            CG = Vector3(0,0,0);
    }
    else //Solid body
    {
        for(size_t i=0; i<mesh->faces.size(); ++i)
        {
            //Get triangle, convert from OpenGL to physics
            glm::vec3 v1gl = mesh->vertices[mesh->faces[i].vertexID[0]].pos;
            glm::vec3 v2gl = mesh->vertices[mesh->faces[i].vertexID[1]].pos;
            glm::vec3 v3gl = mesh->vertices[mesh->faces[i].vertexID[2]].pos;
            Vector3 v1(v1gl.x,v1gl.y,v1gl.z);
            Vector3 v2(v2gl.x,v2gl.y,v2gl.z);
            Vector3 v3(v3gl.x,v3gl.y,v3gl.z);
            
            //Calculate signed volume of a tetrahedra
            Vector3 tetraCG = (v1+v2+v3)/Scalar(4);
            Scalar tetraVolume6 = v1.dot(v2.cross(v3));
            CG += tetraCG * tetraVolume6;
            volume += tetraVolume6;
        }
        
        //Compute mesh CG
        if(volume > Scalar(0))
            CG /= volume;
        else
            CG = Vector3(0,0,0);
        
        //Compute mesh volume
        volume /= Scalar(6);
    }
    
    //2.Calculate moments of inertia for local coordinate system located in CG (not necessarily principal)
    Matrix3 I;
    
    if(wallThickness > Scalar(0)) //Shell - I have doubts if it is correct!
    {
        //Compute properties a shell by subtracting the inner solid from the outer solid
        //Outer solid -> eternal surface
        Scalar Pxx = Scalar(0);
        Scalar Pyy = Scalar(0);
        Scalar Pzz = Scalar(0);
        Scalar Pxy = Scalar(0);
        Scalar Pxz = Scalar(0);
        Scalar Pyz = Scalar(0);
        
        for(size_t i=0; i<mesh->faces.size(); ++i)
        {
            //Triangle verticies with respect to CG
            glm::vec3 v1gl = mesh->vertices[mesh->faces[i].vertexID[0]].pos;
            glm::vec3 v2gl = mesh->vertices[mesh->faces[i].vertexID[1]].pos;
            glm::vec3 v3gl = mesh->vertices[mesh->faces[i].vertexID[2]].pos;
            
            Vector3 v1(v1gl.x,v1gl.y,v1gl.z);
            Vector3 v2(v2gl.x,v2gl.y,v2gl.z);
            Vector3 v3(v3gl.x,v3gl.y,v3gl.z);
            Vector3 n = (v2-v1).cross(v3-v1).normalize();
            v1 = v1 + n*wallThickness/Scalar(2) - CG;
            v2 = v2 + n*wallThickness/Scalar(2) - CG;
            v3 = v3 + n*wallThickness/Scalar(2) - CG;
            
            //Pjk = const * dV * (2*Aj*Ak + 2*Bj*Bk + 2*Cj*Ck + Aj*Bk + Ak*Bj + Aj*Ck + Ak*Cj + Bj*Ck + Bk*Cj)
            Scalar V6 = v1.dot(v2.cross(v3));
            Pxx += V6 * 2 *(v1.x()*v1.x() + v2.x()*v2.x() + v3.x()*v3.x() + v1.x()*v2.x() + v1.x()*v3.x() + v2.x()*v3.x());
            Pyy += V6 * 2 *(v1.y()*v1.y() + v2.y()*v2.y() + v3.y()*v3.y() + v1.y()*v2.y() + v1.y()*v3.y() + v2.y()*v3.y());
            Pzz += V6 * 2 *(v1.z()*v1.z() + v2.z()*v2.z() + v3.z()*v3.z() + v1.z()*v2.z() + v1.z()*v3.z() + v2.z()*v3.z());
            Pxy += V6 * (2*(v1.x()*v1.y() + v2.x()*v2.y() + v3.x()*v3.y()) + v1.x()*v2.y() + v1.y()*v2.x() + v1.x()*v3.y() + v1.y()*v3.x() + v2.x()*v3.y() + v2.y()*v3.x());
            Pxz += V6 * (2*(v1.x()*v1.z() + v2.x()*v2.z() + v3.x()*v3.z()) + v1.x()*v2.z() + v1.z()*v2.x() + v1.x()*v3.z() + v1.z()*v3.x() + v2.x()*v3.z() + v2.z()*v3.x());
            Pyz += V6 * (2*(v1.y()*v1.z() + v2.y()*v2.z() + v3.y()*v3.z()) + v1.y()*v2.z() + v1.z()*v2.y() + v1.y()*v3.z() + v1.z()*v3.y() + v2.y()*v3.z() + v2.z()*v3.y());
        }
        
        Pxx *= mat.density / Scalar(120); //20 from formula and 6 from polyhedron volume
        Pyy *= mat.density / Scalar(120);
        Pzz *= mat.density / Scalar(120);
        Pxy *= mat.density / Scalar(120);
        Pxz *= mat.density / Scalar(120);
        Pyz *= mat.density / Scalar(120);
        
        I = Matrix3(Pyy+Pzz, -Pxy, -Pxz, -Pxy, Pxx+Pzz, -Pyz, -Pxz, -Pyz, Pxx+Pyy);
        
        //Inner solid -> internal surface
        Pxx = Scalar(0);
        Pyy = Scalar(0);
        Pzz = Scalar(0);
        Pxy = Scalar(0);
        Pxz = Scalar(0);
        Pyz = Scalar(0); //products of inertia
        
        for(unsigned int i=0; i<mesh->faces.size(); ++i)
        {
            //Triangle verticies with respect to CG
            glm::vec3 v1gl = mesh->vertices[mesh->faces[i].vertexID[0]].pos;
            glm::vec3 v2gl = mesh->vertices[mesh->faces[i].vertexID[1]].pos;
            glm::vec3 v3gl = mesh->vertices[mesh->faces[i].vertexID[2]].pos;
            
            Vector3 v1(v1gl.x,v1gl.y,v1gl.z);
            Vector3 v2(v2gl.x,v2gl.y,v2gl.z);
            Vector3 v3(v3gl.x,v3gl.y,v3gl.z);
            Vector3 n = (v2-v1).cross(v3-v1).normalize();
            v1 = v1 - n*wallThickness/Scalar(2) - CG;
            v2 = v2 - n*wallThickness/Scalar(2) - CG;
            v3 = v3 - n*wallThickness/Scalar(2) - CG;
            
            //Pjk = const * dV * (2*Aj*Ak + 2*Bj*Bk + 2*Cj*Ck + Aj*Bk + Ak*Bj + Aj*Ck + Ak*Cj + Bj*Ck + Bk*Cj)
            Scalar V6 = v1.dot(v2.cross(v3));
            Pxx += V6 * 2 *(v1.x()*v1.x() + v2.x()*v2.x() + v3.x()*v3.x() + v1.x()*v2.x() + v1.x()*v3.x() + v2.x()*v3.x());
            Pyy += V6 * 2 *(v1.y()*v1.y() + v2.y()*v2.y() + v3.y()*v3.y() + v1.y()*v2.y() + v1.y()*v3.y() + v2.y()*v3.y());
            Pzz += V6 * 2 *(v1.z()*v1.z() + v2.z()*v2.z() + v3.z()*v3.z() + v1.z()*v2.z() + v1.z()*v3.z() + v2.z()*v3.z());
            Pxy += V6 * (2*(v1.x()*v1.y() + v2.x()*v2.y() + v3.x()*v3.y()) + v1.x()*v2.y() + v1.y()*v2.x() + v1.x()*v3.y() + v1.y()*v3.x() + v2.x()*v3.y() + v2.y()*v3.x());
            Pxz += V6 * (2*(v1.x()*v1.z() + v2.x()*v2.z() + v3.x()*v3.z()) + v1.x()*v2.z() + v1.z()*v2.x() + v1.x()*v3.z() + v1.z()*v3.x() + v2.x()*v3.z() + v2.z()*v3.x());
            Pyz += V6 * (2*(v1.y()*v1.z() + v2.y()*v2.z() + v3.y()*v3.z()) + v1.y()*v2.z() + v1.z()*v2.y() + v1.y()*v3.z() + v1.z()*v3.y() + v2.y()*v3.z() + v2.z()*v3.y());
        }
        
        Pxx *= mat.density / Scalar(120); //20 from formula and 6 from polyhedron volume
        Pyy *= mat.density / Scalar(120);
        Pzz *= mat.density / Scalar(120);
        Pxy *= mat.density / Scalar(120);
        Pxz *= mat.density / Scalar(120);
        Pyz *= mat.density / Scalar(120);
        
        I -= Matrix3(Pyy+Pzz, -Pxy, -Pxz, -Pxy, Pxx+Pzz, -Pyz, -Pxz, -Pyz, Pxx+Pyy);
    }
    else
    {
        Scalar Pxx = Scalar(0);
        Scalar Pyy = Scalar(0);
        Scalar Pzz = Scalar(0);
        Scalar Pxy = Scalar(0);
        Scalar Pxz = Scalar(0);
        Scalar Pyz = Scalar(0);
        
        for(size_t i=0; i<mesh->faces.size(); ++i)
        {
            //Triangle verticies with respect to CG
            glm::vec3 v1gl = mesh->vertices[mesh->faces[i].vertexID[0]].pos;
            glm::vec3 v2gl = mesh->vertices[mesh->faces[i].vertexID[1]].pos;
            glm::vec3 v3gl = mesh->vertices[mesh->faces[i].vertexID[2]].pos;
            
            Vector3 v1(v1gl.x,v1gl.y,v1gl.z);
            Vector3 v2(v2gl.x,v2gl.y,v2gl.z);
            Vector3 v3(v3gl.x,v3gl.y,v3gl.z);
            v1 -= CG;
            v2 -= CG;
            v3 -= CG;
            
            //Pjk = const * dV * (2*Aj*Ak + 2*Bj*Bk + 2*Cj*Ck + Aj*Bk + Ak*Bj + Aj*Ck + Ak*Cj + Bj*Ck + Bk*Cj)
            Scalar V6 = v1.dot(v2.cross(v3));
            Pxx += V6 * 2 *(v1.x()*v1.x() + v2.x()*v2.x() + v3.x()*v3.x() + v1.x()*v2.x() + v1.x()*v3.x() + v2.x()*v3.x());
            Pyy += V6 * 2 *(v1.y()*v1.y() + v2.y()*v2.y() + v3.y()*v3.y() + v1.y()*v2.y() + v1.y()*v3.y() + v2.y()*v3.y());
            Pzz += V6 * 2 *(v1.z()*v1.z() + v2.z()*v2.z() + v3.z()*v3.z() + v1.z()*v2.z() + v1.z()*v3.z() + v2.z()*v3.z());
            Pxy += V6 * (2*(v1.x()*v1.y() + v2.x()*v2.y() + v3.x()*v3.y()) + v1.x()*v2.y() + v1.y()*v2.x() + v1.x()*v3.y() + v1.y()*v3.x() + v2.x()*v3.y() + v2.y()*v3.x());
            Pxz += V6 * (2*(v1.x()*v1.z() + v2.x()*v2.z() + v3.x()*v3.z()) + v1.x()*v2.z() + v1.z()*v2.x() + v1.x()*v3.z() + v1.z()*v3.x() + v2.x()*v3.z() + v2.z()*v3.x());
            Pyz += V6 * (2*(v1.y()*v1.z() + v2.y()*v2.z() + v3.y()*v3.z()) + v1.y()*v2.z() + v1.z()*v2.y() + v1.y()*v3.z() + v1.z()*v3.y() + v2.y()*v3.z() + v2.z()*v3.y());
        }
        
        Pxx *= mat.density / Scalar(120); //20 from formula and 6 from polyhedron volume
        Pyy *= mat.density / Scalar(120);
        Pzz *= mat.density / Scalar(120);
        Pxy *= mat.density / Scalar(120);
        Pxz *= mat.density / Scalar(120);
        Pyz *= mat.density / Scalar(120);
        
        I = Matrix3(Pyy+Pzz, -Pxy, -Pxz, -Pxy, Pxx+Pzz, -Pyz, -Pxz, -Pyz, Pxx+Pyy);
    }
    
    //3. Find primary moments of inertia
    Ipri = Vector3(I.getRow(0).getX(), I.getRow(1).getY(), I.getRow(2).getZ());
    Irot = I3();
    
    //Check if inertia matrix is not diagonal
    if(!(btFuzzyZero(I.getRow(0).getY()) && btFuzzyZero(I.getRow(0).getZ())
         && btFuzzyZero(I.getRow(1).getX()) && btFuzzyZero(I.getRow(1).getZ())
         && btFuzzyZero(I.getRow(2).getX()) && btFuzzyZero(I.getRow(2).getY())))
    {
        //3.1. Calculate principal moments of inertia
        Scalar T = I[0][0] + I[1][1] + I[2][2]; //Ixx + Iyy + Izz
        Scalar II = I[0][0]*I[1][1] + I[0][0]*I[2][2] + I[1][1]*I[2][2] - I[0][1]*I[0][1] - I[0][2]*I[0][2] - I[1][2]*I[1][2]; //Ixx Iyy + Ixx Izz + Iyy Izz - Ixy^2 - Ixz^2 - Iyz^2
        Scalar U = btSqrt(T*T-Scalar(3)*II)/Scalar(3);
        Scalar theta = btAcos((-Scalar(2)*T*T*T + Scalar(9)*T*II - Scalar(27)*I.determinant())/(Scalar(54)*U*U*U));
        Scalar A = T/Scalar(3) - Scalar(2)*U*btCos(theta/Scalar(3));
        Scalar B = T/Scalar(3) - Scalar(2)*U*btCos(theta/Scalar(3) - Scalar(2)*M_PI/Scalar(3));
        Scalar C = T/Scalar(3) - Scalar(2)*U*btCos(theta/Scalar(3) + Scalar(2)*M_PI/Scalar(3));
        Ipri = Vector3(A, B, C);
        
        //3.2. Calculate principal axes of inertia
        Matrix3 L;
        Vector3 axis1,axis2,axis3;
        axis1 = findInertiaAxis(I, A);
        axis2 = findInertiaAxis(I, B);
        axis3 = axis1.cross(axis2);
        axis2 = axis3.cross(axis1);
        
        //3.3. Rotate body so that principal axes are parallel to (x,y,z) system
        Irot = Matrix3(axis1[0],axis2[0],axis3[0], axis1[1],axis2[1],axis3[1], axis1[2],axis2[2],axis3[2]);
    }
}

}
