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
    
Obstacle::Obstacle(std::string uniqueName, std::string modelFilename, Scalar scale, const Transform& origin, std::string material, std::string look)
    : Obstacle(uniqueName, modelFilename, scale, origin, "", scale, origin, material, look)
{
}

Obstacle::Obstacle(std::string uniqueName, Scalar sphereRadius, std::string material, std::string look) : StaticEntity(uniqueName, material, look)
{
    phyMesh = OpenGLContent::BuildSphere(sphereRadius);
    graMesh = phyMesh;
    graObjectId = -1;
    
    btSphereShape* shape = new btSphereShape(sphereRadius);
    BuildRigidBody(shape);
}

Obstacle::Obstacle(std::string uniqueName, Vector3 boxDimensions, std::string material, std::string look, unsigned int uvMode) : StaticEntity(uniqueName, material, look)
{
    Vector3 halfExtents = boxDimensions/Scalar(2);
    glm::vec3 glHalfExtents(halfExtents.x(), halfExtents.y(), halfExtents.z());
	phyMesh = OpenGLContent::BuildBox(glHalfExtents, 0, uvMode);
    graMesh = phyMesh;
	graObjectId = -1;
    
    btBoxShape* shape = new btBoxShape(halfExtents);
    BuildRigidBody(shape);
}

Obstacle::Obstacle(std::string uniqueName, Scalar cylinderRadius, Scalar cylinderHeight, std::string material, std::string look) : StaticEntity(uniqueName, material, look)
{
    Scalar halfHeight = cylinderHeight/Scalar(2);
    phyMesh = OpenGLContent::BuildCylinder((GLfloat)cylinderRadius, (GLfloat)halfHeight*2.f);
    graMesh = phyMesh;
    graObjectId = -1;
    
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
        
    graObjectId = ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->BuildObject(graMesh);
    phyObjectId = ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->BuildObject(phyMesh);
}

std::vector<Renderable> Obstacle::Render()
{
    std::vector<Renderable> items(0);
	
    if(rigidBody != NULL && isRenderable())
    {
        Renderable item;
        
        if(dm == DisplayMode::DISPLAY_GRAPHICAL && graObjectId >= 0)
        { 
            item.type = RenderableType::SOLID;
            item.objectId = graObjectId;
            item.lookId = lookId;
            item.model = glMatrixFromTransform(getTransform());
            items.push_back(item);
        }
        else if(dm == DisplayMode::DISPLAY_PHYSICAL && phyObjectId >= 0)
        {
            item.type = RenderableType::SOLID;
            item.objectId = phyObjectId;
            item.lookId = -1;
            item.model = glMatrixFromTransform(getTransform());
            items.push_back(item);
        }
    }
	
	return items;
}

}
