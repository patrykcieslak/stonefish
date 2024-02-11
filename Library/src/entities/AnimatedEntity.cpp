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
//  AnimatedEntity.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 15/07/2020.
//  Copyright (c) 2020 Patryk Cieslak. All rights reserved.
//

#include "entities/AnimatedEntity.h"

#include "core/GraphicalSimulationApp.h"
#include "core/SimulationManager.h"
#include "graphics/OpenGLPipeline.h"
#include "graphics/OpenGLContent.h"

namespace sf
{

AnimatedEntity::AnimatedEntity(std::string uniqueName, Trajectory* traj) : MovingEntity(uniqueName, "", ""), tr(traj)
{
    if(traj == nullptr)
        return;        

    T_CG2O = T_O2C = T_O2G = I4();

    //Build rigid body
    btEmptyShape* shape = new btEmptyShape();
    BuildRigidBody(shape, false);
    phyObjectId = graObjectId = -1;
}

AnimatedEntity::AnimatedEntity(std::string uniqueName, Trajectory* traj, Scalar sphereRadius, const Transform& origin, std::string material, std::string look, bool collides) 
    : MovingEntity(uniqueName, material, look), tr(traj)
{   
    if(traj == nullptr)
        return;

    T_O2C = T_O2G = I4();
    T_CG2O = origin.inverse();

    //Build rigid body
    btSphereShape* shape = new btSphereShape(sphereRadius);
    BuildRigidBody(shape, collides);

    //Build graphical objects
    if(SimulationApp::getApp()->hasGraphics())
    { 
        Mesh* phyMesh = OpenGLContent::BuildSphere((GLfloat)sphereRadius);
        phyObjectId = ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->BuildObject(phyMesh);
        graObjectId = phyObjectId;
    }
}

AnimatedEntity::AnimatedEntity(std::string uniqueName, Trajectory* traj, Scalar cylinderRadius, Scalar cylinderHeight, const Transform& origin, std::string material, std::string look, bool collides)
    : MovingEntity(uniqueName, material, look), tr(traj)
{
    if(traj == nullptr)
        return;

    T_O2C = T_O2G = I4();
    T_CG2O = origin.inverse();

    //Build rigid body
    Scalar halfHeight = cylinderHeight/Scalar(2);
    btCylinderShape* shape = new btCylinderShapeZ(Vector3(cylinderRadius, cylinderRadius, halfHeight));
    BuildRigidBody(shape, collides);

    //Build graphical objects
    if(SimulationApp::getApp()->hasGraphics())
    { 
        Mesh* phyMesh = OpenGLContent::BuildCylinder((GLfloat)cylinderRadius, (GLfloat)cylinderHeight);
        phyObjectId = ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->BuildObject(phyMesh);
        graObjectId = phyObjectId;
    }
}

AnimatedEntity::AnimatedEntity(std::string uniqueName, Trajectory* traj, Vector3 boxDimensions, const Transform& origin, std::string material, std::string look, bool collides) 
    : MovingEntity(uniqueName, material, look), tr(traj)
{
    if(traj == nullptr)
        return;

    T_O2C = T_O2G = I4();
    T_CG2O = origin.inverse();

    //Build rigid body
    btBoxShape* shape = new btBoxShape(boxDimensions/Scalar(2));
    BuildRigidBody(shape, collides);

    //Build graphical objects
    if(SimulationApp::getApp()->hasGraphics())
    { 
        Mesh* phyMesh = OpenGLContent::BuildBox(glm::vec3((GLfloat)boxDimensions.getX()/2.f, (GLfloat)boxDimensions.getY()/2.f, (GLfloat)boxDimensions.getZ()/2.f));
        phyObjectId = ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->BuildObject(phyMesh);
        graObjectId = phyObjectId;
    }
}

AnimatedEntity::AnimatedEntity(std::string uniqueName, Trajectory* traj, std::string modelFilename, Scalar scale, const Transform& origin, std::string material, std::string look, bool collides)
    : AnimatedEntity(uniqueName, traj, modelFilename, scale, origin, "", Scalar(1), I4(), material, look, collides)
{
}

AnimatedEntity::AnimatedEntity(std::string uniqueName, Trajectory* traj, std::string graphicsFilename, Scalar graphicsScale, const Transform& graphicsOrigin,
                       std::string physicsFilename, Scalar physicsScale, const Transform& physicsOrigin, std::string material, std::string look, bool collides)
    : MovingEntity(uniqueName, material, look), tr(traj)
{
    if(traj == nullptr)
        return;

    //Load geometry from files
    Mesh* graMesh = OpenGLContent::LoadMesh(graphicsFilename, graphicsScale, false);
    Mesh* phyMesh;
    T_O2G = graphicsOrigin;
    T_CG2O = I4();
    
    if(physicsFilename != "")
    {
        phyMesh = OpenGLContent::LoadMesh(physicsFilename, physicsScale, false);
        T_O2C = physicsOrigin;
    }
    else
    {
        phyMesh = graMesh;
        T_O2C = T_O2G;
    }

    //Build rigid body
    btConvexHullShape* shape = new btConvexHullShape();
    for(size_t i=0; i<phyMesh->getNumOfVertices(); ++i)
    {
        glm::vec3 pos = phyMesh->getVertexPos(i);
        Vector3 v(pos.x, pos.y, pos.z);
        shape->addPoint(v);
    }
    shape->optimizeConvexHull();
    BuildRigidBody(shape, collides);

    //Build graphical objects
    if(SimulationApp::getApp()->hasGraphics())
    { 
        phyObjectId = ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->BuildObject(phyMesh);
        if(graMesh != phyMesh)
            graObjectId = ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->BuildObject(graMesh);
        else
            graObjectId = phyObjectId;
    }

    //Delete mesh data
    if(graMesh != phyMesh)
    {
        delete phyMesh;
        delete graMesh;
    }
    else
        delete phyMesh;
}

AnimatedEntity::~AnimatedEntity()
{
    if(tr != nullptr)
        delete tr;
}

EntityType AnimatedEntity::getType() const
{
    return EntityType::ANIMATED;
}

Transform AnimatedEntity::getOTransform() const
{
    return getCGTransform() * T_CG2O;
}

Transform AnimatedEntity::getCGTransform() const
{
    if(rigidBody != nullptr)
    {
        Transform trans;
        rigidBody->getMotionState()->getWorldTransform(trans);
        return trans;
    }
    else
        return Transform::getIdentity();
}

Vector3 AnimatedEntity::getLinearVelocity() const
{
    if(rigidBody != nullptr)
        return rigidBody->getLinearVelocity();
    else
        return V0();
}

Vector3 AnimatedEntity::getAngularVelocity() const
{
    if(rigidBody != nullptr)
        return rigidBody->getAngularVelocity();
    else
        return V0();
}

Vector3 AnimatedEntity::getLinearVelocityInLocalPoint(const Vector3& relPos) const
{
    return getLinearVelocity() + getAngularVelocity().cross(relPos);
}

Vector3 AnimatedEntity::getLinearAcceleration() const
{
    return V0();
}
        
Vector3 AnimatedEntity::getAngularAcceleration() const
{
    return V0();
}

Trajectory* AnimatedEntity::getTrajectory()
{
    return tr;
}

void AnimatedEntity::getAABB(Vector3& min, Vector3& max)
{
    if(rigidBody != nullptr)
        rigidBody->getAabb(min, max);
    else
    {
        min.setValue(BT_LARGE_FLOAT, BT_LARGE_FLOAT, BT_LARGE_FLOAT);
        max.setValue(-BT_LARGE_FLOAT, -BT_LARGE_FLOAT, -BT_LARGE_FLOAT);
    }
}

void AnimatedEntity::BuildRigidBody(btCollisionShape* shape, bool collides)
{
    btDefaultMotionState* motionState = new btDefaultMotionState(tr->getInterpolatedTransform());
    shape->setMargin(0.0);
    
    btRigidBody::btRigidBodyConstructionInfo rigidBodyCI(Scalar(0), motionState, shape, V0());
    rigidBodyCI.m_friction = rigidBodyCI.m_rollingFriction = rigidBodyCI.m_restitution = Scalar(0); //not used
    rigidBodyCI.m_linearDamping = rigidBodyCI.m_angularDamping = Scalar(0); //not used
    rigidBodyCI.m_linearSleepingThreshold = rigidBodyCI.m_angularSleepingThreshold = Scalar(0); //not used
    rigidBodyCI.m_additionalDamping = false;
    
    rigidBody = new btRigidBody(rigidBodyCI);
    rigidBody->setUserPointer(this);
    rigidBody->setCollisionFlags(rigidBody->getCollisionFlags() 
                                     | btCollisionObject::CF_KINEMATIC_OBJECT 
                                     | btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);
    if(!collides) rigidBody->setCollisionFlags(rigidBody->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);
    rigidBody->setActivationState(DISABLE_DEACTIVATION);
}

void AnimatedEntity::AddToSimulation(SimulationManager* sm)
{
    if(rigidBody != nullptr)
    {
        if(rigidBody->getCollisionFlags() & btCollisionObject::CF_NO_CONTACT_RESPONSE) //No collisions
            sm->getDynamicsWorld()->addRigidBody(rigidBody, MASK_ANIMATED_NONCOLLIDING, MASK_DYNAMIC);
        else
            sm->getDynamicsWorld()->addRigidBody(rigidBody, MASK_ANIMATED_COLLIDING, MASK_DYNAMIC); //Only collide with dynamic bodies
    }
    
}
        
void AnimatedEntity::AddToSimulation(SimulationManager* sm, const Transform& origin)
{
    AddToSimulation(sm);
}

void AnimatedEntity::Update(Scalar dt)
{
    if(tr == nullptr || rigidBody == nullptr)
        return;

    tr->Play(dt);
    rigidBody->getMotionState()->setWorldTransform(tr->getInterpolatedTransform() *  T_CG2O.inverse());
    rigidBody->setLinearVelocity(tr->getInterpolatedLinearVelocity());
    rigidBody->setAngularVelocity(tr->getInterpolatedAngularVelocity());    
}

std::vector<Renderable> AnimatedEntity::Render()
{
    std::vector<Renderable> items(0);
    
    if(rigidBody != nullptr && isRenderable())
    {
        Renderable item;
        item.type = RenderableType::SOLID_CS;
        item.model = glMatrixFromTransform(getOTransform());
        items.push_back(item);

        if(graObjectId >= 0)
        {
            item.type = RenderableType::SOLID;
            item.materialName = mat.name;
            item.objectId = dm == DisplayMode::GRAPHICAL ? graObjectId : phyObjectId;
            item.lookId = dm == DisplayMode::GRAPHICAL ? lookId : -1;
            items.push_back(item);
        }

        items.push_back(tr->Render());
    }

    return items;
}

}
