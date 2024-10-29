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
//  Obstacle.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 24/05/2014.
//  Copyright (c) 2014-2024 Patryk Cieslak. All rights reserved.
//

#include "entities/statics/Obstacle.h"

#include "core/GraphicalSimulationApp.h"
#include "graphics/OpenGLPipeline.h"
#include "graphics/OpenGLContent.h"

namespace sf
{

Obstacle::Obstacle(std::string uniqueName,
         std::string graphicsFilename, Scalar graphicsScale, const Transform& graphicsOrigin,
         std::string physicsFilename, Scalar physicsScale, const Transform& physicsOrigin, bool convexHull,
         std::string material, std::string look) : StaticEntity(uniqueName, material, look)
{
    graMesh = OpenGLContent::LoadMesh(graphicsFilename, graphicsScale, false);
    OpenGLContent::TransformMesh(graMesh, graphicsOrigin);
    
    if(physicsFilename != "")
    {
        phyMesh = OpenGLContent::LoadMesh(physicsFilename, physicsScale, false);
        OpenGLContent::TransformMesh(phyMesh, physicsOrigin);
    }
    else
        phyMesh = graMesh;
        
    graObjectId = -1;

    //Buidling collision shape
    if(convexHull) // Convex approximation
    {
        btConvexHullShape* shape = new btConvexHullShape();
        for(size_t i=0; i<phyMesh->getNumOfVertices(); ++i)
        {
            glm::vec3 pos = phyMesh->getVertexPos(i);
            Vector3 v(pos.x, pos.y, pos.z);
            shape->addPoint(v);
        }
        //shape->optimizeConvexHull();
        shape->setMargin(0);
        BuildRigidBody(shape);    
    }
    else // Non-convex (arbitrary triangle mesh)
    {
        Scalar* vertices = new Scalar[phyMesh->getNumOfVertices() * 3];
        int* indices = new int[phyMesh->faces.size()*3];
        
        for(size_t i=0; i<phyMesh->getNumOfVertices(); ++i)
        {
            glm::vec3 pos = phyMesh->getVertexPos(i);
            vertices[i*3+0] = pos.x;
            vertices[i*3+1] = pos.y;
            vertices[i*3+2] = pos.z;
        }
        
        for(size_t i=0; i<phyMesh->faces.size(); ++i)
        {
            indices[i*3+0] = phyMesh->faces[i].vertexID[0];
            indices[i*3+1] = phyMesh->faces[i].vertexID[1];
            indices[i*3+2] = phyMesh->faces[i].vertexID[2];
        }
        
        btTriangleIndexVertexArray* triangleArray = new btTriangleIndexVertexArray((int)phyMesh->faces.size(), indices, 3*sizeof(int),
                                                                                (int)phyMesh->getNumOfVertices(), vertices, 3*sizeof(Scalar));
        btBvhTriangleMeshShape* shape = new btBvhTriangleMeshShape(triangleArray, true);
        shape->setMargin(0);
        BuildRigidBody(shape);
    }
}
    
Obstacle::Obstacle(std::string uniqueName, std::string modelFilename, Scalar scale, const Transform& origin, bool convexHull, std::string material, std::string look)
    : Obstacle(uniqueName, modelFilename, scale, origin, "", scale, origin, convexHull, material, look)
{
}

Obstacle::Obstacle(std::string uniqueName, Scalar sphereRadius, const Transform& origin, std::string material, std::string look) : StaticEntity(uniqueName, material, look)
{
    phyMesh = OpenGLContent::BuildSphere(sphereRadius);
    graMesh = phyMesh;
    graObjectId = -1;
    
    btSphereShape* shape = new btSphereShape(sphereRadius);
    if(origin == I4())
        BuildRigidBody(shape);
    else
    {
        OpenGLContent::TransformMesh(phyMesh, origin);
        btCompoundShape* cShape = new btCompoundShape();
        cShape->addChildShape(origin, shape);
        BuildRigidBody(cShape);
    }
}

Obstacle::Obstacle(std::string uniqueName, Vector3 boxDimensions, const Transform& origin, std::string material, std::string look, unsigned int uvMode) : StaticEntity(uniqueName, material, look)
{
    Vector3 halfExtents = boxDimensions/Scalar(2);
    glm::vec3 glHalfExtents(halfExtents.x(), halfExtents.y(), halfExtents.z());
	phyMesh = OpenGLContent::BuildBox(glHalfExtents, 0, uvMode);
    graMesh = phyMesh;
	graObjectId = -1;
    
    btBoxShape* shape = new btBoxShape(halfExtents);
    shape->setMargin(COLLISION_MARGIN);
    if(origin == I4())
        BuildRigidBody(shape);
    else
    {
        OpenGLContent::TransformMesh(phyMesh, origin);
        btCompoundShape* cShape = new btCompoundShape();
        cShape->addChildShape(origin, shape);
        BuildRigidBody(cShape);
    }
}

Obstacle::Obstacle(std::string uniqueName, Scalar cylinderRadius, Scalar cylinderHeight, const Transform& origin, std::string material, std::string look) : StaticEntity(uniqueName, material, look)
{
    Scalar halfHeight = cylinderHeight/Scalar(2);
    phyMesh = OpenGLContent::BuildCylinder((GLfloat)cylinderRadius, (GLfloat)cylinderHeight, (unsigned int)btMax(ceil(2.0*M_PI*cylinderRadius/0.1), 32.0)); //Max 0.1 m cylinder wall slice width
    graMesh = phyMesh;
    graObjectId = -1;
    
    btCylinderShape* shape = new btCylinderShapeZ(Vector3(cylinderRadius, cylinderRadius, halfHeight));
    shape->setMargin(COLLISION_MARGIN);
    if(origin == I4())
        BuildRigidBody(shape);
    else
    {
        OpenGLContent::TransformMesh(phyMesh, origin);
        btCompoundShape* cShape = new btCompoundShape();
        cShape->addChildShape(origin, shape);
        BuildRigidBody(cShape);
    }
}
    
Obstacle::~Obstacle()
{
    if(graMesh != nullptr && graMesh != phyMesh)
        delete graMesh;
}

StaticEntityType Obstacle::getStaticType()
{
    return StaticEntityType::OBSTACLE;
}
    
void Obstacle::BuildGraphicalObject()
{
    if(graMesh == nullptr || !SimulationApp::getApp()->hasGraphics())
        return;
        
    graObjectId = ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->BuildObject(graMesh);
    phyObjectId = ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->BuildObject(phyMesh);
}

std::vector<Renderable> Obstacle::Render()
{
    std::vector<Renderable> items(0);
	
    if(rigidBody != nullptr && isRenderable())
    {
        Renderable item;
        item.type = RenderableType::SOLID;
        item.materialName = mat.name;
        
        if(dm == DisplayMode::GRAPHICAL && graObjectId >= 0)
        { 
            item.objectId = graObjectId;
            item.lookId = lookId;
            item.model = glMatrixFromTransform(getTransform());
            items.push_back(item);
        }
        else if(dm == DisplayMode::PHYSICAL && phyObjectId >= 0)
        {
            item.objectId = phyObjectId;
            item.lookId = -1;
            item.model = glMatrixFromTransform(getTransform());
            items.push_back(item);
        }
    }
	
	return items;
}

}
