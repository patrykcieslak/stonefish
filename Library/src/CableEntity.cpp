//
//  CableEntity.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 12/29/12.
//  Copyright(c) 2012-2017 Patryk Cieslak. All rights reserved.
//

#include "CableEntity.h"
#include "Joint.h"
#include "MathsUtil.hpp"

CableEntity::CableEntity(std::string uniqueName, const btVector3& _end1, const btVector3& _end2, unsigned int parts, btScalar diam, btScalar stiffness, bool selfCollidable, Material* mat) : Entity(uniqueName)
{
    material = mat;
    diameter = UnitSystem::SetLength(diam);
    selfCollision = selfCollidable;
    btVector3 end1 = UnitSystem::SetPosition(_end1);
    btVector3 end2 = UnitSystem::SetPosition(_end2);
    
    btVector3 partVec = (end2-end1)/(btScalar)parts;
    btVector3 partDir = partVec.normalized();
    partLength = partVec.length();
    
    if(partLength < 2.0*diameter)
    {
        parts = (int)floor((end2-end1).length()/(2.0*diameter));
        partVec = (end2-end1)/(btScalar)parts;
        partLength = partVec.length();
    }
    
    btCollisionShape* shape = new btCapsuleShape(diameter/2.0, partLength);
    partVolume = M_PI_4*diameter*diameter * partLength;
    btScalar mass = partVolume * material->density;
    btVector3 inertia(0,0,0);
    shape->calculateLocalInertia(mass, inertia);
    
    btMatrix3x3 rotation;
    btVector3 up = btVector3(0, 0, 1);
    if(fabs(partDir.z())>0.8)
        up = btVector3(0,1,0);
    btVector3 front = partDir;
    btVector3 left = front.cross(up);
    left.normalize();
    up = left.cross(front);
    up.normalize();
    rotation.setValue(left.x(), front.x(), up.x(), left.y(), front.y(), up.y(), left.z(), front.z(), up.z());
    
    for(int i=0; i<parts; i++)
    {
        btVector3 center = end1 + (i+0.5)*partVec;
        btDefaultMotionState* motionState = new btDefaultMotionState(btTransform(rotation, center));
        
        btRigidBody::btRigidBodyConstructionInfo rigidBodyCI(mass, motionState, shape, inertia);
        rigidBodyCI.m_friction = rigidBodyCI.m_rollingFriction = rigidBodyCI.m_restitution = btScalar(1.); //not used
        rigidBodyCI.m_linearDamping = 0;
        rigidBodyCI.m_angularDamping = 0;
        rigidBodyCI.m_linearSleepingThreshold = 0.0;
        rigidBodyCI.m_angularSleepingThreshold = 0.0;
        
        btRigidBody* cablePart = new btRigidBody(rigidBodyCI);
        cablePart->setActivationState(DISABLE_DEACTIVATION);
        cablePart->setUserPointer(this);
        cablePart->setCollisionFlags(cablePart->getCollisionFlags() | btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);
        
        cableParts.push_back(cablePart);
    }
    
    btVector3 pivotInA(0,0.5*partLength, 0);
    btVector3 pivotInB(0,-0.5*partLength,0);
    btTransform frameInA(btMatrix3x3().getIdentity(), pivotInA);
    btTransform frameInB(btMatrix3x3().getIdentity(), pivotInB);
    
    for(int i=1; i<parts; i++)
    {
        btGeneric6DofSpringConstraint* constr = new btGeneric6DofSpringConstraint(*cableParts[i-1], *cableParts[i], frameInA, frameInB, true);
        constr->setLinearLowerLimit(btVector3(0,0,0));
        constr->setLinearUpperLimit(btVector3(0,0,0));
        constr->setAngularLowerLimit(btVector3(-M_PI/2.0,-M_PI/3.0,-M_PI/2.0));
        constr->setAngularUpperLimit(btVector3(M_PI/2.0,M_PI/3.0,M_PI/2.0));
        constr->setStiffness(3, stiffness);
        constr->setStiffness(4, 10*stiffness);
        constr->setStiffness(5, stiffness);
        constr->enableSpring(3, true);
        constr->enableSpring(4, true);
        constr->enableSpring(5, true);
        constr->setDamping(3, 0.5);
        constr->setDamping(4, 0.8);
        constr->setDamping(5, 0.5);
        
        constr->setParam(BT_CONSTRAINT_CFM, CONSTRAINT_CFM, 0);
        constr->setParam(BT_CONSTRAINT_STOP_ERP, CONSTRAINT_STOP_ERP, 0);
        constr->setParam(BT_CONSTRAINT_STOP_CFM, CONSTRAINT_STOP_CFM, 0);
        
        constr->setParam(BT_CONSTRAINT_CFM, CONSTRAINT_CFM, 1);
        constr->setParam(BT_CONSTRAINT_STOP_ERP, CONSTRAINT_STOP_ERP, 1);
        constr->setParam(BT_CONSTRAINT_STOP_CFM, CONSTRAINT_STOP_CFM, 1);
        
        constr->setParam(BT_CONSTRAINT_CFM, CONSTRAINT_CFM, 2);
        constr->setParam(BT_CONSTRAINT_STOP_ERP, CONSTRAINT_STOP_ERP, 2);
        constr->setParam(BT_CONSTRAINT_STOP_CFM, CONSTRAINT_STOP_CFM, 2);
        
        constr->setParam(BT_CONSTRAINT_CFM, CONSTRAINT_CFM, 3);
        constr->setParam(BT_CONSTRAINT_STOP_ERP, CONSTRAINT_STOP_ERP, 3);
        constr->setParam(BT_CONSTRAINT_STOP_CFM, CONSTRAINT_STOP_CFM, 3);
        
        constr->setParam(BT_CONSTRAINT_CFM, CONSTRAINT_CFM, 4);
        constr->setParam(BT_CONSTRAINT_STOP_ERP, CONSTRAINT_STOP_ERP, 4);
        constr->setParam(BT_CONSTRAINT_STOP_CFM, CONSTRAINT_STOP_CFM, 4);
        
        constr->setParam(BT_CONSTRAINT_CFM, CONSTRAINT_CFM, 5);
        constr->setParam(BT_CONSTRAINT_STOP_ERP, CONSTRAINT_STOP_ERP, 5);
        constr->setParam(BT_CONSTRAINT_STOP_CFM, CONSTRAINT_STOP_CFM, 5);
        links.push_back(constr);
    }
	
	partMesh = OpenGLContent::BuildCylinder(diameter/2.f, partLength);
	lookId = -1;
}

CableEntity::~CableEntity()
{
    cableParts.clear();
    links.clear();
    material = NULL;
	
	if(partMesh != NULL)
	{
		delete partMesh;
		partMesh = NULL;
	}
	
	objectId = -1;
	lookId = -1;
}

EntityType CableEntity::getType()
{
    return ENTITY_CABLE;
}

void CableEntity::SetLook(int newLookId)
{
	lookId = newLookId;
}

std::vector<Renderable> CableEntity::Render()
{
	std::vector<Renderable> items(0);
	
    if(isRenderable())
    {
		for(unsigned int i=0; i<cableParts.size(); ++i)
        {
            btTransform trans;
            cableParts[i]->getMotionState()->getWorldTransform(trans);
            
			Renderable item;
			item.objectId = objectId;
			item.lookId = lookId;
			item.model = glMatrixFromBtTransform(trans);
			items.push_back(item);
        }
    }
	
	return items;
}

Material* CableEntity::getMaterial()
{
    return material;
}

btRigidBody* CableEntity::getFirstEnd()
{
    return cableParts[0];
}

btRigidBody* CableEntity::getSecondEnd()
{
    return cableParts[cableParts.size()-1];
}

btScalar CableEntity::getPartVolume()
{
    return partVolume;
}

void CableEntity::ApplyGravity()
{
    for(int i=0; i<cableParts.size(); i++)
        cableParts[i]->applyGravity();
}

void CableEntity::AddToDynamicsWorld(btMultiBodyDynamicsWorld *world)
{
    for(int i=0; i<cableParts.size(); i++)
    {
        if(selfCollision)
            world->addRigidBody(cableParts[i], i % 2 == 0 ? MASK_CABLE_EVEN : MASK_CABLE_ODD, (i % 2 == 0 ? MASK_CABLE_EVEN : MASK_CABLE_ODD) | MASK_DEFAULT | MASK_STATIC);
        else
            world->addRigidBody(cableParts[i], MASK_CABLE_EVEN, MASK_DEFAULT | MASK_STATIC);
    }
    
    for(int i=0; i<links.size(); i++)
        world->addConstraint(links[i]);
		
	//Generate graphical object
	objectId = OpenGLContent::getInstance()->BuildObject(partMesh);
}


void CableEntity::CalculateFluidDynamics(const btVector3& surfaceN, const btVector3&surfaceD, const btVector3&fluidV, const Fluid* fluid,
                                         btScalar& submergedVolume, btVector3& cob,  btVector3& drag, btVector3& angularDrag,
                                         const btTransform& worldTransform, const btVector3& velocity, const btVector3& angularVelocity)
{
    //0.Initialize
    drag = btVector3(0,0,0);
    angularDrag = btVector3(0,0,0);
    cob = btVector3(0,0,0);
    submergedVolume = 0;
    
    //1.Transform plane into rigid body local coordinate system
    btVector3 localSurfaceN = worldTransform.getBasis().inverse() * surfaceN;
    btVector3 localSurfaceD = worldTransform.inverse() * surfaceD;
    btVector3 localFluidV = worldTransform.getBasis().inverse() * (fluidV - velocity);
    btVector3 localAngularV = worldTransform.getBasis().inverse() * angularVelocity;
    
    btScalar d1 = distanceFromCenteredPlane(localSurfaceN, btVector3(0.0, -partLength/2.0, 0.0)-localSurfaceD);
    btScalar d2 = distanceFromCenteredPlane(localSurfaceN, btVector3(0.0, partLength/2.0, 0.0)-localSurfaceD);
    
    if((d1 < -diameter/2.0) && (d2 < -diameter/2.0)) //fully submerged
    {
        submergedVolume = partVolume;
    }
    else if((btFabs(d1) <= diameter/2.0) && (btFabs(d2) <= diameter/2.0)) //partially submerged, approx horizontal
    {
        //full equation is V = L*(R^2*acos((R-h)/R)-(R-h)*sqrt(2*R*h-h^2))
        //linearized V = (pi*R*L/2)*h
        submergedVolume = (M_PI*diameter/2.0*partLength/2.0)*(diameter-(d1+d2)/2.0);
        cob = localSurfaceN * (submergedVolume/partVolume * diameter - diameter)/2.0;
    }
    else if(d1 * d2 < 0) //part crosses surface
    {
        if(d1 > 0) //d1 above fluid
        {
            btScalar fraction = -d2/(-d2+d1);
            submergedVolume = partVolume * fraction;
            cob.setY(partLength/2.0 - partLength * fraction/2.0);
        }
        else //d2 above fluid
        {
            btScalar fraction = -d1/(-d1+d2);
            submergedVolume = partVolume * fraction;
            cob.setY(-partLength/2.0 + partLength * fraction/2.0);
        }
    }
    //else //part doesnt cross surface and is not above it - must be underwater with one distance < diameter/2
    //{
    //    submergedVolume = partVolume;
    //}
    
    drag =  btVector3(localFluidV.x()*btFabs(localFluidV.x())*partLength*diameter*0.5*fluid->density, localFluidV.y()*partLength*fluid->viscosity, localFluidV.z()*btFabs(localFluidV.z())*partLength*diameter*0.5*fluid->density);
    drag = worldTransform.getBasis()*drag;
    angularDrag = worldTransform.getBasis()*(fluid->viscosity*localAngularV);
    cob = worldTransform.getBasis() * cob;
}
