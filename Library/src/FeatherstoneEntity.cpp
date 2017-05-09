//
//  FeatherstoneEntity.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 8/20/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#include "FeatherstoneEntity.h"
#include "OpenGLSolids.h"

#pragma mark Constructors
FeatherstoneEntity::FeatherstoneEntity(std::string uniqueName, unsigned int totalNumOfLinks, SolidEntity* baseSolid, const btTransform& transform, btMultiBodyDynamicsWorld* world, bool fixedBase) : Entity(uniqueName)
{
    baseRenderable = fixedBase ? false : true;
    
    multiBody = new btMultiBody(totalNumOfLinks - 1, baseSolid->getMass(), baseSolid->getMomentsOfInertia(), fixedBase, true);
    btTransform trans = baseSolid->getLocalTransform() * UnitSystem::SetTransform(transform);
    
    multiBody->setBaseWorldTransform(trans);
    multiBody->setUseGyroTerm(true);
    multiBody->setAngularDamping(0.0);
    multiBody->setLinearDamping(0.0);
    multiBody->setMaxAppliedImpulse(10000.0);
    multiBody->setMaxCoordinateVelocity(10000.0);
    multiBody->useRK4Integration(true);
    multiBody->useGlobalVelocities(false);
    multiBody->setHasSelfCollision(false);
    multiBody->setCanSleep(false);
    
    AddLink(baseSolid, transform, world);
}

#pragma mark - Destructor
FeatherstoneEntity::~FeatherstoneEntity()
{
    multiBody = NULL;
    
    for(int i=0; i<links.size(); i++)
        delete links[i].solid;
    
    links.clear();
    joints.clear();
}

#pragma mark - Entity
EntityType FeatherstoneEntity::getType()
{
    return ENTITY_FEATHERSTONE;
}

void FeatherstoneEntity::GetAABB(btVector3& min, btVector3& max)
{
    //Initialize AABB
    min = btVector3(BT_LARGE_FLOAT, BT_LARGE_FLOAT, BT_LARGE_FLOAT);
    max = btVector3(-BT_LARGE_FLOAT, -BT_LARGE_FLOAT, -BT_LARGE_FLOAT);
    
    for(int i = 0; i < links.size(); i++)
    {
        //Get link AABB
        btVector3 lmin;
        btVector3 lmax;
        links[i].solid->getMultibodyLinkCollider()->getCollisionShape()->getAabb(getLinkTransform(i), lmin, lmax);
        
        //Merge with other AABBs
        min[0] = std::min(min[0], lmin[0]);
        min[1] = std::min(min[1], lmin[1]);
        min[2] = std::min(min[2], lmin[2]);
        
        max[0] = std::max(max[0], lmax[0]);
        max[1] = std::max(max[1], lmax[1]);
        max[2] = std::max(max[2], lmax[2]);
    }
}

void FeatherstoneEntity::Render()
{
    if(baseRenderable)
    {
        //Draw base
        btTransform trans = multiBody->getBaseCollider()->getWorldTransform() * links[0].solid->localTransform.inverse();
        btScalar openglTrans[16];
        trans.getOpenGLMatrix(openglTrans);
        
        glPushMatrix();
#ifdef BT_USE_DOUBLE_PRECISION
        glMultMatrixd(openglTrans);
#else
        glMultMatrixf(openglTrans);
#endif
        UseLook(links[0].solid->look);
        glCallList(links[0].solid->displayList);
        
        glPopMatrix();
    }
    
    //Draw rest of links
    for(unsigned int i = 0; i < multiBody->getNumLinks(); i++)
    {
        btMultibodyLink& link = multiBody->getLink(i);
        btTransform trans = link.m_collider->getWorldTransform() * links[i + 1].solid->localTransform.inverse();
        btScalar openglTrans[16];
        trans.getOpenGLMatrix(openglTrans);
        
        glPushMatrix();
#ifdef BT_USE_DOUBLE_PRECISION
        glMultMatrixd(openglTrans);
#else
        glMultMatrixf(openglTrans);
#endif
        UseLook(links[i + 1].solid->look);
        glCallList(links[i + 1].solid->displayList);
        glPopMatrix();
    }
}

void FeatherstoneEntity::AddToDynamicsWorld(btMultiBodyDynamicsWorld* world)
{
    multiBody->setBaseVel(btVector3(0,0,0));
    multiBody->setBaseOmega(btVector3(0,0,0));
    world->addMultiBody(multiBody);
}

#pragma mark - Accessors
void FeatherstoneEntity::setBaseRenderable(bool render)
{
    baseRenderable = render;
}

void FeatherstoneEntity::setJointIC(unsigned int index, btScalar position, btScalar velocity)
{
    if(index >= joints.size())
        return;
    
    switch (joints[index].type)
    {
        case btMultibodyLink::eRevolute:
            multiBody->setJointPos(joints[index].child - 1, UnitSystem::SetAngle(position));
            multiBody->setJointVel(joints[index].child - 1, UnitSystem::SetAngle(velocity));
            break;
            
        case btMultibodyLink::ePrismatic:
            multiBody->setJointPos(joints[index].child - 1, UnitSystem::SetLength(position));
            multiBody->setJointVel(joints[index].child - 1, UnitSystem::SetLength(velocity));
            break;
            
        default:
            break;
    }
}

void FeatherstoneEntity::setJointDamping(unsigned int index, btScalar constantFactor, btScalar viscousFactor)
{
    if(index >= joints.size())
        return;
    
    switch (joints[index].type)
    {
        case btMultibodyLink::eRevolute:
            joints[index].sigDamping = constantFactor > btScalar(0.) ? UnitSystem::SetTorque(constantFactor) : btScalar(0.);
            break;
        
        case btMultibodyLink::ePrismatic:
            joints[index].sigDamping = constantFactor > btScalar(0.) ? UnitSystem::SetForce(constantFactor) : btScalar(0.);
            break;
            
        default:
            break;
    }
    
    joints[index].velDamping = viscousFactor > btScalar(0.) ? viscousFactor : btScalar(0.);
}

void FeatherstoneEntity::getJointPosition(unsigned int index, btScalar &position, btMultibodyLink::eFeatherstoneJointType &jointType)
{
    if(index >= joints.size())
    {
        jointType = btMultibodyLink::eInvalid;
        position = btScalar(0.);
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

void FeatherstoneEntity::getJointVelocity(unsigned int index, btScalar &velocity, btMultibodyLink::eFeatherstoneJointType &jointType)
{
    if(index >= joints.size())
    {
        jointType = btMultibodyLink::eInvalid;
        velocity = btScalar(0.);
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

btTransform FeatherstoneEntity::getLinkTransform(unsigned int index)
{
    if(index >= links.size())
        return btTransform::getIdentity();
    
    return links[index].solid->getTransform();
}

btVector3 FeatherstoneEntity::getLinkLinearVelocity(unsigned int index)
{
    if(index >= links.size())
        return btVector3(0.,0.,0.);
    
    return links[index].solid->getLinearVelocity();
}

btVector3 FeatherstoneEntity::getLinkAngularVelocity(unsigned int index)
{
    if(index >= links.size())
        return btVector3(0.,0.,0.);
    
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

#pragma mark - Creators
void FeatherstoneEntity::AddLink(SolidEntity *solid, const btTransform& transform, btMultiBodyDynamicsWorld* world)
{
    if(solid != NULL)
    {
        links.push_back(FeatherstoneLink(solid, UnitSystem::SetTransform(transform)));
        links[(int)(links.size() - 1)].solid->BuildMultibodyLinkCollider(multiBody, (int)(links.size() - 1), world);
        
        if(links.size() > 1)
        {
            btTransform trans = links[(int)(links.size() -1)].solid->getLocalTransform() * UnitSystem::SetTransform(transform);
            multiBody->getLink((int)links.size() - 2).m_collider->setWorldTransform(trans);
        }
        else
        {
            btTransform trans = links[0].solid->getLocalTransform() * UnitSystem::SetTransform(transform);
            multiBody->getBaseCollider()->setWorldTransform(trans);
        }
    }
}

int FeatherstoneEntity::AddRevoluteJoint(unsigned int parent, unsigned int child, const btVector3& pivot, const btVector3& axis, bool collide)
{
    //No self joint possible and base cannot be a child
    if(parent == child || child == 0)
        return -1;
    
    //Check if links exist
    if(parent >= links.size() || child >= links.size())
        return -1;
    
    //Setup joint
    //q' = q2 * q1
    btQuaternion ornParentToChild = getLinkTransform(child).getRotation().inverse() * getLinkTransform(parent).getRotation();
    btVector3 parentComToPivotOffset = UnitSystem::SetPosition(pivot) - getLinkTransform(parent).getOrigin();
    btVector3 pivotToChildComOffset = getLinkTransform(child).getOrigin() - UnitSystem::SetPosition(pivot);
    
    multiBody->setupRevolute(child - 1, links[child].solid->getMass(), links[child].solid->getMomentsOfInertia(), parent - 1,
                             ornParentToChild, quatRotate(ornParentToChild, axis.normalized()), parentComToPivotOffset, pivotToChildComOffset, !collide);
    
    multiBody->finalizeMultiDof();
    
    multiBody->setJointPos(child - 1, btScalar(0.));
    multiBody->setJointVel(child - 1, btScalar(0.));
    
    FeatherstoneJoint joint(btMultibodyLink::eRevolute, parent, child);
    joints.push_back(joint);
    return (int)(joints.size() - 1);
}

int FeatherstoneEntity::AddPrismaticJoint(unsigned int parent, unsigned int child, const btVector3& axis, bool collide)
{
    //No self joint possible and base cannot be a child
    if(parent == child || child == 0)
        return -1;
    
    //Check if links exist
    if(parent >= links.size() || child >= links.size())
        return -1;
    
    //Setup joint
    //q' = q2 * q1
    btQuaternion ornParentToChild = getLinkTransform(child).getRotation().inverse() * getLinkTransform(parent).getRotation();
    btVector3 parentComToChildComOffset = getLinkTransform(child).getOrigin() - getLinkTransform(parent).getOrigin();
    
    //Check if pivot offset is ok!
    multiBody->setupPrismatic(child - 1, links[child].solid->getMass(), links[child].solid->getMomentsOfInertia(), parent - 1,
                              ornParentToChild, quatRotate(ornParentToChild, axis.normalized()), parentComToChildComOffset, btVector3(0.0,0.0,0.0), !collide);
    
    multiBody->finalizeMultiDof();
    
    multiBody->setJointPos(child - 1, btScalar(0.));
    multiBody->setJointVel(child - 1, btScalar(0.));
    
    FeatherstoneJoint joint(btMultibodyLink::ePrismatic, parent, child);
    joints.push_back(joint);
    return (int)(joints.size() - 1);
}

#pragma mark - Methods
void FeatherstoneEntity::DriveJoint(unsigned int index, btScalar forceTorque)
{
    if(index >= joints.size())
        return;
    
    switch (joints[index].type)
    {
        case btMultibodyLink::eRevolute:
            multiBody->addJointTorque(joints[index].child - 1, UnitSystem::SetTorque(forceTorque));
            break;
            
        case btMultibodyLink::ePrismatic:
            multiBody->addJointTorque(joints[index].child - 1, UnitSystem::SetForce(forceTorque));
            break;
            
        default:
            break;
    }
}

void FeatherstoneEntity::ApplyGravity(const btVector3& g)
{
    bool isSleeping = false;
			
    if(multiBody->getBaseCollider() && multiBody->getBaseCollider()->getActivationState() == ISLAND_SLEEPING)
        isSleeping = true;
    
    for(int i=0; i<multiBody->getNumLinks(); ++i)
    {
        if(multiBody->getLink(i).m_collider && multiBody->getLink(i).m_collider->getActivationState() == ISLAND_SLEEPING)
            isSleeping = true;
    } 

    if(!isSleeping)
    {
        multiBody->addBaseForce(g * multiBody->getBaseMass());

        for(int i=0; i<multiBody->getNumLinks(); ++i) 
            multiBody->addLinkForce(i, g *multiBody->getLinkMass(i));
    }
}

void FeatherstoneEntity::ApplyDamping()
{
    for(unsigned int i = 0; i < joints.size(); i++)
    {
        if(joints[i].sigDamping >= SIMD_EPSILON || joints[i].velDamping >= SIMD_EPSILON) //If damping factors not equal zero
        {
            btScalar velocity = multiBody->getJointVel(joints[i].child - 1);
            
            if(btFabs(velocity) >= SIMD_EPSILON) //If velocity higher than zero
            {
                btScalar damping = - velocity/btFabs(velocity) * joints[i].sigDamping - velocity * joints[i].velDamping;
                multiBody->addJointTorque(joints[i].child - 1, damping);
            }
        }
    }
}

void FeatherstoneEntity::RenderStructure()
{
    if(baseRenderable)
    {
        //Draw base coord
        btTransform trans = multiBody->getBaseCollider()->getWorldTransform();
        btScalar openglTrans[16];
        trans.getOpenGLMatrix(openglTrans);
        
        glPushMatrix();
#ifdef BT_USE_DOUBLE_PRECISION
        glMultMatrixd(openglTrans);
#else
        glMultMatrixf(openglTrans);
#endif
        OpenGLSolids::DrawCoordSystem(0.1f);
        glPopMatrix();
    }
    
    //Draw rest of coords
    for(unsigned int i = 0; i < multiBody->getNumLinks(); i++)
    {
        btMultibodyLink& link = multiBody->getLink(i);
        btTransform trans = link.m_collider->getWorldTransform();
        btScalar openglTrans[16];
        trans.getOpenGLMatrix(openglTrans);
        
        glPushMatrix();
#ifdef BT_USE_DOUBLE_PRECISION
        glMultMatrixd(openglTrans);
#else
        glMultMatrixf(openglTrans);
#endif
        OpenGLSolids::DrawCoordSystem(0.1f);
        glPopMatrix();
    }
}