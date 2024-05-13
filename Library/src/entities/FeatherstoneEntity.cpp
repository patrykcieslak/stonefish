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
//  Copyright (c) 2013-2023 Patryk Cieslak. All rights reserved.
//

#include "entities/FeatherstoneEntity.h"

#include "core/SimulationApp.h"
#include "core/SimulationManager.h"
#include "entities/StaticEntity.h"

namespace sf
{

FeatherstoneEntity::FeatherstoneEntity(std::string uniqueName, unsigned int totalNumOfLinks, SolidEntity* baseSolid, bool fixedBase) : Entity(uniqueName)
{
    Scalar M = baseSolid->getAugmentedMass();
    Vector3 I = baseSolid->getAugmentedInertia();
    multiBody = new btMultiBody(totalNumOfLinks - 1, M, I, fixedBase, true);
    multiBody->setBaseWorldTransform(Transform::getIdentity());
    multiBody->setAngularDamping(Scalar(0));
    multiBody->setLinearDamping(Scalar(0));
    multiBody->setMaxAppliedImpulse(BT_LARGE_FLOAT);
    multiBody->setMaxCoordinateVelocity(Scalar(1000));
    multiBody->useRK4Integration(false); //Enabling RK4 causes unreallistic energy accumulation (strange motions in 0 gravity)
    multiBody->useGlobalVelocities(false); //See previous comment
    multiBody->setHasSelfCollision(false); //No self collision by default
    multiBody->setUseGyroTerm(true);
    multiBody->setCanSleep(true);
   
    AddLink(baseSolid, Transform::getIdentity());
    
    baseRenderable = true;
}

FeatherstoneEntity::~FeatherstoneEntity()
{
    multiBody = nullptr;
    
    for(unsigned int i=0; i<links.size(); ++i)
        delete links[i].solid;
    
    links.clear();
    joints.clear();
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
    
    for(unsigned int i = 0; i < links.size(); i++)
    {
        //Get link AABB
        Vector3 lmin;
        Vector3 lmax;
        links[i].solid->multibodyCollider->getCollisionShape()->getAabb(getLinkTransform(i), lmin, lmax);
        
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
    for(size_t i=0; i<joints.size(); ++i)
    {    
        if(joints[i].limit != nullptr)
            sm->getDynamicsWorld()->addMultiBodyConstraint(joints[i].limit);
    }
    
    //Creating motors (has to be after joint limits and not interleaved!)
    for(size_t i=0; i<joints.size(); ++i)
    {
        if(joints[i].motor != nullptr)
            sm->getDynamicsWorld()->addMultiBodyConstraint(joints[i].motor);
    }
    
    //Resize matrices
    multiBody->finalizeMultiDof();
    
    //Add multibody to the world
    Respawn(origin);
    sm->getDynamicsWorld()->addMultiBody(multiBody);
}

void FeatherstoneEntity::RemoveFromSimulation(SimulationManager* sm)
{
    sm->getDynamicsWorld()->removeMultiBody(multiBody);
}

void FeatherstoneEntity::Respawn(const Transform& origin)
{
    //Set origin position
    setBaseTransform(origin);
    multiBody->setBaseVel(Vector3(0,0,0));
    multiBody->setBaseOmega(Vector3(0,0,0));
    
    //Move joints to the limits
    for(size_t i=0; i<joints.size(); ++i)
    {
        if(joints[i].limit != nullptr)
        {
            if(joints[i].lowerLimit > Scalar(0))
                multiBody->setJointPos((int)i, joints[i].lowerLimit);
            else if(joints[i].upperLimit < Scalar(0))
                multiBody->setJointPos((int)i, joints[i].upperLimit);
        }
    }
    
    //Calculate constrained link positions to avoid jump at the start of simulation
    btAlignedObjectArray<Quaternion> scratchQ;
    btAlignedObjectArray<Vector3> scratchM;
    multiBody->forwardKinematics(scratchQ, scratchM);
    multiBody->updateCollisionObjectWorldTransforms(scratchQ, scratchM);
}

void FeatherstoneEntity::setSelfCollision(bool enabled)
{
    multiBody->setHasSelfCollision(enabled);
}

bool FeatherstoneEntity::hasSelfCollision() const
{
    return multiBody->hasSelfCollision();
}

void FeatherstoneEntity::setDisplayMode(DisplayMode m)
{
    for(size_t i=0; i<links.size(); ++i)
        links[i].solid->setDisplayMode(m);
}

void FeatherstoneEntity::setBaseRenderable(bool render)
{
    baseRenderable = render;
}

void FeatherstoneEntity::setBaseTransform(const Transform& trans)
{
    Transform T0 = trans * links[0].solid->getCG2CTransform().inverse();
    multiBody->getBaseCollider()->setWorldTransform(T0);
    multiBody->setBaseWorldTransform(T0);
    
    for(unsigned int i=1; i<links.size(); ++i)
    {
        Transform tr = links[i].solid->multibodyCollider->getWorldTransform();
        links[i].solid->multibodyCollider->setWorldTransform(trans * tr);
    }
}

void FeatherstoneEntity::setJointIC(unsigned int index, Scalar position, Scalar velocity)
{
    if(index >= joints.size())
        return;
    
    switch (joints[index].type)
    {
        case btMultibodyLink::eRevolute:
            multiBody->setJointPos(joints[index].child - 1, position);
            multiBody->setJointVel(joints[index].child - 1, velocity);
            break;
            
        case btMultibodyLink::ePrismatic:
            multiBody->setJointPos(joints[index].child - 1, position);
            multiBody->setJointVel(joints[index].child - 1, velocity);
            break;
            
        default:
            break;
    }
}

void FeatherstoneEntity::setJointDamping(unsigned int index, Scalar constantFactor, Scalar viscousFactor)
{
    if(index >= joints.size())
        return;
    
    switch (joints[index].type)
    {
        case btMultibodyLink::eRevolute:
            joints[index].sigDamping = constantFactor > Scalar(0) ? constantFactor : Scalar(0);
            break;
        
        case btMultibodyLink::ePrismatic:
            joints[index].sigDamping = constantFactor > Scalar(0) ? constantFactor : Scalar(0);
            break;
            
        default:
            break;
    }
    
    joints[index].velDamping = viscousFactor > Scalar(0) ? viscousFactor : Scalar(0);
}

FeatherstoneJoint FeatherstoneEntity::getJoint(unsigned int index)
{
    return joints[index];
}

std::string FeatherstoneEntity::getJointName(unsigned int index)
{
    if(index >= joints.size())
        return "Invalid id!";
    else 
        return joints[index].name;
}

void FeatherstoneEntity::getJointPosition(unsigned int index, Scalar &position, btMultibodyLink::eFeatherstoneJointType &jointType)
{
    if(index >= joints.size())
    {
        jointType = btMultibodyLink::eInvalid;
        position = Scalar(0.);
    }
    else
    {
        switch (joints[index].type)
        {
            case btMultibodyLink::eRevolute:
                jointType = btMultibodyLink::eRevolute;
                position = multiBody->getJointPos(joints[index].child - 1);
                break;
                
            case btMultibodyLink::ePrismatic:
                jointType = btMultibodyLink::ePrismatic;
                position = multiBody->getJointPos(joints[index].child - 1);
                break;
                
            default:
                break;
        }
    }
}

void FeatherstoneEntity::getJointVelocity(unsigned int index, Scalar &velocity, btMultibodyLink::eFeatherstoneJointType &jointType)
{
    if(index >= joints.size())
    {
        jointType = btMultibodyLink::eInvalid;
        velocity = Scalar(0.);
    }
    else
    {
        switch (joints[index].type)
        {
            case btMultibodyLink::eRevolute:
                jointType = btMultibodyLink::eRevolute;
                velocity = multiBody->getJointVel(joints[index].child - 1);
                break;
                
            case btMultibodyLink::ePrismatic:
                jointType = btMultibodyLink::ePrismatic;
                velocity = multiBody->getJointVel(joints[index].child - 1);
                break;
                
            default:
                break;
        }
    }
}

Scalar FeatherstoneEntity::getJointTorque(unsigned int index)
{
    if(index >= joints.size())
        return Scalar(0);
    else
        return multiBody->getJointTorque(joints[index].child - 1);
}

void FeatherstoneEntity::setMaxMotorForceTorque(unsigned int index, Scalar maxT)
{
    if(index >= joints.size())
        return;
        
    if(joints[index].motor == nullptr)
        return;
        
    joints[index].motor->setMaxAppliedImpulse(maxT * Scalar(1)/SimulationApp::getApp()->getSimulationManager()->getStepsPerSecond());
}

Scalar FeatherstoneEntity::getMotorForceTorque(unsigned int index)
{
    if(index >= joints.size() || joints[index].motor == nullptr)
        return Scalar(0);
    else
        return joints[index].motor->getAppliedImpulse(0) * SimulationApp::getApp()->getSimulationManager()->getStepsPerSecond();
}

unsigned int FeatherstoneEntity::getJointFeedback(unsigned int index, Vector3& force, Vector3& torque)
{
    if(index >= joints.size())
    {
        force.setZero();
        torque.setZero();
        return 0;
    }
    else
    {
        force = Vector3(joints[index].feedback->m_reactionForces.m_topVec[0],
                          joints[index].feedback->m_reactionForces.m_topVec[1],
                          joints[index].feedback->m_reactionForces.m_topVec[2]);
                          
        torque = Vector3(joints[index].feedback->m_reactionForces.m_bottomVec[0],
                           joints[index].feedback->m_reactionForces.m_bottomVec[1],
                           joints[index].feedback->m_reactionForces.m_bottomVec[2]);
                      
        torque += joints[index].pivotInChild.cross(force); //Add missing torque...
        
        return joints[index].child;
    }
}

Vector3 FeatherstoneEntity::getJointAxis(unsigned int index)
{
    if(index >= joints.size())
    {
        return Vector3(0,0,0);
    }
    else
    {
        return joints[index].axisInChild;
    }
}

btMultiBody* FeatherstoneEntity::getMultiBody()
{
    return multiBody;
}

FeatherstoneLink FeatherstoneEntity::getLink(unsigned int index)
{
    return links[index];
}

Transform FeatherstoneEntity::getLinkTransform(unsigned int index)
{
    if(index >= links.size())
        return Transform::getIdentity();
    
    if(index == 0)
        return multiBody->getBaseWorldTransform();
    else
        return links[index].solid->getCGTransform();
}

Vector3 FeatherstoneEntity::getLinkLinearVelocity(unsigned int index)
{
    if(index >= links.size())
        return Vector3(0,0,0);
    
    return links[index].solid->getLinearVelocity();
}

Vector3 FeatherstoneEntity::getLinkAngularVelocity(unsigned int index)
{
    if(index >= links.size())
        return Vector3(0,0,0);
    
    return links[index].solid->getAngularVelocity();
}

unsigned int FeatherstoneEntity::getNumOfLinks()
{
    return (unsigned int)links.size();
}

unsigned int FeatherstoneEntity::getNumOfJoints()
{
    return (unsigned int)joints.size();
}

unsigned int FeatherstoneEntity::getNumOfMovingJoints()
{
    unsigned int movingJoints = 0;
    for(size_t i=0; i<joints.size(); ++i)
    {
        if(joints[i].type != btMultibodyLink::eFixed)
            ++movingJoints;
    }
    
    return movingJoints;
}

void FeatherstoneEntity::AddLink(SolidEntity *solid, const Transform& transform)
{
    if(solid != nullptr)
    {
        //Add link
        links.push_back(FeatherstoneLink(solid, transform));
        //Build collider
        links.back().solid->BuildMultibodyLinkCollider(multiBody, (int)(links.size() - 1), SimulationApp::getApp()->getSimulationManager()->getDynamicsWorld());
        
        if(links.size() > 1) //If not base link
        {
            Transform trans =  transform * links[links.size()-1].solid->getCG2OTransform().inverse();
            links.back().solid->setCGTransform(trans);
        }
        else
        {
            Transform trans = transform * links[0].solid->getCG2OTransform().inverse();
            links[0].solid->setCGTransform(trans);
            multiBody->setBaseWorldTransform(trans);
        }
    }
}

int FeatherstoneEntity::AddRevoluteJoint(std::string name, unsigned int parent, unsigned int child, const Vector3& pivot, const Vector3& axis, bool collisionBetweenJointLinks)
{
    //No self joint possible and base cannot be a child
    if(parent == child || child == 0)
        return -1;
    
    //Check if links exist
    if(parent >= links.size() || child >= links.size())
        return -1;
    
    //Instantiate joint structure
    FeatherstoneJoint joint(name, btMultibodyLink::eRevolute, parent, child);
    
    //Setup joint
    //q' = q2 * q1
    Quaternion ornParentToChild = getLinkTransform(child).getRotation().inverse() * getLinkTransform(parent).getRotation();
    Vector3 parentComToPivotOffset = getLinkTransform(parent).getBasis().inverse() * (pivot - getLinkTransform(parent).getOrigin());
    Vector3 pivotToChildComOffset =  getLinkTransform(child).getBasis().inverse() * (getLinkTransform(child).getOrigin() - pivot);
    
    //Get mass properties (including added mass)
    Scalar M = links[child].solid->getAugmentedMass();
    Vector3 I = links[child].solid->getAugmentedInertia();
    
    //Setup joint
    joint.axisInChild = getLinkTransform(child).getBasis().inverse() * axis.normalized();
    joint.pivotInChild = pivotToChildComOffset;
    multiBody->setupRevolute(child - 1, M, I, parent - 1, ornParentToChild, joint.axisInChild, parentComToPivotOffset, pivotToChildComOffset, !collisionBetweenJointLinks);
   
    //Add feedback
    joint.feedback = new btMultiBodyJointFeedback();
    multiBody->getLink((int)child - 1).m_jointFeedback = joint.feedback;
    joints.push_back(joint);
    
    return ((int)joints.size() - 1);
}

int FeatherstoneEntity::AddPrismaticJoint(std::string name, unsigned int parent, unsigned int child, const Vector3& axis, bool collisionBetweenJointLinks)
{
    //No self joint possible and base cannot be a child
    if(parent == child || child == 0)
        return -1;
    
    //Check if links exist
    if(parent >= links.size() || child >= links.size())
        return -1;
    
    //Instantiate joint structure
    FeatherstoneJoint joint(name, btMultibodyLink::ePrismatic, parent, child);
    
    //Setup joint
    //q' = q2 * q1
    Quaternion ornParentToChild = getLinkTransform(child).getRotation().inverse() * getLinkTransform(parent).getRotation();
    Vector3 parentComToPivotOffset = Vector3(0,0,0);
    Vector3 pivotToChildComOffset = getLinkTransform(child).getBasis().inverse() * (getLinkTransform(child).getOrigin()-getLinkTransform(parent).getOrigin());
    
    //Get mass properties (including added mass)
    Scalar M = links[child].solid->getAugmentedMass();
    Vector3 I = links[child].solid->getAugmentedInertia();
    
    //Check if pivot offset is ok!
    joint.axisInChild = getLinkTransform(child).getBasis().inverse() * axis.normalized();
    joint.pivotInChild = pivotToChildComOffset;
    multiBody->setupPrismatic(child - 1, M, I, parent - 1, ornParentToChild, joint.axisInChild, parentComToPivotOffset, pivotToChildComOffset, !collisionBetweenJointLinks);
    
    //Add feedback
    joint.feedback = new btMultiBodyJointFeedback();
    multiBody->getLink((int)child - 1).m_jointFeedback = joint.feedback;
    joints.push_back(joint);
    
    return ((int)joints.size() - 1);
}

int FeatherstoneEntity::AddFixedJoint(std::string name, unsigned int parent, unsigned int child, const Vector3& pivot)
{
    //No self joint possible and base cannot be a child
    if(parent == child || child == 0)
        return -1;
    
    //Check if links exist
    if(parent >= links.size() || child >= links.size())
        return -1;
    
    //Instantiate joint structure
    FeatherstoneJoint joint(name, btMultibodyLink::eFixed, parent, child);
    
    //Setup joint
    Quaternion ornParentToChild =  getLinkTransform(child).getRotation().inverse() * getLinkTransform(parent).getRotation();
    Vector3 parentComToPivotOffset = getLinkTransform(parent).getBasis().inverse() * (pivot - getLinkTransform(parent).getOrigin());
    Vector3 pivotToChildComOffset =  getLinkTransform(child).getBasis().inverse() * (getLinkTransform(child).getOrigin() - pivot);
    
    //Get mass properties (including added mass)
    Scalar M = links[child].solid->getAugmentedMass();
    Vector3 I = links[child].solid->getAugmentedInertia();
    
    //Setup joint
    joint.axisInChild = Vector3(0,0,0);
    joint.pivotInChild = pivotToChildComOffset;
    multiBody->setupFixed(child - 1, M, I, parent - 1, ornParentToChild, parentComToPivotOffset, pivotToChildComOffset);
    
    //Add feedback
    joint.feedback = new btMultiBodyJointFeedback();
    multiBody->getLink((int)child - 1).m_jointFeedback = joint.feedback;
    joints.push_back(joint);
    
    return ((int)joints.size() - 1);
}

void FeatherstoneEntity::AddJointLimit(unsigned int index, Scalar lower, Scalar upper)
{
    if(joints[index].limit != nullptr
       || index >= joints.size()
       || lower > upper)
        return;
        
    btMultiBodyJointLimitConstraint* jlc = new btMultiBodyJointLimitConstraint(multiBody, index, lower, upper);
    joints[index].limit = jlc;
    joints[index].lowerLimit = lower;
    joints[index].upperLimit = upper;
}

void FeatherstoneEntity::AddJointMotor(unsigned int index, Scalar maxForceTorque)
{
    if(index >= joints.size())
        return;
    
    if(joints[index].motor != nullptr)
    {
        joints[index].motor->setMaxAppliedImpulse(maxForceTorque * Scalar(1)/SimulationApp::getApp()->getSimulationManager()->getStepsPerSecond());
    }
    else
    {
        btMultiBodyJointMotor* jmc = new btMultiBodyJointMotor(multiBody, index, Scalar(0), maxForceTorque * Scalar(1)/SimulationApp::getApp()->getSimulationManager()->getStepsPerSecond());
        joints[index].motor = jmc;
    }
}

void FeatherstoneEntity::MotorPositionSetpoint(unsigned int index, Scalar pos, Scalar kp)
{
    if(index >= joints.size())
        return;
        
    if(joints[index].motor == nullptr)
        return;
    
    if(joints[index].lowerLimit < joints[index].upperLimit) //if joint limits exist the desired position has to be restricted to avoid violating constraints!
        pos = pos < joints[index].lowerLimit ? joints[index].lowerLimit : (pos > joints[index].upperLimit ? joints[index].upperLimit : pos);
    
    joints[index].motor->setPositionTarget(pos, kp);
}

void FeatherstoneEntity::MotorVelocitySetpoint(unsigned int index, Scalar vel, Scalar kd)
{
    if(index >= joints.size())
        return;
        
    if(joints[index].motor == nullptr)
        return;
        
    joints[index].motor->setVelocityTarget(vel, kd);
}

void FeatherstoneEntity::DriveJoint(unsigned int index, Scalar forceTorque)
{
    if(index >= joints.size())
        return;
        
    switch (joints[index].type)
    {
        case btMultibodyLink::eRevolute:
            multiBody->addJointTorque(joints[index].child - 1, forceTorque);
            break;
            
        case btMultibodyLink::ePrismatic:
            multiBody->addJointTorque(joints[index].child - 1, forceTorque);
            break;
            
        default:
            break;
    }
}

void FeatherstoneEntity::ApplyGravity(const Vector3& g)
{
    bool isSleeping = false;
    
    if(multiBody->getBaseCollider() && multiBody->getBaseCollider()->getActivationState() == ISLAND_SLEEPING)
        isSleeping = true;
    
    for(int i=0; i<multiBody->getNumLinks(); ++i)
    {
        if(multiBody->getLink(i).m_collider && multiBody->getLink(i).m_collider->getActivationState() == ISLAND_SLEEPING)
        {
            isSleeping = true;
            break;
        }
    } 

    if(!isSleeping)
    {
        multiBody->addBaseForce(g * links[0].solid->getMass());

        for(int i=0; i<multiBody->getNumLinks(); ++i) 
        {
            multiBody->addLinkForce(i, g * links[i+1].solid->getMass());
        }
    }
}

void FeatherstoneEntity::ApplyDamping()
{
    for(unsigned int i=0; i<joints.size(); ++i)
    {
        if(joints[i].sigDamping >= SIMD_EPSILON || joints[i].velDamping >= SIMD_EPSILON) //If damping factors not equal zero
        {
            Scalar velocity = multiBody->getJointVel(joints[i].child - 1);
            
            if(btFabs(velocity) >= SIMD_EPSILON) //If velocity higher than zero
            {
                Scalar damping = - velocity/btFabs(velocity) * joints[i].sigDamping - velocity * joints[i].velDamping;
                multiBody->addJointTorque(joints[i].child - 1, damping);
            }
        }
    }
}

void FeatherstoneEntity::AddLinkForce(unsigned int index, const Vector3& F)
{
    if(index >= links.size())
        return;
    
    if(index == 0)
        multiBody->addBaseForce(F);
    else
        multiBody->addLinkForce(index-1, F);
}
 
void FeatherstoneEntity::AddLinkTorque(unsigned int index, const Vector3& tau)
{
    if(index >= links.size())
        return;
        
    if(index == 0)
        multiBody->addBaseTorque(tau);
    else
        multiBody->addLinkTorque(index-1, tau);
}

void FeatherstoneEntity::UpdateAcceleration(Scalar dt)
{
    for(size_t i = 0; i<links.size(); ++i)
        links[i].solid->UpdateAcceleration(dt);
}

std::vector<Renderable> FeatherstoneEntity::Render()
{	
    std::vector<Renderable> items(0);
    
    //Draw base
    if(baseRenderable)
    {
        std::vector<Renderable> _base = links[0].solid->Render();
        items.insert(items.end(), _base.begin(), _base.end());
    }
    
    //Draw rest of links
    for(size_t i = 1; i < links.size(); ++i)
    {
        std::vector<Renderable> _link = links[i].solid->Render();
        items.insert(items.end(), _link.begin(), _link.end());
    }
    
    //Draw link axes
    Renderable item;
    item.type = RenderableType::MULTIBODY_AXIS;
    item.model = glm::mat4(1.f);
    
    for(size_t i = 1; i < links.size(); ++i)
    {
        btMultibodyLink& link = multiBody->getLink((int)i-1);
        Vector3 pivot = getLinkTransform(link.m_parent+1) * link.m_eVector;
        Vector3 axisEnd = pivot;
        
        if(link.m_jointType == btMultibodyLink::eFeatherstoneJointType::eRevolute)
        {
            Vector3 axisInWorld = getLinkTransform((unsigned int)i).getBasis() * link.getAxisTop(0);
            axisEnd += axisInWorld * Scalar(0.3);
        }
        else if(link.m_jointType == btMultibodyLink::eFeatherstoneJointType::ePrismatic)
        {
            Vector3 axisInWorld = getLinkTransform((unsigned int)i).getBasis() * link.getAxisBottom(0);
            axisEnd += axisInWorld * Scalar(0.3);
        }
        
        item.points.push_back(glm::vec3((GLfloat)pivot.x(), (GLfloat)pivot.y(), (GLfloat)pivot.z()));
        item.points.push_back(glm::vec3((GLfloat)axisEnd.x(), (GLfloat)axisEnd.y(), (GLfloat)axisEnd.z()));
    }
    
    items.push_back(item);
    
    return items;
}

}
