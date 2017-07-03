//
//  Plane.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 1/30/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#include "Plane.h"

#include "SimulationApp.h"

Plane::Plane(std::string uniqueName, btScalar planeSize, Material* mat, const btTransform& worldTransform, int lookId) : StaticEntity(uniqueName, mat, lookId)
{
    size = UnitSystem::SetLength(planeSize);
    
    btDefaultMotionState* motionState = new btDefaultMotionState(UnitSystem::SetTransform(worldTransform));
    btCollisionShape* shape = new btStaticPlaneShape(SimulationApp::getApp()->getSimulationManager()->isZAxisUp() ? btVector3(0,0,1.) : btVector3(0,0,-1.),0);
    
    btRigidBody::btRigidBodyConstructionInfo rigidBodyCI(0, motionState, shape, btVector3(0.,0.,0.));
    rigidBodyCI.m_friction = rigidBodyCI.m_rollingFriction = rigidBodyCI.m_restitution = btScalar(0); //not used
    rigidBody = new btRigidBody(rigidBodyCI);
    rigidBody->setUserPointer(this);
    rigidBody->setCollisionFlags(rigidBody->getCollisionFlags() | btCollisionObject::CF_STATIC_OBJECT | btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);
    
	mesh = OpenGLContent::BuildPlane(size/(GLfloat)2);
	objectId = OpenGLContent::getInstance()->BuildObject(mesh);
}

Plane::~Plane()
{
}

void Plane::GetAABB(btVector3 &min, btVector3 &max)
{
    //Plane shouldn't affect shadow calculation
    min.setValue(BT_LARGE_FLOAT, BT_LARGE_FLOAT, BT_LARGE_FLOAT);
    max.setValue(-BT_LARGE_FLOAT, -BT_LARGE_FLOAT, -BT_LARGE_FLOAT);
}

StaticEntityType Plane::getStaticType()
{
    return STATIC_PLANE;
}
