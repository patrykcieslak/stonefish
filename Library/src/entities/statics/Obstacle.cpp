//
//  Obstacle.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 24/05/2014.
//  Copyright (c) 2014-2018 Patryk Cieslak. All rights reserved.
//

#include "entities/statics/Obstacle.h"

#include "core/GraphicalSimulationApp.h"
#include "graphics/OpenGLPipeline.h"
#include "graphics/OpenGLContent.h"

namespace sf
{

Obstacle::Obstacle(std::string uniqueName,
         std::string graphicsFilename, Scalar graphicsScale, const Transform& graphicsOrigin,
         std::string physicsFilename, Scalar physicsScale, const Transform& physicsOrigin,
         Material m, int lookId, bool smoothGraphicsNormals) : StaticEntity(uniqueName, m, lookId)
{
    
    graMesh = OpenGLContent::LoadMesh(graphicsFilename, graphicsScale, smoothGraphicsNormals);
    OpenGLContent::TransformMesh(graMesh, graphicsOrigin);
    
    if(physicsFilename != "")
    {
        phyMesh = OpenGLContent::LoadMesh(physicsFilename, physicsScale, false);
        OpenGLContent::TransformMesh(phyMesh, physicsOrigin);
    }
    else
        phyMesh = graMesh;
    
    Scalar* vertices = new Scalar[phyMesh->vertices.size()*3];
    int* indices = new int[phyMesh->faces.size()*3];
    
    for(unsigned int i=0; i<phyMesh->vertices.size(); ++i)
    {
        vertices[i*3+0] = phyMesh->vertices[i].pos.x;
        vertices[i*3+1] = phyMesh->vertices[i].pos.y;
        vertices[i*3+2] = phyMesh->vertices[i].pos.z;
    }
    
    for(unsigned int i=0; i<phyMesh->faces.size(); ++i)
    {
        indices[i*3+0] = phyMesh->faces[i].vertexID[0];
        indices[i*3+1] = phyMesh->faces[i].vertexID[1];
        indices[i*3+2] = phyMesh->faces[i].vertexID[2];
    }
    
    btTriangleIndexVertexArray* triangleArray = new btTriangleIndexVertexArray((int)phyMesh->faces.size(), indices, 3*sizeof(int),
                                                                               (int)phyMesh->vertices.size(), vertices, 3*sizeof(Scalar));
    btBvhTriangleMeshShape* shape = new btBvhTriangleMeshShape(triangleArray, true);
    BuildRigidBody(shape);
    
    //delete[] vertices;
    //delete[] indices;
    //delete triangleArray;
}
    
Obstacle::Obstacle(std::string uniqueName, std::string modelFilename, Scalar scale, const Transform& origin, Material m, int lookId, bool smoothNormals)
    : Obstacle(uniqueName, modelFilename, scale, origin, "", scale, origin, m, lookId, smoothNormals)
{
}

Obstacle::Obstacle(std::string uniqueName, Scalar sphereRadius, Material m, int lookId) : StaticEntity(uniqueName, m, lookId)
{
    phyMesh = OpenGLContent::BuildSphere(sphereRadius);
    graMesh = phyMesh;
    
    btSphereShape* shape = new btSphereShape(sphereRadius);
    BuildRigidBody(shape);
}

Obstacle::Obstacle(std::string uniqueName, Vector3 boxDimensions, Material m, int lookId, unsigned int uvMode) : StaticEntity(uniqueName, m, lookId)
{
    Vector3 halfExtents = boxDimensions/Scalar(2);
    glm::vec3 glHalfExtents(halfExtents.x(), halfExtents.y(), halfExtents.z());
	phyMesh = OpenGLContent::BuildBox(glHalfExtents, 0, uvMode);
    graMesh = phyMesh;
	
    btBoxShape* shape = new btBoxShape(halfExtents);
    BuildRigidBody(shape);
}

Obstacle::Obstacle(std::string uniqueName, Scalar cylinderRadius, Scalar cylinderHeight, Material m, int lookId) : StaticEntity(uniqueName, m, lookId)
{
    Scalar halfHeight = cylinderHeight/Scalar(2);
    phyMesh = OpenGLContent::BuildCylinder((GLfloat)cylinderRadius, (GLfloat)halfHeight*2.f);
    graMesh = phyMesh;
    
    btCylinderShape* shape = new btCylinderShapeZ(Vector3(cylinderRadius, cylinderRadius, halfHeight));
    BuildRigidBody(shape);
}
    
Obstacle::~Obstacle()
{
    if(graMesh != NULL)
    {
        if(graMesh == phyMesh) phyMesh = NULL;
        delete graMesh;
    }
}

StaticEntityType Obstacle::getStaticType()
{
    return STATIC_OBSTACLE;
}
    
void Obstacle::BuildGraphicalObject()
{
    if(graMesh == NULL || !SimulationApp::getApp()->hasGraphics())
        return;
        
    objectId = ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->BuildObject(graMesh);
}

}
