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
//  Copyright (c) 2014-2026 Patryk Cieslak. All rights reserved.
//

#include "entities/statics/Obstacle.h"

#include "core/GraphicalSimulationApp.h"
#include "graphics/OpenGLPipeline.h"
#include "graphics/OpenGLContent.h"

namespace sf
{

Obstacle::Obstacle(const std::string& uniqueName,
         const std::string& graphicsFilename, Scalar graphicsScale, const Transform& graphicsOrigin,
         const std::string& physicsFilename, Scalar physicsScale, const Transform& physicsOrigin, bool convexHull,
         const std::string& material, const std::string& look) : StaticEntity(uniqueName, material, look)
{
    graMesh_ = OpenGLContent::LoadMesh(graphicsFilename, graphicsScale, false);
    OpenGLContent::TransformMesh(graMesh_.get(), graphicsOrigin);
    
    if(physicsFilename != "")
    {
        phyMesh_ = OpenGLContent::LoadMesh(physicsFilename, physicsScale, false);
        OpenGLContent::TransformMesh(phyMesh_.get(), physicsOrigin);
    }
    else
        phyMesh_ = graMesh_;
        
    graObjectId_ = -1;

    //Buidling collision shape
    if(convexHull) // Convex approximation
    {
        std::unique_ptr<btConvexHullShape> shape = std::make_unique<btConvexHullShape>();
        for(size_t i=0; i<phyMesh_->getNumOfVertices(); ++i)
        {
            glm::vec3 pos = phyMesh_->getVertexPos(i);
            Vector3 v(pos.x, pos.y, pos.z);
            shape->addPoint(v);
        }
        //shape->optimizeConvexHull();
        shape->setMargin(0);
        collisionShape_ = std::move(shape);
    }
    else // Non-convex (arbitrary triangle mesh)
    {
        Scalar* vertices = new Scalar[phyMesh_->getNumOfVertices() * 3];
        int* indices = new int[phyMesh_->faces.size()*3];
        
        for(size_t i=0; i<phyMesh_->getNumOfVertices(); ++i)
        {
            glm::vec3 pos = phyMesh_->getVertexPos(i);
            vertices[i*3+0] = pos.x;
            vertices[i*3+1] = pos.y;
            vertices[i*3+2] = pos.z;
        }
        
        for(size_t i=0; i<phyMesh_->faces.size(); ++i)
        {
            indices[i*3+0] = phyMesh_->faces[i].vertexID[0];
            indices[i*3+1] = phyMesh_->faces[i].vertexID[1];
            indices[i*3+2] = phyMesh_->faces[i].vertexID[2];
        }
        
        btTriangleIndexVertexArray* triangleArray = new btTriangleIndexVertexArray((int)phyMesh_->faces.size(), indices, 3*sizeof(int),
                                                                                (int)phyMesh_->getNumOfVertices(), vertices, 3*sizeof(Scalar));
        std::unique_ptr<btBvhTriangleMeshShape> shape = std::make_unique<btBvhTriangleMeshShape>(triangleArray, true);
        shape->setMargin(0);
        collisionShape_ = std::move(shape);
    }
    BuildRigidBody();
}
    
Obstacle::Obstacle(const std::string& uniqueName, const std::string& modelFilename, Scalar scale, const Transform& origin, bool convexHull, const std::string& material, const std::string& look)
    : Obstacle(uniqueName, modelFilename, scale, origin, "", scale, origin, convexHull, material, look)
{
}

Obstacle::Obstacle(const std::string& uniqueName, Scalar sphereRadius, const Transform& origin, const std::string& material, const std::string& look) : StaticEntity(uniqueName, material, look)
{
    phyMesh_ = OpenGLContent::BuildSphere(sphereRadius);
    graMesh_ = phyMesh_;
    graObjectId_ = -1;
    
    btSphereShape* shape = new btSphereShape(sphereRadius);
    if(origin == I4())
    {
        collisionShape_.reset(shape);
        BuildRigidBody();
    }
    else
    {
        OpenGLContent::TransformMesh(phyMesh_.get(), origin);
        std::unique_ptr<btCompoundShape> cShape = std::make_unique<btCompoundShape>();
        cShape->addChildShape(origin, shape);
        collisionShape_ = std::move(cShape);
        BuildRigidBody();
    }
}

Obstacle::Obstacle(const std::string& uniqueName, Vector3 boxDimensions, const Transform& origin, const std::string& material, const std::string& look, unsigned int uvMode) : StaticEntity(uniqueName, material, look)
{
    Vector3 halfExtents = boxDimensions/Scalar(2);
    glm::vec3 glHalfExtents(halfExtents.x(), halfExtents.y(), halfExtents.z());
	phyMesh_ = OpenGLContent::BuildBox(glHalfExtents, 0, uvMode);
    graMesh_ = phyMesh_;
	graObjectId_ = -1;
    
    btBoxShape* shape = new btBoxShape(halfExtents);
    shape->setMargin(COLLISION_MARGIN);
    if(origin == I4())
    {
        collisionShape_.reset(shape);
        BuildRigidBody();
    }
    else
    {
        OpenGLContent::TransformMesh(phyMesh_.get(), origin);
        std::unique_ptr<btCompoundShape> cShape = std::make_unique<btCompoundShape>();
        cShape->addChildShape(origin, shape);
        collisionShape_ = std::move(cShape);
        BuildRigidBody();
    }
}

Obstacle::Obstacle(const std::string& uniqueName, Scalar cylinderRadius, Scalar cylinderHeight, const Transform& origin, const std::string& material, const std::string& look) : StaticEntity(uniqueName, material, look)
{
    Scalar halfHeight = cylinderHeight/Scalar(2);
    phyMesh_ = OpenGLContent::BuildCylinder((GLfloat)cylinderRadius, (GLfloat)cylinderHeight, (unsigned int)btMax(ceil(2.0*M_PI*cylinderRadius/0.1), 32.0)); //Max 0.1 m cylinder wall slice width
    graMesh_ = phyMesh_;
    graObjectId_ = -1;
    
    btCylinderShape* shape = new btCylinderShapeZ(Vector3(cylinderRadius, cylinderRadius, halfHeight));
    shape->setMargin(COLLISION_MARGIN);
    if(origin == I4())
    {
        collisionShape_.reset(shape);    
        BuildRigidBody();
    }
    else
    {
        OpenGLContent::TransformMesh(phyMesh_.get(), origin);
        std::unique_ptr<btCompoundShape> cShape = std::make_unique<btCompoundShape>();
        cShape->addChildShape(origin, shape);
        collisionShape_ = std::move(cShape);
        BuildRigidBody();
    }
}

Obstacle::~Obstacle()
{
    btBvhTriangleMeshShape* tms;
    if ((tms = dynamic_cast<btBvhTriangleMeshShape*>(collisionShape_.get())) != nullptr)
    {
        btTriangleIndexVertexArray* tva = static_cast<btTriangleIndexVertexArray*>(tms->getMeshInterface());
        delete [] tva->getIndexedMeshArray()[0].m_vertexBase;
        delete [] tva->getIndexedMeshArray()[0].m_triangleIndexBase;
        delete tva;
    }
    btCompoundShape* cs;
    if ((cs = dynamic_cast<btCompoundShape*>(collisionShape_.get())) != nullptr)
    {
        for (int i = 0; i < cs->getNumChildShapes(); ++i)
            delete cs->getChildShape(i);
    }
}

StaticEntityType Obstacle::getStaticType()
{
    return StaticEntityType::OBSTACLE;
}
    
void Obstacle::BuildGraphicalObject()
{
    if(graMesh_ == nullptr || !SimulationApp::getApp()->hasGraphics())
        return;
        
    graObjectId_ = static_cast<GraphicalSimulationApp*>(SimulationApp::getApp())->getGLPipeline()->getContent()->BuildObject(graMesh_.get());
    phyObjectId_ = static_cast<GraphicalSimulationApp*>(SimulationApp::getApp())->getGLPipeline()->getContent()->BuildObject(phyMesh_.get());
}

std::vector<Renderable> Obstacle::Render()
{
    std::vector<Renderable> items(0);
	
    if(rigidBody_ != nullptr && isRenderable())
    {
        Renderable item;
        item.type = RenderableType::SOLID;
        item.materialName = mat_.name;
        
        if(dm_ == DisplayMode::GRAPHICAL && graObjectId_ >= 0)
        { 
            item.objectId = graObjectId_;
            item.lookId = lookId_;
            item.model = glMatrixFromTransform(getTransform());
            items.push_back(item);
        }
        else if(dm_ == DisplayMode::PHYSICAL && phyObjectId_ >= 0)
        {
            item.objectId = phyObjectId_;
            item.lookId = -1;
            item.model = glMatrixFromTransform(getTransform());
            items.push_back(item);
        }
    }
	
	return items;
}

}
