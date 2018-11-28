//
//  Obstacle.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 24/05/2014.
//  Copyright (c) 2014-2017 Patryk Cieslak. All rights reserved.
//

#include "entities/statics/Obstacle.h"

using namespace sf;

Obstacle::Obstacle(std::string uniqueName, std::string modelFilename, Scalar scale, Material m, int lookId, bool smoothNormals) : StaticEntity(uniqueName, m, lookId)
{
    mesh = OpenGLContent::LoadMesh(modelFilename, scale, smoothNormals);
    
    Scalar* vertices = new Scalar[mesh->vertices.size()*3];
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
	    
    btTriangleIndexVertexArray* triangleArray = new btTriangleIndexVertexArray((int)mesh->faces.size(), indices, 3*sizeof(int),
                                                                               (int)mesh->vertices.size(), vertices, 3*sizeof(Scalar));
    btBvhTriangleMeshShape* shape = new btBvhTriangleMeshShape(triangleArray, true);
    BuildRigidBody(shape);
    
    //delete[] vertices;
    //delete[] indices;
    //delete triangleArray;
}

Obstacle::Obstacle(std::string uniqueName, Scalar sphereRadius, Material m, int lookId) : StaticEntity(uniqueName, m, lookId)
{
    mesh = OpenGLContent::BuildSphere(sphereRadius);
    
    btSphereShape* shape = new btSphereShape(sphereRadius);
    BuildRigidBody(shape);
}

Obstacle::Obstacle(std::string uniqueName, Vector3 boxDimensions, Material m, int lookId) : StaticEntity(uniqueName, m, lookId)
{
    Vector3 halfExtents = boxDimensions/Scalar(2);
    glm::vec3 glHalfExtents(halfExtents.x(), halfExtents.y(), halfExtents.z());
	mesh = OpenGLContent::BuildBox(glHalfExtents);
	
    btBoxShape* shape = new btBoxShape(halfExtents);
    BuildRigidBody(shape);
}

Obstacle::Obstacle(std::string uniqueName, Scalar cylinderRadius, Scalar cylinderHeight, Material m, int lookId) : StaticEntity(uniqueName, m, lookId)
{
    Scalar halfHeight = cylinderHeight/Scalar(2);
    mesh = OpenGLContent::BuildCylinder((GLfloat)cylinderRadius, (GLfloat)halfHeight*2.f);
	
    btCylinderShape* shape = new btCylinderShape(Vector3(cylinderRadius, halfHeight, cylinderRadius));
    BuildRigidBody(shape);
}

StaticEntityType Obstacle::getStaticType()
{
    return STATIC_OBSTACLE;
}
