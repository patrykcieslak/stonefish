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
//  Copyright (c) 2020-2026 Patryk Cieslak. All rights reserved.
//

#include "entities/AnimatedEntity.h"

#include "core/GraphicalSimulationApp.h"
#include "core/SimulationManager.h"
#include "graphics/OpenGLPipeline.h"
#include "graphics/OpenGLContent.h"

namespace sf
{

AnimatedEntity::AnimatedEntity(const std::string& uniqueName, std::unique_ptr<Trajectory> traj) : MovingEntity(uniqueName, "", ""), traj_(std::move(traj))
{
    if(traj_ == nullptr)
        throw std::invalid_argument("Trajectory pointer cannot be null");

    T_CG2O_ = T_O2C_ = T_O2G_ = I4();
    
    linearAcc_.setZero();
    angularAcc_.setZero();

    //Build rigid body
    collisionShape_ = std::make_unique<btEmptyShape>();
    BuildRigidBody(false);
    phyObjectId_ = graObjectId_ = -1;
}

AnimatedEntity::AnimatedEntity(const std::string& uniqueName, std::unique_ptr<Trajectory> traj, Scalar sphereRadius, const Transform& origin, const std::string& material, const std::string& look, bool collides) 
    : MovingEntity(uniqueName, material, look), traj_(std::move(traj))
{   
    if(traj_ == nullptr)
        throw std::invalid_argument("Trajectory pointer cannot be null");

    T_O2C_ = T_O2G_ = I4();
    T_CG2O_ = origin.inverse();

    //Build rigid body
    collisionShape_ = std::make_unique<btSphereShape>(sphereRadius);
    BuildRigidBody(collides);

    //Build graphical objects
    if(SimulationApp::getApp()->hasGraphics())
    { 
        std::unique_ptr<Mesh> phyMesh = OpenGLContent::BuildSphere((GLfloat)sphereRadius);
        phyObjectId_ = ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->BuildObject(phyMesh.get());
        graObjectId_ = phyObjectId_;
    }
}

AnimatedEntity::AnimatedEntity(const std::string& uniqueName, std::unique_ptr<Trajectory> traj, Scalar cylinderRadius, Scalar cylinderHeight, const Transform& origin, const std::string& material, const std::string& look, bool collides)
    : MovingEntity(uniqueName, material, look), traj_(std::move(traj))
{
    if(traj_ == nullptr)
        throw std::invalid_argument("Trajectory pointer cannot be null");

    T_O2C_ = T_O2G_ = I4();
    T_CG2O_ = origin.inverse();

    //Build rigid body
    Scalar halfHeight = cylinderHeight/Scalar(2);
    collisionShape_ = std::make_unique<btCylinderShapeZ>(Vector3(cylinderRadius, cylinderRadius, halfHeight));
    BuildRigidBody(collides);

    //Build graphical objects
    if(SimulationApp::getApp()->hasGraphics())
    { 
        std::unique_ptr<Mesh> phyMesh = OpenGLContent::BuildCylinder((GLfloat)cylinderRadius, (GLfloat)cylinderHeight);
        phyObjectId_ = ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->BuildObject(phyMesh.get());
        graObjectId_ = phyObjectId_;
    }
}

AnimatedEntity::AnimatedEntity(const std::string& uniqueName, std::unique_ptr<Trajectory> traj, Vector3 boxDimensions, const Transform& origin, const std::string& material, const std::string& look, bool collides) 
    : MovingEntity(uniqueName, material, look), traj_(std::move(traj))
{
    if(traj_ == nullptr)
        throw std::invalid_argument("Trajectory pointer cannot be null");

    T_O2C_ = T_O2G_ = I4();
    T_CG2O_ = origin.inverse();

    //Build rigid body
    collisionShape_ = std::make_unique<btBoxShape>(boxDimensions/Scalar(2));
    BuildRigidBody(collides);

    //Build graphical objects
    if(SimulationApp::getApp()->hasGraphics())
    { 
        std::unique_ptr<Mesh> phyMesh = OpenGLContent::BuildBox(glm::vec3((GLfloat)boxDimensions.getX()/2.f, (GLfloat)boxDimensions.getY()/2.f, (GLfloat)boxDimensions.getZ()/2.f));
        phyObjectId_ = ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->BuildObject(phyMesh.get());
        graObjectId_ = phyObjectId_;
    }
}

AnimatedEntity::AnimatedEntity(const std::string& uniqueName, std::unique_ptr<Trajectory> traj, const std::string& modelFilename, Scalar scale, const Transform& origin, const std::string& material, const std::string& look, bool collides)
    : AnimatedEntity(uniqueName, std::move(traj), modelFilename, scale, origin, "", Scalar(1), I4(), material, look, collides)
{
}

AnimatedEntity::AnimatedEntity(const std::string& uniqueName, std::unique_ptr<Trajectory> traj, const std::string& graphicsFilename, Scalar graphicsScale, const Transform& graphicsOrigin,
                       const std::string& physicsFilename, Scalar physicsScale, const Transform& physicsOrigin, const std::string& material, const std::string& look, bool collides)
    : MovingEntity(uniqueName, material, look), traj_(std::move(traj))
{
    if(traj_ == nullptr)
        throw std::invalid_argument("Trajectory pointer cannot be null");

    //Load geometry from files
    std::shared_ptr<Mesh> graMesh = OpenGLContent::LoadMesh(graphicsFilename, graphicsScale, false);
    std::shared_ptr<Mesh> phyMesh;
    T_O2G_ = graphicsOrigin;
    T_CG2O_ = I4();
    
    if(physicsFilename != "")
    {
        phyMesh = OpenGLContent::LoadMesh(physicsFilename, physicsScale, false);
        T_O2C_ = physicsOrigin;
    }
    else
    {
        phyMesh = graMesh;
        T_O2C_ = T_O2G_;
    }

    //Build rigid body
    std::unique_ptr<btConvexHullShape> shape = std::make_unique<btConvexHullShape>();
    for(size_t i=0; i<phyMesh->getNumOfVertices(); ++i)
    {
        glm::vec3 pos = phyMesh->getVertexPos(i);
        Vector3 v(pos.x, pos.y, pos.z);
        shape->addPoint(v);
    }
    shape->optimizeConvexHull();
    collisionShape_ = std::move(shape);
    BuildRigidBody(collides);

    //Build graphical objects
    if(SimulationApp::getApp()->hasGraphics())
    { 
        phyObjectId_ = ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->BuildObject(phyMesh.get());
        if(graMesh != phyMesh)
            graObjectId_ = ((GraphicalSimulationApp*)SimulationApp::getApp())->getGLPipeline()->getContent()->BuildObject(graMesh.get());
        else
            graObjectId_ = phyObjectId_;
    }
}

EntityType AnimatedEntity::getType() const
{
    return EntityType::ANIMATED;
}

Transform AnimatedEntity::getOTransform() const
{
    return getCGTransform() * T_CG2O_;
}

Transform AnimatedEntity::getCGTransform() const
{
    if(rigidBody_ != nullptr)
    {
        Transform trans;
        rigidBody_->getMotionState()->getWorldTransform(trans);
        return trans;
    }
    else
        return Transform::getIdentity();
}

Vector3 AnimatedEntity::getLinearVelocity() const
{
    if(rigidBody_ != nullptr)
        return rigidBody_->getLinearVelocity();
    else
        return V0();
}

Vector3 AnimatedEntity::getAngularVelocity() const
{
    if(rigidBody_ != nullptr)
        return rigidBody_->getAngularVelocity();
    else
        return V0();
}

Vector3 AnimatedEntity::getLinearVelocityInLocalPoint(const Vector3& relPos) const
{
    return getLinearVelocity() + getAngularVelocity().cross(relPos);
}

Vector3 AnimatedEntity::getLinearAcceleration() const
{
    return linearAcc_;
}
        
Vector3 AnimatedEntity::getAngularAcceleration() const
{
    return angularAcc_;
}

Trajectory* AnimatedEntity::getTrajectory()
{
    return traj_.get();
}

void AnimatedEntity::getAABB(Vector3& min, Vector3& max)
{
    if(rigidBody_ != nullptr)
        rigidBody_->getAabb(min, max);
    else
    {
        min.setValue(BT_LARGE_FLOAT, BT_LARGE_FLOAT, BT_LARGE_FLOAT);
        max.setValue(-BT_LARGE_FLOAT, -BT_LARGE_FLOAT, -BT_LARGE_FLOAT);
    }
}

void AnimatedEntity::BuildRigidBody(bool collides)
{
    motionState_ = std::make_unique<btDefaultMotionState>(traj_->getInterpolatedTransform());
    collisionShape_->setMargin(0.0);
    
    btRigidBody::btRigidBodyConstructionInfo rigidBodyCI(Scalar(0), motionState_.get(), collisionShape_.get(), V0());
    rigidBodyCI.m_friction = rigidBodyCI.m_rollingFriction = rigidBodyCI.m_restitution = Scalar(0); //not used
    rigidBodyCI.m_linearDamping = rigidBodyCI.m_angularDamping = Scalar(0); //not used
    rigidBodyCI.m_linearSleepingThreshold = rigidBodyCI.m_angularSleepingThreshold = Scalar(0); //not used
    rigidBodyCI.m_additionalDamping = false;
    
    rigidBody_ = std::make_unique<btRigidBody>(rigidBodyCI);
    rigidBody_->setUserPointer(this);
    rigidBody_->setCollisionFlags(rigidBody_->getCollisionFlags() 
                                     | btCollisionObject::CF_KINEMATIC_OBJECT 
                                     | btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);
    if(!collides) rigidBody_->setCollisionFlags(rigidBody_->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);
    rigidBody_->setActivationState(DISABLE_DEACTIVATION);
}

void AnimatedEntity::AddToSimulation(SimulationManager* sm)
{
    if(rigidBody_ != nullptr)
    {
        if(rigidBody_->getCollisionFlags() & btCollisionObject::CF_NO_CONTACT_RESPONSE) //No collisions
            sm->getDynamicsWorld()->addRigidBody(rigidBody_.get(), MASK_ANIMATED_NONCOLLIDING, MASK_DYNAMIC);
        else
            sm->getDynamicsWorld()->addRigidBody(rigidBody_.get(), MASK_ANIMATED_COLLIDING, MASK_DYNAMIC); //Only collide with dynamic bodies
    }
}
        
void AnimatedEntity::AddToSimulation(SimulationManager* sm, const Transform& origin)
{
    AddToSimulation(sm);
}

void AnimatedEntity::Update(Scalar dt)
{
    if(traj_ == nullptr || rigidBody_ == nullptr)
        return;

    traj_->Play(dt);
    rigidBody_->getMotionState()->setWorldTransform(traj_->getInterpolatedTransform() *  T_CG2O_.inverse());
    rigidBody_->setLinearVelocity(traj_->getInterpolatedLinearVelocity());
    rigidBody_->setAngularVelocity(traj_->getInterpolatedAngularVelocity());    
    setLinearAcceleration(traj_->getInterpolatedLinearAcceleration());
}

std::vector<Renderable> AnimatedEntity::Render()
{
    std::vector<Renderable> items(0);
    
    if(rigidBody_ != nullptr && isRenderable())
    {
        Renderable item;
        item.type = RenderableType::SOLID_CS;
        item.model = glMatrixFromTransform(getOTransform());
        items.push_back(item);

        if(graObjectId_ >= 0)
        {
            item.type = RenderableType::SOLID;
            item.cor = glVectorFromVector(getOTransform().getOrigin());
            item.vel = glVectorFromVector(getLinearVelocity());
            item.avel = glVectorFromVector(getAngularVelocity());
            item.materialName = mat_.name;
            item.objectId = dm_ == DisplayMode::GRAPHICAL ? graObjectId_ : phyObjectId_;
            item.lookId = dm_ == DisplayMode::GRAPHICAL ? lookId_ : -1;
            items.push_back(item);
        }
        
        std::vector<Renderable> trajectoryItems = traj_->Render();
        items.insert(items.begin(), trajectoryItems.begin(), trajectoryItems.end());
    }

    return items;
}

}
