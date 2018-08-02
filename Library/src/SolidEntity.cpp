//
//  SolidEntity.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 12/29/12.
//  Copyright (c) 2012 Patryk Cieslak. All rights reserved.
//

#include "SolidEntity.h"
#include "MathsUtil.hpp"
#include "SystemUtil.hpp"
#include "Console.h"
#include "Ocean.h"

SolidEntity::SolidEntity(std::string uniqueName, Material m, int _lookId, btScalar thickness, bool isBuoyant) : Entity(uniqueName)
{
	mat = m;
    mass = btScalar(0);
    aMass = eigMatrix6x6::Zero();
	Ipri.setZero();
    volume = btScalar(0);
	thick = UnitSystem::SetLength(thickness);
    CoB.setZero();
    localTransform = btTransform::getIdentity(); //CoG = (0,0,0)
    ellipsoidR.setZero();
    ellipsoidTransform = btTransform::getIdentity();
	computeHydro = true;
    buoyant = isBuoyant;
    Fb.setZero();
    Tb.setZero();
    Fds.setZero();
    Tds.setZero();
    Fdp.setZero();
    Tdp.setZero();
    Fa.setZero();
    Ta.setZero();
    
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

void SolidEntity::ScalePhysicalPropertiesToArbitraryMass(btScalar mass)
{
    if(rigidBody != NULL)
    {
        btScalar oldMass = this->mass;
        this->mass = UnitSystem::SetMass(mass);
        Ipri *= this->mass/oldMass;
        rigidBody->setMassProps(this->mass, Ipri);
    }
    else if(multibodyCollider == NULL) 
    {
        btScalar oldMass = this->mass;
        this->mass = UnitSystem::SetMass(mass);
        Ipri *= this->mass/oldMass;        
    }
}

void SolidEntity::SetArbitraryPhysicalProperties(btScalar mass, const btVector3& inertia, const btTransform& cogTransform)
{
    if(rigidBody != NULL)
    {
        this->mass = UnitSystem::SetMass(mass);
        Ipri = UnitSystem::SetInertia(inertia);
        rigidBody->setMassProps(this->mass, Ipri);
        
        btTransform oldLocalTransform = localTransform;
        localTransform = UnitSystem::SetTransform(cogTransform);
        btCompoundShape* colShape = (btCompoundShape*)rigidBody->getCollisionShape();
        rigidBody->setCenterOfMassTransform(oldLocalTransform.inverse() * localTransform * rigidBody->getCenterOfMassTransform());
        colShape->updateChildTransform(0, localTransform.inverse());
    }
    else if(multibodyCollider == NULL) // && rigidBody == NULL
    {
        this->mass = UnitSystem::SetMass(mass);
        Ipri = UnitSystem::SetInertia(inertia);
        localTransform = UnitSystem::SetTransform(cogTransform);
    }
}

void SolidEntity::SetHydrodynamicProperties(const eigMatrix6x6& addedMass, const eigMatrix6x6& damping, const btTransform& cobTransform)
{
}

void SolidEntity::setComputeHydrodynamics(bool flag)
{
	computeHydro = flag;
}

void SolidEntity::SetLook(int newLookId)
{
    lookId = newLookId;
}

void SolidEntity::setDisplayCoordSys(bool enabled)
{
    dispCoordSys = enabled;
}

bool SolidEntity::isCoordSysVisible()
{
    return dispCoordSys;
}

int SolidEntity::getLook()
{
    return lookId;
}

int SolidEntity::getObject()
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

bool SolidEntity::isBuoyant()
{
    return buoyant;
}

void SolidEntity::GetAABB(btVector3& min, btVector3& max)
{
    if(rigidBody != NULL)
        rigidBody->getAabb(min, max);
    else if(multibodyCollider != NULL)
        multibodyCollider->getCollisionShape()->getAabb(getTransform(), min, max);
}

std::vector<Renderable> SolidEntity::Render()
{
	std::vector<Renderable> items(0);
	
	if((rigidBody != NULL || multibodyCollider != NULL) && objectId >= 0 && isRenderable())
	{
		btTransform oTrans =  getTransform() * localTransform.inverse();
		
		Renderable item;
		item.type = RenderableType::SOLID;
        item.objectId = objectId;
		item.lookId = lookId;
		item.model = glMatrixFromBtTransform(oTrans);
		items.push_back(item);
        
        item.type = RenderableType::SOLID_CS;
        item.model = glMatrixFromBtTransform(getTransform());
        items.push_back(item);
        
        item.type = RenderableType::HYDRO;
        item.model = glMatrixFromBtTransform(oTrans * ellipsoidTransform);
        item.points.push_back(glm::vec3((GLfloat)ellipsoidR[0], (GLfloat)ellipsoidR[1], (GLfloat)ellipsoidR[2]));
        items.push_back(item);
        
        btVector3 cobWorld = oTrans * CoB;
        item.type = RenderableType::HYDRO_CS;
        item.model = glMatrixFromBtTransform(btTransform(btQuaternion::getIdentity(), cobWorld));
        item.points.push_back(glm::vec3(volume, volume, volume));
        items.push_back(item);
	}
	
	return items;
}

btTransform SolidEntity::getTransform() const
{
    if(rigidBody != NULL)
    {
        btTransform trans;
        rigidBody->getMotionState()->getWorldTransform(trans);
        return trans;
    }
    else if(multibodyCollider != NULL)
    {
        return multibodyCollider->getWorldTransform();
    }
    else
        return btTransform::getIdentity();
}

void SolidEntity::setTransform(const btTransform& trans)
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

btVector3 SolidEntity::getLinearVelocity()
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
        btVector3 linVelocity = multiBody->getBaseVel(); //Global
        
        if(index >= 0) //If collider is not base
        {
            for(int i = 0; i <= index; i++) //Accumulate velocity resulting from joints
            {
                if(multiBody->getLink(i).m_jointType == btMultibodyLink::ePrismatic) //Just add linear velocity
                {
                    btVector3 axis = multiBody->getLink(i).getAxisBottom(0); //Local axis
                    btVector3 vel = multiBody->getJointVel(i) * axis; //Local velocity
                    btVector3 gvel = multiBody->localDirToWorld(i, vel); //Global velocity
                    linVelocity += gvel;
                }
                else if(multiBody->getLink(i).m_jointType == btMultibodyLink::eRevolute) //Add linear velocity due to rotation
                {
                    btVector3 axis = multiBody->getLink(i).getAxisBottom(0); //Local linear motion
                    btVector3 vel = multiBody->getJointVel(i) * axis; //Local velocity
                    btVector3 gvel = multiBody->localDirToWorld(i, vel); //Global velocity
                    linVelocity += gvel;
                }
            }
        }
		return linVelocity;
    }
    else
        return btVector3(0,0,0);
}

btVector3 SolidEntity::getAngularVelocity()
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
        btVector3 angVelocity = multiBody->getBaseOmega(); //Global
        
        if(index >= 0)
        {
            for(int i = 0; i <= index; i++) //Accumulate velocity resulting from joints
                if(multiBody->getLink(i).m_jointType == btMultibodyLink::eRevolute) //Only revolute joints can change angular velocity
                {
                    btVector3 axis = multiBody->getLink(i).getAxisTop(0); //Local axis
                    btVector3 vel = multiBody->getJointVel(i) * axis; //Local velocity
                    btVector3 gvel = multiBody->localDirToWorld(i, vel); //Global velocity
                    angVelocity += gvel;
                }
        }
        
        return angVelocity;
    }
    else
        return btVector3(0,0,0);
}

btVector3 SolidEntity::getLinearVelocityInLocalPoint(const btVector3& relPos)
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
        return btVector3(0.,0.,0.);
}

btVector3 SolidEntity::getLinearAcceleration()
{
    return linearAcc;
}

btVector3 SolidEntity::getAngularAcceleration()
{
    return angularAcc;
}

btScalar SolidEntity::getVolume()
{
    return volume;
}

btTransform SolidEntity::getGeomToCOGTransform()
{
    return localTransform;
}

btVector3 SolidEntity::getCOBInGeomFrame()
{
    return CoB;
}

btVector3 SolidEntity::getInertia()
{
    return Ipri;
}

btScalar SolidEntity::getMass()
{
    return mass;
}

eigMatrix6x6 SolidEntity::getAddedMass()
{
    return aMass;
}

Material SolidEntity::getMaterial()
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

void SolidEntity::ComputeEquivEllipsoid()
{
    //Get vertices of solid
	std::vector<Vertex>* vertices = getMeshVertices();
    if(vertices->size() < 9)
    {
        delete vertices;
        return;
    }
    
	//Fill points matrix
	eigMatrix P(vertices->size(), 3);
	for(unsigned int i=0; i<vertices->size(); ++i)
    {
        btVector3 vpos((*vertices)[i].pos.x, (*vertices)[i].pos.y, (*vertices)[i].pos.z);
        vpos = localTransform.inverse() * vpos;
		P.row(i) << vpos.x(), vpos.y(), vpos.z();
    }
    delete vertices;
    
    //Ellipsoid fit
    //Compute contraints -> axis aligned, three radii
	eigMatrix A(P.rows(), 6);
	A.col(0) = P.col(0).array() * P.col(0).array() + P.col(1).array() * P.col(1).array() - 2 * P.col(2).array() * P.col(2).array();
	A.col(1) = P.col(0).array() * P.col(0).array() + P.col(2).array() * P.col(2).array() - 2 * P.col(1).array() * P.col(1).array();
	A.col(2) = 2 * P.col(0);
	A.col(3) = 2 * P.col(1);
	A.col(4) = 2 * P.col(2);
	A.col(5) = eigMatrix::Ones(P.rows(), 1);
	
	//Compute contraints -> arbitrary axes, three radii
	/*eigMatrix A(P.rows(), 9);
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
	eigMatrix b(P.rows(), 1);
	eigMatrix x(A.cols(), 1);	
	//squared norm
	b = P.col(0).array() * P.col(0).array() + P.col(1).array() * P.col(1).array() + P.col(2).array() * P.col(2).array();
	//solution
	//x = (A.transpose() * A).ldlt().solve(A.transpose() * b); //normal equations
	x = A.jacobiSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(b); 
    
	//Find ellipsoid parameters
	eigMatrix p(10, 1);
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
	
	eigMatrix E(4, 4);
	E << p(0), p(3), p(4), p(6),
		 p(3), p(1), p(5), p(7),
		 p(4), p(5), p(2), p(8),
		 p(6), p(7), p(8), p(9);
		 
	//Compute center
	eigMatrix c(3, 1);
	c = -E.block(0, 0, 3, 3).jacobiSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(p.block(6, 0, 3, 1));
	
	//Compute transform matrix
	eigMatrix4x4 T;
	T.setIdentity();
	T.block(3, 0, 1, 3) = c.transpose();
	T = T * E * T.transpose();
	
	//Compute axes
	Eigen::SelfAdjointEigenSolver<eigMatrix> eigenSolver(T.block(0, 0, 3, 3)/(-T(3,3)));
	if(eigenSolver.info() != Eigen::Success) 
	{
		cError("Error computing ellipsoid for %s!", getName().c_str());
		return;
	}
	
    //Ellipsoid radii
	eigMatrix r(3, 1);
	r = Eigen::sqrt(1.0/Eigen::abs(eigenSolver.eigenvalues().array()));
    ellipsoidR = btVector3(r(0), r(1), r(2));
    
    //Ellipsoid axes
	eigMatrix axes(3, 3);
	axes = eigenSolver.eigenvectors().array();
    
    //Reorder radii
    ellipsoidTransform.setIdentity();
    ellipsoidTransform.setBasis(btMatrix3x3(axes(0,0), axes(0,1), axes(0,2), axes(1,0), axes(1,1), axes(1,2), axes(2,0), axes(2,1), axes(2,2)));
    ellipsoidR = ellipsoidTransform.getBasis() * ellipsoidR;
    
    //Compute added mass
    //Search for the longest semiaxis
    btScalar rho = 1000.0; //Fluid density
    btScalar r12 = (r(1) + r(2))/btScalar(2);
    btScalar m1 = LambKFactor(r(0), r12)*btScalar(4)/btScalar(3)*M_PI*rho*r(0)*r12*r12;
    btScalar m2 = btScalar(4)/btScalar(3)*M_PI*rho*r(2)*r(2)*r(0);
    btScalar m3 = btScalar(4)/btScalar(3)*M_PI*rho*r(1)*r(1)*r(0);
    btScalar I1 = btScalar(0); //THIS SHOULD BE > 0
    btScalar I2 = btScalar(1)/btScalar(12)*M_PI*rho*r(1)*r(1)*btPow(r(0), btScalar(3));
    btScalar I3 = btScalar(1)/btScalar(12)*M_PI*rho*r(2)*r(2)*btPow(r(0), btScalar(3));
    
    btVector3 M = ellipsoidTransform.getBasis() * btVector3(m1,m2,m3);
    btVector3 I = ellipsoidTransform.getBasis() * btVector3(I1,I2,I3);
    
    aMass(0,0) = M.x();
    aMass(1,1) = M.y();
    aMass(2,2) = M.z();
    aMass(3,3) = I.x();
    aMass(4,4) = I.y();
    aMass(5,5) = I.z();
    
    //Set transform with respect to geometry
    ellipsoidTransform.getBasis().setIdentity();
    ellipsoidTransform.setOrigin(btVector3(c(0), c(1), c(2)));
    ellipsoidTransform = localTransform * ellipsoidTransform;
#ifdef DEBUG
    std::cout << getName() << " added mass: " << aMass << std::endl << std::endl;
#endif
}

btScalar SolidEntity::LambKFactor(btScalar r1, btScalar r2)
{
    btScalar e = btScalar(1) - r2*r2/r1;
    btScalar alpha0 = btScalar(2)*(btScalar(1)-e*e)/(e*e) * (btScalar(0.5)*btLog((btScalar(1)+e)/(btScalar(1)-e)) - e);
    return alpha0/(btScalar(2)-alpha0);
}

void SolidEntity::BuildGraphicalObject()
{
	if(mesh == NULL)
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
                colShape->getChildShape(i)->setMargin(btScalar(0));
                colShape->updateChildTransform(i, localTransform.inverse() * colShape->getChildTransform(i), true);
            }
        }
        else //For other shapes, create compound shape which allow for the shift against gravity centre
        {
            colShape = new btCompoundShape();
            colShape0->setMargin(btScalar(0));
            colShape->addChildShape(localTransform.inverse(), colShape0);            
        }
        colShape->setMargin(btScalar(0));
        
        //Construct Bullet rigid body
        btScalar M = mass + btMin(btMin(aMass(0,0), aMass(1,1)), aMass(2,2));
        btVector3 I = Ipri + btVector3(aMass(3,3), aMass(4,4), aMass(5,5));
        
        btRigidBody::btRigidBodyConstructionInfo rigidBodyCI(M, motionState, colShape, I);
        rigidBodyCI.m_friction = rigidBodyCI.m_rollingFriction = rigidBodyCI.m_restitution = btScalar(0.); //not used
        rigidBodyCI.m_linearDamping = rigidBodyCI.m_angularDamping = btScalar(0.); //not used
		rigidBodyCI.m_linearSleepingThreshold = rigidBodyCI.m_angularSleepingThreshold = btScalar(0.); //not used
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
                colShape->getChildShape(i)->setMargin(btScalar(0));
                colShape->updateChildTransform(i, localTransform.inverse() * colShape->getChildTransform(i), true);
            }
        }
        else //For other shapes, create compound shape which allow for the shift against gravity centre
        {
            colShape = new btCompoundShape();
            colShape0->setMargin(btScalar(0));
            colShape->addChildShape(localTransform.inverse(), colShape0);            
            
        }
        colShape->setMargin(btScalar(0));
        
        //Construct Bullet multi-body link
        multibodyCollider = new btMultiBodyLinkCollider(mb, child - 1);
        multibodyCollider->setCollisionShape(colShape);
        multibodyCollider->setUserPointer(this); //HAS TO BE AFTER SETTING COLLISION SHAPE TO PROPAGATE TO ALL OF COMPOUND SUBSHAPES!!!!!
        multibodyCollider->setFriction(btScalar(0));
        multibodyCollider->setRestitution(btScalar(0));
        multibodyCollider->setRollingFriction(btScalar(0));
        multibodyCollider->setSpinningFriction(btScalar(0));
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
    AddToDynamicsWorld(world, btTransform::getIdentity());
}

void SolidEntity::AddToDynamicsWorld(btMultiBodyDynamicsWorld* world, const btTransform& worldTransform)
{
    if(rigidBody == NULL)
    {
        BuildRigidBody();
		BuildGraphicalObject();
        
        //rigidBody->setMotionState(new btDefaultMotionState(UnitSystem::SetTransform(worldTransform)));
        rigidBody->setCenterOfMassTransform(UnitSystem::SetTransform(worldTransform) * localTransform);
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

void SolidEntity::UpdateAcceleration(btScalar dt)
{
	//Filter velocity
	btScalar alpha = 0.5;
	btVector3 currentLinearVel = alpha * getLinearVelocity() + (btScalar(1)-alpha) * filteredLinearVel;
	btVector3 currentAngularVel = alpha * getAngularVelocity() + (btScalar(1)-alpha) * filteredAngularVel;
		
	//Compute derivative
	linearAcc = (currentLinearVel - filteredLinearVel)/dt;
	angularAcc = (currentAngularVel - filteredAngularVel)/dt;
		
	//Update filtered
	filteredLinearVel = currentLinearVel;
	filteredAngularVel = currentAngularVel;
}

void SolidEntity::SetAcceleration(const btVector3& lin, const btVector3& ang)
{
	linearAcc = lin;
	angularAcc = ang;
}

void SolidEntity::ApplyGravity(const btVector3& g)
{
    if(rigidBody != NULL)
    {
        rigidBody->applyCentralForce(g * mass);
    }
}

void SolidEntity::ApplyCentralForce(const btVector3& force)
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

void SolidEntity::ApplyTorque(const btVector3& torque)
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

void SolidEntity::ComputeFluidForces(HydrodynamicsSettings settings, const Ocean* liquid, const btTransform& cogTransform, const btTransform& geometryTransform, const btVector3& v, const btVector3& omega, const btVector3& a, const btVector3& epsilon, btVector3& _Fb, btVector3& _Tb, btVector3& _Fds, btVector3& _Tds, btVector3& _Fdp, btVector3& _Tdp)
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
    btVector3 p = cogTransform.getOrigin();
    btScalar viscousity = liquid->getLiquid()->viscosity;
 
	//Loop through all faces...
    for(unsigned int i=0; i<mesh->faces.size(); ++i)
    {
        //Global coordinates
		glm::vec3 p1gl = mesh->vertices[mesh->faces[i].vertexID[0]].pos;
		glm::vec3 p2gl = mesh->vertices[mesh->faces[i].vertexID[1]].pos;
		glm::vec3 p3gl = mesh->vertices[mesh->faces[i].vertexID[2]].pos;
        btVector3 p1 = geometryTransform * btVector3(p1gl.x,p1gl.y,p1gl.z);
        btVector3 p2 = geometryTransform * btVector3(p2gl.x,p2gl.y,p2gl.z);
        btVector3 p3 = geometryTransform * btVector3(p3gl.x,p3gl.y,p3gl.z);
        
        //Check if underwater
        btScalar depth[3];
        depth[0] = liquid->GetDepth(p1);
        depth[1] = liquid->GetDepth(p2);
        depth[2] = liquid->GetDepth(p3);
        
        if(depth[0] < btScalar(0.) && depth[1] < btScalar(0.) && depth[2] < btScalar(0.))
            continue;
        
        //Calculate face properties
        btVector3 fc;
        btVector3 fn;
        btVector3 fn1;
        btScalar A;
        
        if(depth[0] < btScalar(0.))
        {
            if(depth[1] < btScalar(0.))
            {
                p1 = p3 + (p1-p3) * (depth[2]/(btFabs(depth[0]) + depth[2]));
                p2 = p3 + (p2-p3) * (depth[2]/(btFabs(depth[1]) + depth[2]));
                //p3 without change
                
                //Calculate
                btVector3 fv1 = p2-p1; //One side of the face (triangle)
                btVector3 fv2 = p3-p1; //Another side of the face (triangle)
                fc = (p1+p2+p3)/btScalar(3); //Face centroid
        
                fn = fv1.cross(fv2); //Normal of the face (length != 1)
                btScalar len = fn.length();
                
                if(btFuzzyZero(len))
                    continue;
                
                fn1 = fn/len; //Normalised normal (length = 1)
                A = len/btScalar(2); //Area of the face (triangle)
            }
            else if(depth[2] < btScalar(0.))
            {
                p1 = p2 + (p1-p2) * (depth[1]/(btFabs(depth[0]) + depth[1]));
                //p2 withour change
                p3 = p2 + (p3-p2) * (depth[1]/(btFabs(depth[2]) + depth[1]));
                
                //Calculate
                btVector3 fv1 = p2-p1; //One side of the face (triangle)
                btVector3 fv2 = p3-p1; //Another side of the face (triangle)
                fc = (p1+p2+p3)/btScalar(3); //Face centroid
        
                fn = fv1.cross(fv2); //Normal of the face (length != 1)
                btScalar len = fn.length();
                
                if(btFuzzyZero(len))
                    continue;
                
                fn1 = fn/len; //Normalised normal (length = 1)
                A = len/btScalar(2); //Area of the face (triangle)
            }
            else //depth[1] >= 0 && depth[2] >= 0
            {
                //Quad!!!!
                btVector3 p1temp = p2 + (p1-p2) * (depth[1]/(btFabs(depth[0]) + depth[1]));
                //p2 without change
                //p3 without change
                btVector3 p4 = p3 + (p1-p3) * (depth[2]/(btFabs(depth[0]) + depth[2]));
                p1 = p1temp;
                
                //Calculate
                btVector3 fv1 = p2-p1;
                btVector3 fv2 = p3-p1;
                btVector3 fv3 = p4-p1;
                
                fc = (p1 + p2 + p3 + p4)/btScalar(4);
                fn = fv1.cross(fv2);
                btScalar len = fn.length();
                
                if(btFuzzyZero(len))
                    continue;
                
                fn1 = fn/len;
                A = len + fv2.cross(fv3).length();
                fn = fn1 * A;
            }
        }
        else if(depth[1] < btScalar(0.))
        {
            if(depth[2] < btScalar(0.))
            {
                //p1 without change
                p2 = p1 + (p2-p1) * (depth[0]/(btFabs(depth[1]) + depth[0]));
                p3 = p1 + (p3-p1) * (depth[0]/(btFabs(depth[2]) + depth[0]));
                
                //Calculate
                btVector3 fv1 = p2-p1; //One side of the face (triangle)
                btVector3 fv2 = p3-p1; //Another side of the face (triangle)
                fc = (p1+p2+p3)/btScalar(3); //Face centroid
        
                fn = fv1.cross(fv2); //Normal of the face (length != 1)
                btScalar len = fn.length();
                
                if(btFuzzyZero(len))
                    continue;
                
                fn1 = fn/len; //Normalised normal (length = 1)
                A = len/btScalar(2); //Area of the face (triangle)
            }
            else
            {
                //Quad!!!!
                btVector3 p2temp = p1 + (p2-p1) * (depth[0]/(btFabs(depth[1]) + depth[0]));
                //p2 without change
                //p3 without change
                btVector3 p4 = p3 + (p2-p3) * (depth[2]/(btFabs(depth[1]) + depth[2]));
                p2 = p2temp;
                
                //Calculate
                btVector3 fv1 = p2-p1;
                btVector3 fv2 = p3-p1;
                btVector3 fv3 = p4-p1;
                
                fc = (p1 + p2 + p3 + p4)/btScalar(4);
                fn = fv1.cross(fv2);
                btScalar len = fn.length();
                
                if(btFuzzyZero(len))
                    continue;
                
                fn1 = fn/len;
                A = len + fv2.cross(fv3).length();
                fn = fn1 * A;
            }
        }
        else if(depth[2] < btScalar(0.))
        {
            
            //Quad!!!!
            btVector3 p3temp = p2 + (p3-p2) * (depth[1]/(btFabs(depth[2]) + depth[1]));
            //p2 without change
            //p3 without change
            btVector3 p4 = p1 + (p3-p1) * (depth[0]/(btFabs(depth[2]) + depth[0]));
            p3 = p3temp;
                
            //Calculate
            btVector3 fv1 = p2-p1;
            btVector3 fv2 = p3-p1;
            btVector3 fv3 = p4-p1;
                
            fc = (p1 + p2 + p3 + p4)/btScalar(4);
            fn = fv1.cross(fv2);
            btScalar len = fn.length();
            
            if(btFuzzyZero(len))
                continue;
            
            fn1 = fn/len;
            A = len + fv2.cross(fv3).length();
            fn = fn1 * A;
        }
        else //All underwater
        {
            btVector3 fv1 = p2-p1; //One side of the face (triangle)
            btVector3 fv2 = p3-p1; //Another side of the face (triangle)
            fc = (p1+p2+p3)/btScalar(3); //Face centroid
        
            fn = fv1.cross(fv2); //Normal of the face (length != 1)
            btScalar len = fn.length();
            
            if(btFuzzyZero(len))
                continue;
            
            fn1 = fn/len; //Normalised normal (length = 1)
            A = len/btScalar(2); //Area of the face (triangle)
        }
        
        btScalar pressure = liquid->GetPressure(fc);   //(liquid->GetPressure(p1) + liquid->GetPressure(p2) + liquid->GetPressure(p3))/btScalar(3);
        
        //Buoyancy force
        if(settings.reallisticBuoyancy && buoyant)
        {
            btVector3 Fbi = -fn1 * A * pressure;  //-fn/btScalar(2)*pressure; //Buoyancy force per face (based on pressure)
            
            //Accumulate
            _Fb += Fbi;
            _Tb += (fc - p).cross(Fbi);
        }
        
        //Damping force
        if(settings.dampingForces)
		{
            //Skin drag force
            btVector3 vc = liquid->GetFluidVelocity(fc) - (v + omega.cross(fc - p)); //Water velocity at face center
            btVector3 vt = vc - (vc.dot(fn)*fn)/fn.length2(); //Water velocity projected on face (tangent to face)
            btVector3 Fds = viscousity * vt * A / btScalar(0.0001);
            //btVector3 Fds = vt.safeNormalize()*btScalar(0.5)*fluid->getFluid()->density*btScalar(1.328)/1000.0*vt.length2()*fn.length()/btScalar(2);
        
            //Pressure drag force
            btVector3 vn = vc - vt; //Water velocity normal to face
            btVector3 Fdp(0,0,0);
            
            if(fn.dot(vn) < btScalar(0))
            {
                Fdp = btScalar(0.5) * liquid->getLiquid()->density * vn * vn.length() * A;
            }
            
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

void SolidEntity::ComputeFluidForces(HydrodynamicsSettings settings, const Ocean* liquid)
{
    if(!computeHydro)
        return;
    
    btTransform T = getTransform() * localTransform.inverse();
    btVector3 v = getLinearVelocity();
    btVector3 omega = getAngularVelocity();
	btVector3 a = getLinearAcceleration();
    btVector3 epsilon = getAngularAcceleration();
    btTransform eTrans = getTransform() * localTransform.inverse() * ellipsoidTransform;
    
    //Check if fully submerged --> simplifies buoyancy calculation
    btVector3 aabbMin, aabbMax;
    GetAABB(aabbMin, aabbMax);
    
    if(liquid->GetDepth(aabbMin) > btScalar(0) && liquid->GetDepth(aabbMax) > btScalar(0))
    {
        Fb = -volume*liquid->getLiquid()->density * SimulationApp::getApp()->getSimulationManager()->getGravity();
        Tb = (T*CoB - getTransform().getOrigin()).cross(Fb);
        settings.reallisticBuoyancy = false; //disable buoyancy calculation
    }
    
    if(settings.dampingForces || settings.reallisticBuoyancy)
    {
        ComputeFluidForces(settings, liquid, getTransform(), T, v, omega, a, epsilon, Fb, Tb, Fds, Tds, Fdp, Tdp);
        
        if(settings.dampingForces)
        {
            //Correct drag based on ellipsoid approximation of shape
            btVector3 eFd = eTrans.getBasis().inverse() * Fdp;
            //btVector3 eTd = eTrans.getBasis().inverse() * Td;
            btVector3 Cd(btScalar(1)/ellipsoidR.x(), btScalar(1)/ellipsoidR.y(), btScalar(1)/ellipsoidR.z());
            btScalar maxCd = btMax(btMax(Cd.x(), Cd.y()), Cd.z());
            Cd /= maxCd;
            eFd = btVector3(Cd.x()*eFd.x(), Cd.y()*eFd.y(), Cd.z()*eFd.z());
            Fdp = eTrans.getBasis() * eFd;
        }
    }
    
    /*
    if(settings.addedMassForces)
    {
        btScalar rho = liquid->getFluid()->density;
        btVector3 ea = eTrans.getBasis().inverse() * a;
        
        btScalar r2 = (ellipsoidR.y() + ellipsoidR.z())/btScalar(2);
        btScalar m11 = LambKFactor(ellipsoidR.x(), r2)*btScalar(4)/btScalar(3)*M_PI*rho*ellipsoidR.x()*r2*r2;
        btScalar m22 = 0;
        btScalar m33 = 0;
        
        btVector3 eFa = btVector3(-ea.x() * m11, -ea.y() * m22, -ea.z() * m33);
        Fa = eTrans.getBasis() * eFa;
        
        //Ta = -epsilon * 0.333 * Ipri;
#ifdef DEBUG
        std::cout << getName() << " added mass: " << Fa.x() << ", " << Fa.y() << ", " << Fa.z() << " " << Ta.x() << ", " << Ta.y() << ", " << Ta.z() << std::endl;
#endif
    }*/
}

void SolidEntity::ApplyFluidForces()
{
    ApplyCentralForce(Fb + Fds + Fdp + Fa);
    ApplyTorque(Tb + Tds + Tdp + Ta);
}
