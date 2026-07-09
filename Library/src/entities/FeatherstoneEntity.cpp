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
//  FeatherstoneEntity.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 8/20/13.
//  Copyright (c) 2013-2026 Patryk Cieslak. All rights reserved.
//

#include "entities/FeatherstoneEntity.h"

#include "core/SimulationApp.h"
#include "core/SimulationManager.h"
#include "entities/StaticEntity.h"

namespace sf
{

FeatherstoneEntity::FeatherstoneEntity(const std::string& uniqueName, size_t totalNumOfLinks, std::unique_ptr<SolidEntity> baseSolid, bool fixedBase) : Entity(uniqueName)
{
    Scalar M = baseSolid->getAugmentedMass();
    Vector3 I = baseSolid->getAugmentedInertia();
    multiBody_ = std::make_unique<btMultiBody>(totalNumOfLinks - 1, M, I, fixedBase, true);
    multiBody_->setBaseWorldTransform(Transform::getIdentity());
    multiBody_->setAngularDamping(Scalar(0));
    multiBody_->setLinearDamping(Scalar(0));
    multiBody_->setMaxAppliedImpulse(BT_LARGE_FLOAT);
    multiBody_->setMaxCoordinateVelocity(Scalar(1000));
    multiBody_->useRK4Integration(false); //Enabling RK4 causes unreallistic energy accumulation (strange motions in 0 gravity)
    multiBody_->useGlobalVelocities(false); //See previous comment
    multiBody_->setHasSelfCollision(false); //No self collision by default
    multiBody_->setUseGyroTerm(true);
    multiBody_->setCanSleep(true);
   
    AddLink(std::move(baseSolid), Transform::getIdentity());
    
    baseRenderable_ = true;
}

EntityType FeatherstoneEntity::getType() const
{
    return EntityType::FEATHERSTONE;
}

void FeatherstoneEntity::getAABB(Vector3& min, Vector3& max)
{
    //Initialize AABB
    min = Vector3(BT_LARGE_FLOAT, BT_LARGE_FLOAT, BT_LARGE_FLOAT);
    max = Vector3(-BT_LARGE_FLOAT, -BT_LARGE_FLOAT, -BT_LARGE_FLOAT);
    
    for(size_t i = 0; i < links_.size(); i++)
    {
        //Get link AABB
        Vector3 lmin;
        Vector3 lmax;
        links_[i].solid->multibodyCollider_->getCollisionShape()->getAabb(getLinkTransform(i), lmin, lmax);
        
        //Merge with other AABBs
        min[0] = std::min(min[0], lmin[0]);
        min[1] = std::min(min[1], lmin[1]);
        min[2] = std::min(min[2], lmin[2]);
        
        max[0] = std::max(max[0], lmax[0]);
        max[1] = std::max(max[1], lmax[1]);
        max[2] = std::max(max[2], lmax[2]);
    }
}

void FeatherstoneEntity::AddToSimulation(SimulationManager* sm)
{
    AddToSimulation(sm, Transform::getIdentity());
}

void FeatherstoneEntity::AddToSimulation(SimulationManager* sm, const Transform& origin)
{
    //Creating joint limit constraints
    for(size_t i=0; i<joints_.size(); ++i)
    {    
        if(joints_[i].limit != nullptr)
            sm->getDynamicsWorld()->addMultiBodyConstraint(joints_[i].limit);
    }
    
    //Creating motors (has to be after joint limits and not interleaved!)
    for(size_t i=0; i<joints_.size(); ++i)
    {
        if(joints_[i].motor != nullptr)
            sm->getDynamicsWorld()->addMultiBodyConstraint(joints_[i].motor);
    }
    
    //Resize matrices
    multiBody_->finalizeMultiDof();

    //Apply solver settings
    multiBody_->setAngularDamping(sm->getDynamicsWorld()->getSolverInfo().m_damping);
    multiBody_->setLinearDamping(sm->getDynamicsWorld()->getSolverInfo().m_damping);
    
    //Add multibody to the world
    Respawn(origin);
    sm->getDynamicsWorld()->addMultiBody(multiBody_.get());
}

void FeatherstoneEntity::RemoveFromSimulation(SimulationManager* sm)
{
    sm->getDynamicsWorld()->removeMultiBody(multiBody_.get());
}

void FeatherstoneEntity::Respawn(const Transform& origin)
{
    //Set origin position
    setBaseTransform(origin);
    multiBody_->setBaseVel(Vector3(0,0,0));
    multiBody_->setBaseOmega(Vector3(0,0,0));
    
    //Move joints to the limits
    for(size_t i=0; i<joints_.size(); ++i)
    {
        if(joints_[i].limit != nullptr)
        {
            if(joints_[i].lowerLimit > Scalar(0))
                multiBody_->setJointPos((int)i, joints_[i].lowerLimit);
            else if(joints_[i].upperLimit < Scalar(0))
                multiBody_->setJointPos((int)i, joints_[i].upperLimit);
        }
    }
    
    //Calculate constrained link positions to avoid jump at the start of simulation
    btAlignedObjectArray<Quaternion> scratchQ;
    btAlignedObjectArray<Vector3> scratchM;
    multiBody_->forwardKinematics(scratchQ, scratchM);
    multiBody_->updateCollisionObjectWorldTransforms(scratchQ, scratchM);
}

void FeatherstoneEntity::setSelfCollision(bool enabled)
{
    multiBody_->setHasSelfCollision(enabled);
}

bool FeatherstoneEntity::hasSelfCollision() const
{
    return multiBody_->hasSelfCollision();
}

void FeatherstoneEntity::setDisplayMode(DisplayMode m)
{
    for(size_t i=0; i<links_.size(); ++i)
        links_[i].solid->setDisplayMode(m);
}

void FeatherstoneEntity::setBaseRenderable(bool render)
{
    baseRenderable_ = render;
}

void FeatherstoneEntity::setBaseTransform(const Transform& trans)
{
    Transform T0 = trans * links_[0].solid->getCG2CTransform().inverse();
    multiBody_->getBaseCollider()->setWorldTransform(T0);
    multiBody_->setBaseWorldTransform(T0);
    
    for(size_t i=1; i<links_.size(); ++i)
    {
        Transform tr = links_[i].solid->multibodyCollider_->getWorldTransform();
        links_[i].solid->multibodyCollider_->setWorldTransform(trans * tr);
    }
}

void FeatherstoneEntity::setJointIC(size_t index, Scalar position, Scalar velocity)
{
    if(index >= joints_.size())
        return;
    
    switch (joints_[index].type)
    {
        case btMultibodyLink::eRevolute:
            multiBody_->setJointPos(joints_[index].child - 1, position);
            multiBody_->setJointVel(joints_[index].child - 1, velocity);
            break;
            
        case btMultibodyLink::ePrismatic:
            multiBody_->setJointPos(joints_[index].child - 1, position);
            multiBody_->setJointVel(joints_[index].child - 1, velocity);
            break;
            
        default:
            break;
    }
}

void FeatherstoneEntity::setJointDamping(size_t index, Scalar constantFactor, Scalar viscousFactor)
{
    if(index >= joints_.size())
        return;
    
    switch (joints_[index].type)
    {
        case btMultibodyLink::eRevolute:
            joints_[index].sigDamping = constantFactor > Scalar(0) ? constantFactor : Scalar(0);
            break;
        
        case btMultibodyLink::ePrismatic:
            joints_[index].sigDamping = constantFactor > Scalar(0) ? constantFactor : Scalar(0);
            break;
            
        default:
            break;
    }
    
    joints_[index].velDamping = viscousFactor > Scalar(0) ? viscousFactor : Scalar(0);
}

const FeatherstoneJoint& FeatherstoneEntity::getJoint(size_t index)
{
    return joints_.at(index);
}

const std::string& FeatherstoneEntity::getJointName(size_t index) const
{
    if (index < joints_.size())
        return joints_[index].name;
    else
        throw std::invalid_argument("Joint with id=" + std::to_string(index) + " does not exist for '" + getName() + "'!");
}

void FeatherstoneEntity::getJointPosition(size_t index, Scalar &position, btMultibodyLink::eFeatherstoneJointType &jointType)
{
    if(index >= joints_.size())
    {
        jointType = btMultibodyLink::eInvalid;
        position = Scalar(0.);
    }
    else
    {
        switch (joints_[index].type)
        {
            case btMultibodyLink::eRevolute:
                jointType = btMultibodyLink::eRevolute;
                position = multiBody_->getJointPos(joints_[index].child - 1);
                break;
                
            case btMultibodyLink::ePrismatic:
                jointType = btMultibodyLink::ePrismatic;
                position = multiBody_->getJointPos(joints_[index].child - 1);
                break;
                
            default:
                break;
        }
    }
}

void FeatherstoneEntity::getJointVelocity(size_t index, Scalar &velocity, btMultibodyLink::eFeatherstoneJointType &jointType)
{
    if(index >= joints_.size())
    {
        jointType = btMultibodyLink::eInvalid;
        velocity = Scalar(0.);
    }
    else
    {
        switch (joints_[index].type)
        {
            case btMultibodyLink::eRevolute:
                jointType = btMultibodyLink::eRevolute;
                velocity = multiBody_->getJointVel(joints_[index].child - 1);
                break;
                
            case btMultibodyLink::ePrismatic:
                jointType = btMultibodyLink::ePrismatic;
                velocity = multiBody_->getJointVel(joints_[index].child - 1);
                break;
                
            default:
                break;
        }
    }
}

Scalar FeatherstoneEntity::getJointTorque(size_t index)
{
    if(index >= joints_.size())
        return Scalar(0);
    else
        return multiBody_->getJointTorque(joints_[index].child - 1);
}

void FeatherstoneEntity::setMaxMotorForceTorque(size_t index, Scalar maxT)
{
    if(index >= joints_.size())
        return;
        
    if(joints_[index].motor == nullptr)
        return;
        
    joints_[index].motor->setMaxAppliedImpulse(maxT * Scalar(1)/SimulationApp::getApp()->getSimulationManager()->getStepsPerSecond());
}

Scalar FeatherstoneEntity::getMotorForceTorque(size_t index)
{
    if(index >= joints_.size() || joints_[index].motor == nullptr)
        return Scalar(0);
    else
        return joints_[index].motor->getAppliedImpulse(0) * SimulationApp::getApp()->getSimulationManager()->getStepsPerSecond();
}

size_t FeatherstoneEntity::getJointFeedback(size_t index, Vector3& force, Vector3& torque)
{
    if(index >= joints_.size())
    {
        force.setZero();
        torque.setZero();
        return 0;
    }
    else
    {
        force = Vector3(joints_[index].feedback->m_reactionForces.m_topVec[0],
                          joints_[index].feedback->m_reactionForces.m_topVec[1],
                          joints_[index].feedback->m_reactionForces.m_topVec[2]);
                          
        torque = Vector3(joints_[index].feedback->m_reactionForces.m_bottomVec[0],
                           joints_[index].feedback->m_reactionForces.m_bottomVec[1],
                           joints_[index].feedback->m_reactionForces.m_bottomVec[2]);
                      
        torque += joints_[index].pivotInChild.cross(force); //Add missing torque...
        
        return joints_[index].child;
    }
}

Vector3 FeatherstoneEntity::getJointAxis(size_t index)
{
    if(index >= joints_.size())
    {
        return Vector3(0,0,0);
    }
    else
    {
        return joints_[index].axisInChild;
    }
}

btMultiBody* FeatherstoneEntity::getMultiBody()
{
    return multiBody_.get();
}

const FeatherstoneLink& FeatherstoneEntity::getLink(size_t index)
{
    return links_[index];
}

Transform FeatherstoneEntity::getLinkTransform(size_t index)
{
    if(index >= links_.size())
        return Transform::getIdentity();
    
    if(index == 0)
        return multiBody_->getBaseWorldTransform();
    else
        return links_[index].solid->getCGTransform();
}

Vector3 FeatherstoneEntity::getLinkLinearVelocity(size_t index)
{
    if(index >= links_.size())
        return Vector3(0,0,0);
    
    return links_[index].solid->getLinearVelocity();
}

Vector3 FeatherstoneEntity::getLinkAngularVelocity(size_t index)
{
    if(index >= links_.size())
        return Vector3(0,0,0);
    
    return links_[index].solid->getAngularVelocity();
}

size_t FeatherstoneEntity::getNumOfLinks()
{
    return links_.size();
}

size_t FeatherstoneEntity::getNumOfJoints()
{
    return joints_.size();
}

size_t FeatherstoneEntity::getNumOfMovingJoints()
{
    size_t movingJoints = 0;
    for(size_t i=0; i<joints_.size(); ++i)
    {
        if(joints_[i].type != btMultibodyLink::eFixed)
            ++movingJoints;
    }
    
    return movingJoints;
}

void FeatherstoneEntity::AddLink(std::unique_ptr<SolidEntity> solid, const Transform& transform)
{
    if(solid != nullptr)
    {
        //Add link
        links_.push_back(FeatherstoneLink(std::move(solid), transform));
        //Build collider
        links_.back().solid->BuildMultibodyLinkCollider(multiBody_.get(), (int)(links_.size() - 1), SimulationApp::getApp()->getSimulationManager()->getDynamicsWorld());
        
        if(links_.size() > 1) //If not base link
        {
            Transform trans =  transform * links_[links_.size()-1].solid->getCG2OTransform().inverse();
            links_.back().solid->setCGTransform(trans);
        }
        else
        {
            Transform trans = transform * links_[0].solid->getCG2OTransform().inverse();
            links_[0].solid->setCGTransform(trans);
            multiBody_->setBaseWorldTransform(trans);
        }
    }
}

int FeatherstoneEntity::AddRevoluteJoint(const std::string& name, size_t parent, size_t child, const Vector3& pivot, const Vector3& axis, bool collisionBetweenJointLinks)
{
    //No self joint possible and base cannot be a child
    if(parent == child || child == 0)
        return -1;
    
    //Check if links exist
    if(parent >= links_.size() || child >= links_.size())
        return -1;
    
    //Instantiate joint structure
    FeatherstoneJoint joint(name, btMultibodyLink::eRevolute, parent, child);
    
    //Setup joint
    //q' = q2 * q1
    Quaternion ornParentToChild = getLinkTransform(child).getRotation().inverse() * getLinkTransform(parent).getRotation();
    Vector3 parentComToPivotOffset = getLinkTransform(parent).getBasis().inverse() * (pivot - getLinkTransform(parent).getOrigin());
    Vector3 pivotToChildComOffset =  getLinkTransform(child).getBasis().inverse() * (getLinkTransform(child).getOrigin() - pivot);
    
    //Get mass properties (including added mass)
    Scalar M = links_[child].solid->getAugmentedMass();
    Vector3 I = links_[child].solid->getAugmentedInertia();
    
    //Setup joint
    joint.axisInChild = getLinkTransform(child).getBasis().inverse() * axis.normalized();
    joint.pivotInChild = pivotToChildComOffset;
    multiBody_->setupRevolute(child - 1, M, I, parent - 1, ornParentToChild, joint.axisInChild, parentComToPivotOffset, pivotToChildComOffset, !collisionBetweenJointLinks);
   
    //Add feedback
    joint.feedback = new btMultiBodyJointFeedback();
    multiBody_->getLink((int)child - 1).m_jointFeedback = joint.feedback;
    joints_.push_back(joint);
    
    return ((int)joints_.size() - 1);
}

int FeatherstoneEntity::AddPrismaticJoint(const std::string& name, size_t parent, size_t child, const Vector3& axis, bool collisionBetweenJointLinks)
{
    //No self joint possible and base cannot be a child
    if(parent == child || child == 0)
        return -1;
    
    //Check if links exist
    if(parent >= links_.size() || child >= links_.size())
        return -1;
    
    //Instantiate joint structure
    FeatherstoneJoint joint(name, btMultibodyLink::ePrismatic, parent, child);
    
    //Setup joint
    //q' = q2 * q1
    Quaternion ornParentToChild = getLinkTransform(child).getRotation().inverse() * getLinkTransform(parent).getRotation();
    Vector3 parentComToPivotOffset = Vector3(0,0,0);
    Vector3 pivotToChildComOffset = getLinkTransform(child).getBasis().inverse() * (getLinkTransform(child).getOrigin()-getLinkTransform(parent).getOrigin());
    
    //Get mass properties (including added mass)
    Scalar M = links_[child].solid->getAugmentedMass();
    Vector3 I = links_[child].solid->getAugmentedInertia();
    
    //Check if pivot offset is ok!
    joint.axisInChild = getLinkTransform(child).getBasis().inverse() * axis.normalized();
    joint.pivotInChild = pivotToChildComOffset;
    multiBody_->setupPrismatic(child - 1, M, I, parent - 1, ornParentToChild, joint.axisInChild, parentComToPivotOffset, pivotToChildComOffset, !collisionBetweenJointLinks);
    
    //Add feedback
    joint.feedback = new btMultiBodyJointFeedback();
    multiBody_->getLink((int)child - 1).m_jointFeedback = joint.feedback;
    joints_.push_back(joint);
    
    return ((int)joints_.size() - 1);
}

int FeatherstoneEntity::AddFixedJoint(const std::string& name, size_t parent, size_t child, const Vector3& pivot)
{
    //No self joint possible and base cannot be a child
    if(parent == child || child == 0)
        return -1;
    
    //Check if links exist
    if(parent >= links_.size() || child >= links_.size())
        return -1;
    
    //Instantiate joint structure
    FeatherstoneJoint joint(name, btMultibodyLink::eFixed, parent, child);
    
    //Setup joint
    Quaternion ornParentToChild =  getLinkTransform(child).getRotation().inverse() * getLinkTransform(parent).getRotation();
    Vector3 parentComToPivotOffset = getLinkTransform(parent).getBasis().inverse() * (pivot - getLinkTransform(parent).getOrigin());
    Vector3 pivotToChildComOffset =  getLinkTransform(child).getBasis().inverse() * (getLinkTransform(child).getOrigin() - pivot);
    
    //Get mass properties (including added mass)
    Scalar M = links_[child].solid->getAugmentedMass();
    Vector3 I = links_[child].solid->getAugmentedInertia();
    
    //Setup joint
    joint.axisInChild = Vector3(0,0,0);
    joint.pivotInChild = pivotToChildComOffset;
    multiBody_->setupFixed(child - 1, M, I, parent - 1, ornParentToChild, parentComToPivotOffset, pivotToChildComOffset);
    
    //Add feedback
    joint.feedback = new btMultiBodyJointFeedback();
    multiBody_->getLink((int)child - 1).m_jointFeedback = joint.feedback;
    joints_.push_back(joint);
    
    return ((int)joints_.size() - 1);
}

void FeatherstoneEntity::AddJointLimit(size_t index, Scalar lower, Scalar upper)
{
    if(joints_[index].limit != nullptr
       || index >= joints_.size()
       || lower > upper)
        return;
        
    btMultiBodyJointLimitConstraint* jlc = new btMultiBodyJointLimitConstraint(multiBody_.get(), index, lower, upper);
    joints_[index].limit = jlc;
    joints_[index].lowerLimit = lower;
    joints_[index].upperLimit = upper;
}

void FeatherstoneEntity::AddJointMotor(size_t index, Scalar maxForceTorque)
{
    if(index >= joints_.size())
        return;
    
    if(joints_[index].motor != nullptr)
    {
        joints_[index].motor->setMaxAppliedImpulse(maxForceTorque * Scalar(1)/SimulationApp::getApp()->getSimulationManager()->getStepsPerSecond());
    }
    else
    {
        btMultiBodyJointMotor* jmc = new btMultiBodyJointMotor(multiBody_.get(), index, Scalar(0), maxForceTorque * Scalar(1)/SimulationApp::getApp()->getSimulationManager()->getStepsPerSecond());
        joints_[index].motor = jmc;
    }
}

void FeatherstoneEntity::MotorPositionSetpoint(size_t index, Scalar pos, Scalar kp)
{
    if(index >= joints_.size())
        return;
        
    if(joints_[index].motor == nullptr)
        return;
    
    if(joints_[index].lowerLimit < joints_[index].upperLimit) //if joint limits exist the desired position has to be restricted to avoid violating constraints!
        pos = pos < joints_[index].lowerLimit ? joints_[index].lowerLimit : (pos > joints_[index].upperLimit ? joints_[index].upperLimit : pos);
    
    joints_[index].motor->setPositionTarget(pos, kp);
}

void FeatherstoneEntity::MotorVelocitySetpoint(size_t index, Scalar vel, Scalar kd)
{
    if(index >= joints_.size())
        return;
        
    if(joints_[index].motor == nullptr)
        return;
        
    joints_[index].motor->setVelocityTarget(vel, kd);
}

void FeatherstoneEntity::DriveJoint(size_t index, Scalar forceTorque)
{
    if(index >= joints_.size())
        return;
        
    switch (joints_[index].type)
    {
        case btMultibodyLink::eRevolute:
            multiBody_->addJointTorque(joints_[index].child - 1, forceTorque);
            break;
            
        case btMultibodyLink::ePrismatic:
            multiBody_->addJointTorque(joints_[index].child - 1, forceTorque);
            break;
            
        default:
            break;
    }
}

void FeatherstoneEntity::ApplyGravity(const Vector3& g)
{
    bool isSleeping = false;
    
    if(multiBody_->getBaseCollider() && multiBody_->getBaseCollider()->getActivationState() == ISLAND_SLEEPING)
        isSleeping = true;
    
    for(int i=0; i<multiBody_->getNumLinks(); ++i)
    {
        if(multiBody_->getLink(i).m_collider && multiBody_->getLink(i).m_collider->getActivationState() == ISLAND_SLEEPING)
        {
            isSleeping = true;
            break;
        }
    } 

    if(!isSleeping)
    {
        multiBody_->addBaseForce(g * links_[0].solid->getMass());

        for(int i=0; i<multiBody_->getNumLinks(); ++i) 
        {
            multiBody_->addLinkForce(i, g * links_[i+1].solid->getMass());
        }
    }
}

void FeatherstoneEntity::ApplyDamping()
{
    for(size_t i=0; i<joints_.size(); ++i)
    {
        if(joints_[i].sigDamping >= SIMD_EPSILON || joints_[i].velDamping >= SIMD_EPSILON) //If damping factors not equal zero
        {
            Scalar velocity = multiBody_->getJointVel(joints_[i].child - 1);
            
            if(btFabs(velocity) >= SIMD_EPSILON) //If velocity higher than zero
            {
                Scalar damping = - velocity/btFabs(velocity) * joints_[i].sigDamping - velocity * joints_[i].velDamping;
                multiBody_->addJointTorque(joints_[i].child - 1, damping);
            }
        }
    }
}

void FeatherstoneEntity::AddLinkForce(size_t index, const Vector3& F)
{
    if(index >= links_.size())
        return;
    
    if(index == 0)
        multiBody_->addBaseForce(F);
    else
        multiBody_->addLinkForce(index-1, F);
}
 
void FeatherstoneEntity::AddLinkTorque(size_t index, const Vector3& tau)
{
    if(index >= links_.size())
        return;
        
    if(index == 0)
        multiBody_->addBaseTorque(tau);
    else
        multiBody_->addLinkTorque(index-1, tau);
}

void FeatherstoneEntity::UpdateAcceleration(Scalar dt)
{
    for(size_t i = 0; i<links_.size(); ++i)
        links_[i].solid->UpdateAcceleration(dt);
}

std::vector<Renderable> FeatherstoneEntity::Render()
{	
    std::vector<Renderable> items(0);
    
    //Draw base
    if(baseRenderable_)
    {
        std::vector<Renderable> _base = links_[0].solid->Render();
        items.insert(items.end(), _base.begin(), _base.end());
    }
    
    //Draw rest of links
    for(size_t i = 1; i < links_.size(); ++i)
    {
        std::vector<Renderable> _link = links_[i].solid->Render();
        items.insert(items.end(), _link.begin(), _link.end());
    }
    
    //Draw link axes
    Renderable item;
    item.type = RenderableType::MULTIBODY_AXIS;
    item.model = glm::mat4(1.f);
    item.data = std::make_shared<std::vector<glm::vec3>>();
    auto points = item.getDataAsPoints();
    
    for(size_t i = 1; i < links_.size(); ++i)
    {
        btMultibodyLink& link = multiBody_->getLink((int)i-1);
        Vector3 pivot = getLinkTransform(link.m_parent+1) * link.m_eVector;
        Vector3 axisEnd = pivot;
        
        if(link.m_jointType == btMultibodyLink::eFeatherstoneJointType::eRevolute)
        {
            Vector3 axisInWorld = getLinkTransform((size_t)i).getBasis() * link.getAxisTop(0);
            axisEnd += axisInWorld * Scalar(0.3);
        }
        else if(link.m_jointType == btMultibodyLink::eFeatherstoneJointType::ePrismatic)
        {
            Vector3 axisInWorld = getLinkTransform((size_t)i).getBasis() * link.getAxisBottom(0);
            axisEnd += axisInWorld * Scalar(0.3);
        }
        
        points->push_back(glm::vec3((GLfloat)pivot.x(), (GLfloat)pivot.y(), (GLfloat)pivot.z()));
        points->push_back(glm::vec3((GLfloat)axisEnd.x(), (GLfloat)axisEnd.y(), (GLfloat)axisEnd.z()));
    }
    
    items.push_back(item);
    
    return items;
}

}
