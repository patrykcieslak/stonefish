//
//  Obstacle.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 24/05/2014.
//  Copyright (c) 2014-2017 Patryk Cieslak. All rights reserved.
//

#include "Obstacle.h"

Obstacle::Obstacle(std::string uniqueName, std::string modelFilename, btScalar scale, Material m, int lookId, bool smoothNormals) : StaticEntity(uniqueName, m, lookId)
{
    scale = UnitSystem::SetLength(scale);
    
    mesh = OpenGLContent::LoadMesh(modelFilename, scale, smoothNormals);
    objectId = OpenGLContent::getInstance()->BuildObject(mesh);
    
    btScalar* vertices = new btScalar[mesh->vertices.size()*3];
    int* indices = new int[mesh->faces.size()*3];
    
    for(unsigned int i=0; i<mesh->vertices.size(); ++i)
    {
        vertices[i*3+0] = mesh->vertices[i].pos.x;
        vertices[i*3+1] = mesh->vertices[i].pos.y;
        vertices[i*3+2] = mesh->vertices[i].pos.z;
    }
    
	for(unsigned int i=0; i<mesh->faces.size(); ++i)
    {
        indices[i*3+0] = mesh->faces[i].vertexID[0];
        indices[i*3+1] = mesh->faces[i].vertexID[1];
        indices[i*3+2] = mesh->faces[i].vertexID[2];
    }
	    
    btTriangleIndexVertexArray* triangleArray = new btTriangleIndexVertexArray(mesh->faces.size(), indices, 3*sizeof(int),
                                                                               mesh->vertices.size(), vertices, 3*sizeof(btScalar));
    btBvhTriangleMeshShape* shape = new btBvhTriangleMeshShape(triangleArray, true);
    shape->setMargin(UnitSystem::Length(UnitSystems::MKS, UnitSystem::GetInternalUnitSystem(), btScalar(0.001)));
    BuildRigidBody(shape);
    
    delete[] vertices;
    delete[] indices;
    delete triangleArray;
}

Obstacle::Obstacle(std::string uniqueName, btScalar sphereRadius, Material m, int lookId) : StaticEntity(uniqueName, m, lookId)
{
    sphereRadius = UnitSystem::SetLength(sphereRadius);
    
    btSphereShape* shape = new btSphereShape(sphereRadius);
    shape->setMargin(UnitSystem::Length(UnitSystems::MKS, UnitSystem::GetInternalUnitSystem(), btScalar(0.001)));
    BuildRigidBody(shape);
    
    mesh = OpenGLContent::BuildSphere(sphereRadius);
    objectId = OpenGLContent::getInstance()->BuildObject(mesh);
}

Obstacle::Obstacle(std::string uniqueName, btVector3 boxDimensions, Material m, int lookId) : StaticEntity(uniqueName, m, lookId)
{
    btVector3 halfExtents = UnitSystem::SetPosition(boxDimensions/btScalar(2));
    
    btBoxShape* shape = new btBoxShape(halfExtents);
    shape->setMargin(UnitSystem::Length(UnitSystems::MKS, UnitSystem::GetInternalUnitSystem(), btScalar(0.001)));
    BuildRigidBody(shape);
    
    glm::vec3 glHalfExtents(halfExtents.x(), halfExtents.y(), halfExtents.z());
	mesh = OpenGLContent::BuildBox(glHalfExtents);
	objectId = OpenGLContent::getInstance()->BuildObject(mesh);
}

Obstacle::Obstacle(std::string uniqueName, btScalar cylinderRadius, btScalar cylinderHeight, Material m, int lookId) : StaticEntity(uniqueName, m, lookId)
{
    cylinderRadius = UnitSystem::SetLength(cylinderRadius);
    btScalar halfHeight = UnitSystem::SetLength(cylinderHeight/btScalar(2));
    
    btCylinderShape* shape = new btCylinderShape(btVector3(cylinderRadius, halfHeight, cylinderRadius));
    shape->setMargin(UnitSystem::Length(UnitSystems::MKS, UnitSystem::GetInternalUnitSystem(), btScalar(0.001)));
    BuildRigidBody(shape);
    
    mesh = OpenGLContent::BuildCylinder((GLfloat)cylinderRadius, (GLfloat)halfHeight*2.f);
	objectId = OpenGLContent::getInstance()->BuildObject(mesh);
}

StaticEntityType Obstacle::getStaticType()
{
    return STATIC_OBSTACLE;
}