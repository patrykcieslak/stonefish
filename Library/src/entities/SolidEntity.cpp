//
//  SolidEntity.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 12/29/12.
//  Copyright (c) 2012-2018 Patryk Cieslak. All rights reserved.
//

#include "entities/SolidEntity.h"

#include "core/SimulationApp.h"
#include "graphics/Console.h"
#include "utils/MathUtil.hpp"
#include "utils/SystemUtil.hpp"
#include "entities/forcefields/Ocean.h"

using namespace sf;

SolidEntity::SolidEntity(std::string uniqueName, Material m, int _lookId, Scalar thickness, bool isBuoyant) : Entity(uniqueName)
{
	mat = m;
    mass = Scalar(0);
    aMass = Matrix6Eigen::Zero();
	Ipri.setZero();
    volume = Scalar(0);
	thick = thickness;
    CB.setZero();
    T_G2CG = Transform::getIdentity(); //CoG = (0,0,0)
    
    hydroProxyType = HYDRO_PROXY_NONE;
    hydroProxyParams = std::vector<Scalar>(0);
    hydroProxyTransform = Transform::getIdentity();
    
	computeHydro = true;
    buoyant = isBuoyant;
    Fb.setZero();
    Tb.setZero();
    Fds.setZero();
    Tds.setZero();
    Fdp.setZero();
    Tdp.setZero();
    
	filteredLinearVel.setZero();
	filteredAngularVel.setZero();
    linearAcc.setZero();
    angularAcc.setZero();
	
    rigidBody = NULL;
    multibodyCollider = NULL;
    
	mesh = NULL;
	lookId = _lookId;
	objectId = -1;
	
    dispCoordSys = false;
}

SolidEntity::~SolidEntity()
{
    if(mesh != NULL)
	{
		delete mesh;
		mesh = NULL;
	}
	
    rigidBody = NULL;
    multibodyCollider = NULL;
}

EntityType SolidEntity::getType()
{
    return ENTITY_SOLID;
}

void SolidEntity::ScalePhysicalPropertiesToArbitraryMass(Scalar mass)
{
    if(rigidBody != NULL)
    {
        Scalar oldMass = this->mass;
        this->mass = mass;
        Ipri *= this->mass/oldMass;
        rigidBody->setMassProps(this->mass, Ipri);
    }
    else if(multibodyCollider == NULL) 
    {
        Scalar oldMass = this->mass;
        this->mass = mass;
        Ipri *= this->mass/oldMass;        
    }
}

void SolidEntity::SetArbitraryPhysicalProperties(Scalar mass, const Vector3& inertia, const Transform& G2CG)
{
    if(rigidBody != NULL)
    {
        this->mass = mass;
        Ipri = inertia;
        rigidBody->setMassProps(this->mass, Ipri);
        
        Transform oldLocalTransform = T_G2CG;
        T_G2CG = G2CG;
        btCompoundShape* colShape = (btCompoundShape*)rigidBody->getCollisionShape();
        rigidBody->setCenterOfMassTransform(oldLocalTransform.inverse() * T_G2CG * rigidBody->getCenterOfMassTransform());
        colShape->updateChildTransform(0, T_G2CG.inverse());
    }
    else if(multibodyCollider == NULL) // && rigidBody == NULL
    {
        this->mass = mass;
        Ipri = inertia;
        T_G2CG = G2CG;
    }
}

void SolidEntity::SetHydrodynamicProperties(const Matrix6Eigen& addedMass, const Matrix6Eigen& damping, const Transform& G2CB)
{
}

void SolidEntity::setComputeHydrodynamics(bool flag)
{
	computeHydro = flag;
}

void SolidEntity::setLook(int newLookId)
{
    lookId = newLookId;
}

void SolidEntity::setDisplayCoordSys(bool enabled)
{
    dispCoordSys = enabled;
}

bool SolidEntity::isCoordSysVisible() const
{
    return dispCoordSys;
}

int SolidEntity::getLook() const
{
    return lookId;
}

int SolidEntity::getObject() const
{
	return objectId;
}

btRigidBody* SolidEntity::getRigidBody()
{
    return rigidBody;
}

btMultiBodyLinkCollider* SolidEntity::getMultibodyLinkCollider()
{
    return multibodyCollider;
}

bool SolidEntity::isBuoyant() const
{
    return buoyant;
}

void SolidEntity::getAABB(Vector3& min, Vector3& max)
{
    if(rigidBody != NULL)
        rigidBody->getAabb(min, max);
    else if(multibodyCollider != NULL)
        multibodyCollider->getCollisionShape()->getAabb(getCGTransform(), min, max);
}

std::vector<Renderable> SolidEntity::Render()
{
	std::vector<Renderable> items(0);
	
	if((rigidBody != NULL || multibodyCollider != NULL) && objectId >= 0 && isRenderable())
	{
        Transform oTrans = getGTransform();
		
		Renderable item;
		item.type = RenderableType::SOLID;
        item.objectId = objectId;
		item.lookId = lookId;
		item.model = glMatrixFromBtTransform(oTrans);
		items.push_back(item);
        
        item.type = RenderableType::SOLID_CS;
        item.model = glMatrixFromBtTransform(getCGTransform());
        items.push_back(item);
        
        switch(hydroProxyType)
        {
            case HYDRO_PROXY_NONE:
                break;
                
            case HYDRO_PROXY_SPHERE:
                item.type = RenderableType::HYDRO_ELLIPSOID;
                item.model = glMatrixFromBtTransform(oTrans * hydroProxyTransform);
                item.points.push_back(glm::vec3((GLfloat)hydroProxyParams[0], (GLfloat)hydroProxyParams[0], (GLfloat)hydroProxyParams[0]));
                items.push_back(item);
                break;
                
            case HYDRO_PROXY_CYLINDER:
                item.type = RenderableType::HYDRO_CYLINDER;
                item.model = glMatrixFromBtTransform(oTrans * hydroProxyTransform);
                item.points.push_back(glm::vec3((GLfloat)hydroProxyParams[0], (GLfloat)hydroProxyParams[0], (GLfloat)hydroProxyParams[1]));
                items.push_back(item);
                break;
                
            case HYDRO_PROXY_ELLIPSOID:
                item.type = RenderableType::HYDRO_ELLIPSOID;
                item.model = glMatrixFromBtTransform(oTrans * hydroProxyTransform);
                item.points.push_back(glm::vec3((GLfloat)hydroProxyParams[0], (GLfloat)hydroProxyParams[1], (GLfloat)hydroProxyParams[2]));
                items.push_back(item);
                break;
        }
        
        Vector3 cobWorld = oTrans * CB;
        item.type = RenderableType::HYDRO_CS;
        item.model = glMatrixFromBtTransform(Transform(Quaternion::getIdentity(), cobWorld));
        item.points.push_back(glm::vec3(volume, volume, volume));
        items.push_back(item);
	}
	
	return items;
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
        
        if(index >= 0) //If collider is not base
        {
            for(int i = 0; i <= index; i++) //Accumulate velocity resulting from joints
            {
                if(multiBody->getLink(i).m_jointType == btMultibodyLink::ePrismatic) //Just add linear velocity
                {
                    Vector3 axis = multiBody->getLink(i).getAxisBottom(0); //Local axis
                    Vector3 vel = multiBody->getJointVel(i) * axis; //Local velocity
                    Vector3 gvel = multiBody->localDirToWorld(i, vel); //Global velocity
                    linVelocity += gvel;
                }
                else if(multiBody->getLink(i).m_jointType == btMultibodyLink::eRevolute) //Add linear velocity due to rotation
                {
                    Vector3 axis = multiBody->getLink(i).getAxisBottom(0); //Local linear motion
                    Vector3 vel = multiBody->getJointVel(i) * axis; //Local velocity
                    Vector3 gvel = multiBody->localDirToWorld(i, vel); //Global velocity
                    linVelocity += gvel;
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
            for(int i = 0; i <= index; i++) //Accumulate velocity resulting from joints
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
        return Vector3(0.,0.,0.);
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

Transform SolidEntity::getGTransform() const
{
    return getCGTransform() * getG2CGTransform().inverse();
}

Transform SolidEntity::getG2CGTransform() const
{
    return T_G2CG;
}

Vector3 SolidEntity::getCBPositionInGFrame() const
{
    return CB;
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

Material SolidEntity::getMaterial() const
{
    return mat;
}

std::vector<Vertex>* SolidEntity::getMeshVertices()
{
    std::vector<Vertex>* pVert = new std::vector<Vertex>(0);
    
    if(mesh != NULL)
        pVert->insert(pVert->end(), mesh->vertices.begin(), mesh->vertices.end());
        
    return pVert;
}

void SolidEntity::ComputeHydrodynamicProxy(HydrodynamicProxyType t)
{
    switch(t)
    {
        case HYDRO_PROXY_NONE:
            break;
            
        case HYDRO_PROXY_SPHERE:
            ComputeProxySphere();
            break;
            
        case HYDRO_PROXY_CYLINDER:
            ComputeProxyCylinder();
            break;
            
        case HYDRO_PROXY_ELLIPSOID:
            ComputeProxyEllipsoid();
            break;
    }
}

void SolidEntity::ComputeProxySphere()
{
    //Get vertices of solid
    std::vector<Vertex>* vertices = getMeshVertices();
    if(vertices->size() < 2)
    {
        delete vertices;
        return;
    }
    
    //Fit spherical envelope aligned with COG
    Scalar radius = 0;
    
    for(unsigned int i=0; i<vertices->size(); ++i)
    {
        Vector3 vpos((*vertices)[i].pos.x, (*vertices)[i].pos.y, (*vertices)[i].pos.z);
        vpos = T_G2CG.inverse() * vpos;
        
        Scalar tmp = vpos.length2();
        
        if(radius < tmp)
            radius = tmp;
    }
    
    //Set parameters
    hydroProxyType = HYDRO_PROXY_SPHERE;
    hydroProxyParams.resize(1);
    hydroProxyParams[0] = btSqrt(radius);
    hydroProxyTransform.setIdentity();
    
    //Added mass and inertia
    Scalar rho = Scalar(1000);
    Scalar m = Scalar(2)*M_PI*rho*radius*radius*radius/Scalar(3);
    Scalar I = Scalar(0);
    
    aMass(0,0) = m;
    aMass(1,1) = m;
    aMass(2,2) = m;
    aMass(3,3) = I;
    aMass(4,4) = I;
    aMass(5,5) = I;
    
#ifdef DEBUG
    //std::cout << getName() << " added mass: " << aMass << std::endl << std::endl;
#endif
}

void SolidEntity::ComputeProxyCylinder()
{
    //Get vertices of solid
	std::vector<Vertex>* vertices = getMeshVertices();
    if(vertices->size() < 2)
    {
        delete vertices;
        return;
    }
    
    //Fit cylindrical envelope alinged with x,y,z axes
    Scalar cylinders[6] = {0,0,0,0,0,0};
    
    for(unsigned int i=0; i<vertices->size(); ++i)
    {
        Vector3 vpos((*vertices)[i].pos.x, (*vertices)[i].pos.y, (*vertices)[i].pos.z);
        vpos = T_G2CG.inverse() * vpos;
        
        Scalar tmp;
        
        //Along X
        if(cylinders[0] < (tmp = vpos.z()*vpos.z() + vpos.y()*vpos.y()))
            cylinders[0] = tmp;
        if(cylinders[1] < (tmp = btFabs(vpos.x())))
            cylinders[1] = tmp;
        //Along Y
        if(cylinders[2] < (tmp = vpos.x()*vpos.x() + vpos.z()*vpos.z()))
            cylinders[2] = tmp;
        if(cylinders[3] < (tmp = btFabs(vpos.y())))
            cylinders[3] = tmp;
        //Along Z
        if(cylinders[4] < (tmp = vpos.x()*vpos.x() + vpos.y()*vpos.y()))
            cylinders[4] = tmp;
        if(cylinders[5] < (tmp = btFabs(vpos.z())))
            cylinders[5] = tmp;
    }
    
    //Calculate volume
    Scalar volume[3];
    volume[0] = cylinders[0] * cylinders[1];
    volume[1] = cylinders[2] * cylinders[3];
    volume[2] = cylinders[4] * cylinders[5];
    
    //Choose smallest volume
    hydroProxyType = HYDRO_PROXY_CYLINDER;
    hydroProxyParams.resize(2);
    
    if(volume[0] <= volume[1] && volume[0] <= volume[2]) //X cylinder smallest
    {
        hydroProxyParams[0] = btSqrt(cylinders[0]);
        hydroProxyParams[1] = cylinders[1]*Scalar(2);
        hydroProxyTransform = Transform(Quaternion(-M_PI_2, 0, 0), Vector3(0,0,0));
    }
    else if(volume[1] <= volume[0] && volume[1] <= volume[2]) //Y cylinder smallest
    {
        hydroProxyParams[0] = btSqrt(cylinders[2]);
        hydroProxyParams[1] = cylinders[3]*Scalar(2);
        hydroProxyTransform = Transform(Quaternion(0, M_PI_2, 0), Vector3(0,0,0));
    }
    else //Z cylinder smallest
    {
        hydroProxyParams[0] = btSqrt(cylinders[4]);
        hydroProxyParams[1] = cylinders[5]*Scalar(2);
        
        
        hydroProxyTransform = Transform::getIdentity();
    }
    
    //Added mass and inertia
    Scalar rho = Scalar(1000);
    Scalar m1 = rho*M_PI*hydroProxyParams[0]*hydroProxyParams[0]; //Parallel to axis
    Scalar m2 = rho*M_PI*hydroProxyParams[0]*hydroProxyParams[0]*Scalar(2)*hydroProxyParams[1]; //Perpendicular to axis
    Scalar I1 = Scalar(0);
    Scalar I2 = Scalar(1)/Scalar(12)*M_PI*rho*hydroProxyParams[1]*hydroProxyParams[1]*btPow(hydroProxyParams[0], Scalar(3));
    
    Vector3 M = hydroProxyTransform.getBasis() * Vector3(m2, m2, m1);
    Vector3 I = hydroProxyTransform.getBasis() * Vector3(I2, I2, I1);
    
    aMass(0,0) = btFabs(M.x());
    aMass(1,1) = btFabs(M.y());
    aMass(2,2) = btFabs(M.z());
    aMass(3,3) = btFabs(I.x());
    aMass(4,4) = btFabs(I.y());
    aMass(5,5) = btFabs(I.z());
    
    hydroProxyTransform = T_G2CG * hydroProxyTransform;
    
#ifdef DEBUG
    //std::cout << getName() << " added mass: " << aMass << std::endl << std::endl;
#endif
}

void SolidEntity::ComputeProxyEllipsoid()
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
        vpos = T_G2CG.inverse() * vpos;
		P.row(i) << vpos.x(), vpos.y(), vpos.z();
    }
    delete vertices;
    
    //Ellipsoid fit
    //Compute contraints -> axis aligned, three radii
	MatrixXEigen A(P.rows(), 6);
	A.col(0) = P.col(0).array() * P.col(0).array() + P.col(1).array() * P.col(1).array() - 2 * P.col(2).array() * P.col(2).array();
	A.col(1) = P.col(0).array() * P.col(0).array() + P.col(2).array() * P.col(2).array() - 2 * P.col(1).array() * P.col(1).array();
	A.col(2) = 2 * P.col(0);
	A.col(3) = 2 * P.col(1);
	A.col(4) = 2 * P.col(2);
	A.col(5) = MatrixXEigen::Ones(P.rows(), 1);
	
	//Compute contraints -> arbitrary axes, three radii
	/*MatrixXEigen A(P.rows(), 9);
	A.col(0) = P.col(0).array() * P.col(0).array() + P.col(1).array() * P.col(1).array() - 2 * P.col(2).array() * P.col(2).array();
	A.col(1) = P.col(0).array() * P.col(0).array() + P.col(2).array() * P.col(2).array() - 2 * P.col(1).array() * P.col(1).array();
	A.col(2) = 2 * P.col(0).array() * P.col(1).array();
	A.col(3) = 2 * P.col(0).array() * P.col(2).array();
	A.col(4) = 2 * P.col(1).array() * P.col(2).array();
	A.col(5) = 2 * P.col(0);
	A.col(6) = 2 * P.col(1);
	A.col(7) = 2 * P.col(2);
	A.col(8) = eigMatrix::Ones(P.rows(), 1);*/
	
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
    
    /*
	p(0) = x(0) + x(1) - 1;
	p(1) = x(0) - 2 * x(1) - 1;
	p(2) = x(1) - 2 * x(0) - 1;
	p(3) = x(2);
	p(4) = x(3);
	p(5) = x(4);
	p(6) = x(5);
	p(7) = x(6);
	p(8) = x(7);
	p(9) = x(8);*/
	
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
    
    hydroProxyType = HYDRO_PROXY_ELLIPSOID;
    hydroProxyParams.resize(3);
    hydroProxyParams[0] = ellipsoidR.getX();
    hydroProxyParams[1] = ellipsoidR.getY();
    hydroProxyParams[2] = ellipsoidR.getZ();
    
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
    ellipsoidTransform.getBasis().setIdentity();
    ellipsoidTransform.setOrigin(Vector3(c(0), c(1), c(2)));
    ellipsoidTransform = T_G2CG * ellipsoidTransform;
    
    hydroProxyTransform = ellipsoidTransform;
    
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
	if(mesh == NULL || !SimulationApp::getApp()->hasGraphics())
		return;
		
	objectId = OpenGLContent::getInstance()->BuildObject(mesh);	
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
                colShape->updateChildTransform(i, T_G2CG.inverse() * colShape->getChildTransform(i), true);
            }
        }
        else //For other shapes, create compound shape which allow for the shift against gravity centre
        {
            colShape = new btCompoundShape();
            colShape0->setMargin(Scalar(0));
            colShape->addChildShape(T_G2CG.inverse(), colShape0);
        }
        colShape->setMargin(Scalar(0));
        
        //Construct Bullet rigid body
        Scalar M = mass + btMin(btMin(aMass(0,0), aMass(1,1)), aMass(2,2));
        Vector3 I = Ipri + Vector3(aMass(3,3), aMass(4,4), aMass(5,5));
        
        btRigidBody::btRigidBodyConstructionInfo rigidBodyCI(M, motionState, colShape, I);
        rigidBodyCI.m_friction = rigidBodyCI.m_rollingFriction = rigidBodyCI.m_restitution = Scalar(0.); //not used
        rigidBodyCI.m_linearDamping = rigidBodyCI.m_angularDamping = Scalar(0.); //not used
		rigidBodyCI.m_linearSleepingThreshold = rigidBodyCI.m_angularSleepingThreshold = Scalar(0.); //not used
        rigidBodyCI.m_additionalDamping = false;
        
        rigidBody = new btRigidBody(rigidBodyCI);
        rigidBody->setUserPointer(this);
        rigidBody->setFlags(rigidBody->getFlags() | BT_ENABLE_GYROSCOPIC_FORCE_IMPLICIT_BODY);
		//rigidBody->setCollisionFlags(rigidBody->getCollisionFlags() | btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);
        rigidBody->setActivationState(DISABLE_DEACTIVATION);
        //rigidBody->setCcdMotionThreshold(0.01);
        //rigidBody->setCcdSweptSphereRadius(0.9);
        
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
                colShape->updateChildTransform(i, T_G2CG.inverse() * colShape->getChildTransform(i), true);
            }
        }
        else //For other shapes, create compound shape which allow for the shift against gravity centre
        {
            colShape = new btCompoundShape();
            colShape0->setMargin(Scalar(0));
            colShape->addChildShape(T_G2CG.inverse(), colShape0);
            
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
		//multibodyCollider->setCollisionFlags(multibodyCollider->getCollisionFlags() | btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);
		//multibodyCollider->setContactStiffnessAndDamping(1000000.0,0.1);
		multibodyCollider->setActivationState(DISABLE_DEACTIVATION);
        
        if(child > 0)
            mb->getLink(child - 1).m_collider = multibodyCollider;
        else
            mb->setBaseCollider(multibodyCollider);
        
        world->addCollisionObject(multibodyCollider, MASK_DEFAULT, MASK_STATIC | MASK_DEFAULT);
        
        //Graphics
        BuildGraphicalObject();
        
        cInfo("Built multibody link %s (mass[kg]: %1.3lf; inertia[kgm2]: %1.3lf, %1.3lf, %1.3lf; volume[cm3]: %1.1lf)", getName().c_str(), mass, Ipri.x(), Ipri.y(), Ipri.z(), volume*1e6);
    }
}

void SolidEntity::AddToDynamicsWorld(btMultiBodyDynamicsWorld* world)
{
    AddToDynamicsWorld(world, Transform::getIdentity());
}

void SolidEntity::AddToDynamicsWorld(btMultiBodyDynamicsWorld* world, const Transform& worldTransform)
{
    if(rigidBody == NULL)
    {
        BuildRigidBody();
		BuildGraphicalObject();
        
        //rigidBody->setMotionState(new btDefaultMotionState(UnitSystem::SetTransform(worldTransform)));
        rigidBody->setCenterOfMassTransform(worldTransform * T_G2CG);
        world->addRigidBody(rigidBody, MASK_DEFAULT, MASK_STATIC | MASK_DEFAULT);
    }
}

void SolidEntity::RemoveFromDynamicsWorld(btMultiBodyDynamicsWorld* world)
{
    if(rigidBody != NULL)
    {
        world->removeRigidBody(rigidBody);
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

void SolidEntity::SetAcceleration(const Vector3& lin, const Vector3& ang)
{
	linearAcc = lin;
	angularAcc = ang;
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

BodyFluidPosition SolidEntity::CheckBodyFluidPosition(Ocean* liquid)
{
    Vector3 aabbMin, aabbMax;
    getAABB(aabbMin, aabbMax);
    Vector3 d = aabbMax-aabbMin;
    
    unsigned int submerged = 0;
    if(liquid->GetDepth(aabbMin) > Scalar(0)) ++submerged;
    if(liquid->GetDepth(aabbMax) > Scalar(0)) ++submerged;
    if(liquid->GetDepth(aabbMin + Vector3(d.x(), 0, 0)) > Scalar(0)) ++submerged;
    if(liquid->GetDepth(aabbMin + Vector3(0, d.y(), 0)) > Scalar(0)) ++submerged;
    if(liquid->GetDepth(aabbMin + Vector3(d.x(), d.y(), 0)) > Scalar(0)) ++submerged;
    if(liquid->GetDepth(aabbMin + Vector3(0, 0, d.z())) > Scalar(0)) ++submerged;
    if(liquid->GetDepth(aabbMin + Vector3(d.x(), 0, d.z())) > Scalar(0)) ++submerged;
    if(liquid->GetDepth(aabbMin + Vector3(0, d.y(), d.z())) > Scalar(0)) ++submerged;
    
    if(submerged == 0)
        return BodyFluidPosition::OUTSIDE_FLUID;
    else if(submerged == 8)
        return BodyFluidPosition::INSIDE_FLUID;
    else
        return BodyFluidPosition::CROSSING_FLUID_SURFACE;
}

void SolidEntity::ComputeFluidForcesSurface(HydrodynamicsSettings settings, Ocean* liquid, const Transform& T_CG, const Transform& T_G,
                                            const Vector3& v, const Vector3& omega, Vector3& _Fb, Vector3& _Tb, Vector3& _Fds, Vector3& _Tds, Vector3& _Fdp, Vector3& _Tdp)
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
        _Fds.setZero();
        _Tds.setZero();
        _Fdp.setZero();
        _Tdp.setZero();
    }
	
    //Set zeros
	if(mesh == NULL)
		return;
    
#ifdef DEBUG    
    uint64_t start = GetTimeInMicroseconds();
#endif
      
    //Calculate fluid dynamics forces and torques
    Vector3 p = T_CG.getOrigin();
    Scalar viscousity = liquid->getLiquid()->viscosity;
 
	//Loop through all faces...
    for(unsigned int i=0; i<mesh->faces.size(); ++i)
    {
        //Global coordinates
		glm::vec3 p1gl = mesh->vertices[mesh->faces[i].vertexID[0]].pos;
		glm::vec3 p2gl = mesh->vertices[mesh->faces[i].vertexID[1]].pos;
		glm::vec3 p3gl = mesh->vertices[mesh->faces[i].vertexID[2]].pos;
        Vector3 p1 = T_G * Vector3(p1gl.x,p1gl.y,p1gl.z);
        Vector3 p2 = T_G * Vector3(p2gl.x,p2gl.y,p2gl.z);
        Vector3 p3 = T_G * Vector3(p3gl.x,p3gl.y,p3gl.z);
        
        //Check if face underwater
        Scalar depth[3];
        depth[0] = liquid->GetDepth(p1);
        depth[1] = liquid->GetDepth(p2);
        depth[2] = liquid->GetDepth(p3);
        
        if(depth[0] < Scalar(0.) && depth[1] < Scalar(0.) && depth[2] < Scalar(0.))
            continue;
        
        //Calculate face properties
        Vector3 fc;
        Vector3 fn;
        Vector3 fn1;
        Scalar A;
        
        if(depth[0] < Scalar(0.))
        {
            if(depth[1] < Scalar(0.))
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
                
                if(btFuzzyZero(len))
                    continue;
                
                fn1 = fn/len; //Normalised normal (length = 1)
                A = len/Scalar(2); //Area of the face (triangle)
            }
            else if(depth[2] < Scalar(0.))
            {
                p1 = p2 + (p1-p2) * (depth[1]/(btFabs(depth[0]) + depth[1]));
                //p2 withour change
                p3 = p2 + (p3-p2) * (depth[1]/(btFabs(depth[2]) + depth[1]));
                
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
            }
            else //depth[1] >= 0 && depth[2] >= 0
            {
                //Quad!!!!
                Vector3 p1temp = p2 + (p1-p2) * (depth[1]/(btFabs(depth[0]) + depth[1]));
                //p2 without change
                //p3 without change
                Vector3 p4 = p3 + (p1-p3) * (depth[2]/(btFabs(depth[0]) + depth[2]));
                p1 = p1temp;
                
                //Calculate
                Vector3 fv1 = p2-p1;
                Vector3 fv2 = p3-p1;
                Vector3 fv3 = p4-p1;
                
                fc = (p1 + p2 + p3 + p4)/Scalar(4);
                fn = fv1.cross(fv2);
                Scalar len = fn.length();
                
                if(btFuzzyZero(len))
                    continue;
                
                fn1 = fn/len;
                A = len + fv2.cross(fv3).length();
                fn = fn1 * A;
            }
        }
        else if(depth[1] < Scalar(0.))
        {
            if(depth[2] < Scalar(0.))
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
            }
            else
            {
                //Quad!!!!
                Vector3 p2temp = p1 + (p2-p1) * (depth[0]/(btFabs(depth[1]) + depth[0]));
                //p2 without change
                //p3 without change
                Vector3 p4 = p3 + (p2-p3) * (depth[2]/(btFabs(depth[1]) + depth[2]));
                p2 = p2temp;
                
                //Calculate
                Vector3 fv1 = p2-p1;
                Vector3 fv2 = p3-p1;
                Vector3 fv3 = p4-p1;
                
                fc = (p1 + p2 + p3 + p4)/Scalar(4);
                fn = fv1.cross(fv2);
                Scalar len = fn.length();
                
                if(btFuzzyZero(len))
                    continue;
                
                fn1 = fn/len;
                A = len + fv2.cross(fv3).length();
                fn = fn1 * A;
            }
        }
        else if(depth[2] < Scalar(0.))
        {
            
            //Quad!!!!
            Vector3 p3temp = p2 + (p3-p2) * (depth[1]/(btFabs(depth[2]) + depth[1]));
            //p2 without change
            //p3 without change
            Vector3 p4 = p1 + (p3-p1) * (depth[0]/(btFabs(depth[2]) + depth[0]));
            p3 = p3temp;
                
            //Calculate
            Vector3 fv1 = p2-p1;
            Vector3 fv2 = p3-p1;
            Vector3 fv3 = p4-p1;
                
            fc = (p1 + p2 + p3 + p4)/Scalar(4);
            fn = fv1.cross(fv2);
            Scalar len = fn.length();
            
            if(btFuzzyZero(len))
                continue;
            
            fn1 = fn/len;
            A = len + fv2.cross(fv3).length();
            fn = fn1 * A;
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
        }
        
        Scalar pressure = liquid->GetPressure(fc);
        
        //Buoyancy force
        if(settings.reallisticBuoyancy && buoyant)
        {
            Vector3 Fbi = -fn1 * A * pressure; //Buoyancy force per face (based on pressure)
            
            //Accumulate
            _Fb += Fbi;
            _Tb += (fc - p).cross(Fbi);
        }
        
        //Damping force
        if(settings.dampingForces)
		{
            //Skin drag force
            Vector3 vc = liquid->GetFluidVelocity(fc) - (v + omega.cross(fc - p)); //Water velocity at face center
            Vector3 vt = vc - (vc.dot(fn)*fn)/fn.length2(); //Water velocity projected on face (tangent to face)
            Vector3 Fds = viscousity * vt * A / Scalar(0.0001);
            //Vector3 Fds = vt.safeNormalize()*Scalar(0.5)*fluid->getFluid()->density*Scalar(1.328)/1000.0*vt.length2()*fn.length()/Scalar(2);
        
            //Pressure drag force
            Vector3 vn = vc - vt; //Water velocity normal to face
            Vector3 Fdp(0,0,0);
            
            if(fn.dot(vn) < Scalar(0))
                Fdp = Scalar(0.5) * liquid->getLiquid()->density * vn * vn.safeNorm() * A;
            
            //Accumulate
            _Fds += Fds;
            _Tds += (fc - p).cross(Fds); 
            _Fdp += Fdp; 
            _Tdp += (fc - p).cross(Fdp);
        }
    }

#ifdef DEBUG
    uint64_t elapsed = GetTimeInMicroseconds() - start;
    //std::cout << getName() << ": " <<  elapsed << std::endl; 
#endif
}

void SolidEntity::ComputeFluidForcesSubmerged(HydrodynamicsSettings settings, Ocean* liquid, const Transform& T_CG, const Transform& T_G,
                                              const Vector3& v, const Vector3& omega, Vector3& _Fb, Vector3& _Tb, Vector3& _Fds, Vector3& _Tds, Vector3& _Fdp, Vector3& _Tdp)
{
    //Compute buoyancy based on CB position
    _Fb = -volume*liquid->getLiquid()->density * SimulationApp::getApp()->getSimulationManager()->getGravity();
    _Tb = (T_G * CB - T_CG.getOrigin()).cross(Fb);
    
    //Damping forces
    if(settings.dampingForces)
    {
        _Fds.setZero();
        _Tds.setZero();
        _Fdp.setZero();
        _Tdp.setZero();
    }
    else
        return;
    
    //Set zeros
    if(mesh == NULL)
        return;
    
#ifdef DEBUG
    uint64_t start = GetTimeInMicroseconds();
#endif
    
    //Calculate fluid dynamics forces and torques
    Vector3 p = T_CG.getOrigin();
    Scalar viscousity = liquid->getLiquid()->viscosity;
    
    //Loop through all faces...
    for(unsigned int i=0; i<mesh->faces.size(); ++i)
    {
        //Global coordinates
        glm::vec3 p1gl = mesh->vertices[mesh->faces[i].vertexID[0]].pos;
        glm::vec3 p2gl = mesh->vertices[mesh->faces[i].vertexID[1]].pos;
        glm::vec3 p3gl = mesh->vertices[mesh->faces[i].vertexID[2]].pos;
        Vector3 p1 = T_G * Vector3(p1gl.x,p1gl.y,p1gl.z);
        Vector3 p2 = T_G * Vector3(p2gl.x,p2gl.y,p2gl.z);
        Vector3 p3 = T_G * Vector3(p3gl.x,p3gl.y,p3gl.z);
        
        //Calculate face properties
        Vector3 fv1 = p2-p1; //One side of the face (triangle)
        Vector3 fv2 = p3-p1; //Another side of the face (triangle)
        Vector3 fn = fv1.cross(fv2); //Normal of the face (length != 1)
        Scalar len = fn.safeNorm();
        
        if(len == Scalar(0)) continue; //If triangle is incorrect (two sides parallel)
        
        Vector3 fc = (p1+p2+p3)/Scalar(3); //Face centroid
        Vector3 fn1 = fn/len; //Normalised normal (length = 1)
        Scalar A = len/Scalar(2); //Area of the face (triangle)
        
        //Damping force
        if(settings.dampingForces)
        {
            //Skin drag force
            Vector3 vc = liquid->GetFluidVelocity(fc) - (v + omega.cross(fc - p)); //Water velocity at face center
            Vector3 vt = vc - (vc.dot(fn)*fn)/fn.length2(); //Water velocity projected on face (tangent to face)
            Vector3 Fds = viscousity * vt * A / Scalar(0.0001);
            //Vector3 Fds = vt.safeNormalize()*Scalar(0.5)*fluid->getFluid()->density*Scalar(1.328)/1000.0*vt.length2()*fn.length()/Scalar(2);
            
            //Pressure drag force
            Vector3 vn = vc - vt; //Water velocity normal to face
            Vector3 Fdp(0,0,0);
            
            if(fn.dot(vn) < Scalar(0))
                Fdp = Scalar(0.5) * liquid->getLiquid()->density * vn * vn.safeNorm() * A;
            
            //Accumulate
            _Fds += Fds;
            _Tds += (fc - p).cross(Fds);
            _Fdp += Fdp;
            _Tdp += (fc - p).cross(Fdp);
        }
    }
    
#ifdef DEBUG
    uint64_t elapsed = GetTimeInMicroseconds() - start;
    //std::cout << getName() << ": " <<  elapsed << std::endl;
#endif
}

void SolidEntity::ComputeFluidForces(HydrodynamicsSettings settings, Ocean* liquid)
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
	
    //Check if fully submerged --> simplifies buoyancy calculation
    if(bf == BodyFluidPosition::INSIDE_FLUID)
        ComputeFluidForcesSubmerged(settings, liquid, getCGTransform(), T, v, omega, Fb, Tb, Fds, Tds, Fdp, Tdp);
    else //CROSSING_FLUID_SURFACE
        ComputeFluidForcesSurface(settings, liquid, getCGTransform(), T, v, omega, Fb, Tb, Fds, Tds, Fdp, Tdp);
    
    //Correct damping based on approximate shape
    if(settings.dampingForces)
    {
        Transform hpTrans = getCGTransform() * T_G2CG.inverse() * hydroProxyTransform;
        
        switch(hydroProxyType)
        {
            case HYDRO_PROXY_NONE:
                //No information to correct
                break;
                
            case HYDRO_PROXY_SPHERE:
                //No need to correct
                break;
                
            case HYDRO_PROXY_CYLINDER:
            {
                //Correct drag based on cylindrical approximation of shape
                Vector3 cFd = hpTrans.getBasis().inverse() * Fdp;
                Vector3 Cd(0.5, 0.5, 1.0);
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

void SolidEntity::ApplyFluidForces()
{
    ApplyCentralForce(Fb + Fds + Fdp);
    ApplyTorque(Tb + Tds + Tdp);
}
