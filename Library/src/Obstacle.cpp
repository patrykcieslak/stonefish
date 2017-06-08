//
//  Obstacle.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 24/05/2014.
//  Copyright (c) 2014 Patryk Cieslak. All rights reserved.
//

#include "Obstacle.h"

Obstacle::Obstacle(std::string uniqueName, const char* modelFilename, btScalar scale, Material* mat, const btTransform& worldTransform, int lookId, bool smoothNormals) : StaticEntity(uniqueName, mat, lookId)
{
    //Load mesh
    scale = UnitSystem::SetLength(scale);
    mesh = OpenGLContent::LoadMesh(modelFilename, scale, smoothNormals);
    
    //Build collision shape
	/*
	int* indices = new int[mesh->faces.size()*3];
    for(int i=0; i<mesh->faces.size(); i++)
    {
        indices[i*3+0] = mesh->faces[i].vertexID[0];
        indices[i*3+1] = mesh->faces[i].vertexID[1];
        indices[i*3+2] = mesh->faces[i].vertexID[2];
    }
	
    triangleArray = new btTriangleIndexVertexArray(mesh->faces.size(), indices, 3*sizeof(int),
                                                   mesh->vertices.size(), (GLfloat*)&mesh->vertices[0].x, sizeof(glm::vec3));
    btBvhTriangleMeshShape* shape = new btBvhTriangleMeshShape(triangleArray, true);
    
	
    //Build rigid body
    btDefaultMotionState* motionState = new btDefaultMotionState(UnitSystem::SetTransform(worldTransform));
    btRigidBody::btRigidBodyConstructionInfo rigidBodyCI(0, motionState, shape, btVector3(0,0,0));
    rigidBodyCI.m_friction = rigidBodyCI.m_rollingFriction = rigidBodyCI.m_restitution = btScalar(1.); //not used
    rigidBody = new btRigidBody(rigidBodyCI);
    rigidBody->setUserPointer(this);
    rigidBody->setCollisionFlags(rigidBody->getCollisionFlags() | btCollisionObject::CF_STATIC_OBJECT | btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);
    
	objectId = OpenGLContent::getInstance()->BuildObject(mesh);
	*/
}

Obstacle::~Obstacle()
{
    delete mesh;
}

StaticEntityType Obstacle::getStaticType()
{
    return STATIC_OBSTACLE;
}