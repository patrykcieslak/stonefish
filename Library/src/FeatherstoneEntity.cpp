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
    
    multiBody = new btMultiBody(totalNumOfLinks - 1, baseSolid->getMass(), baseSolid->getMomentsOfInertia(), fixedBase, false);
    btTransform trans = baseSolid->getLocalTransform() * transform;
    multiBody->setBasePos(trans.getOrigin());
    multiBody->setWorldToBaseRot(trans.getRotation());
    
    multiBody->setBaseVel(btVector3(0.,0.,0.));
    multiBody->setBaseOmega(btVector3(0.,0.,0.));
    multiBody->setUseGyroTerm(true);
    multiBody->setAngularDamping(0.0);
    multiBody->setLinearDamping(0.0);
    multiBody->useRK4Integration(true);
    
    AddLink(baseSolid, transform, world);
}

#pragma mark - Destructor
FeatherstoneEntity::~FeatherstoneEntity()
{
    multiBody = NULL;
    links.clear();
}

#pragma mark - Entity
EntityType FeatherstoneEntity::getType()
{
    return ENTITY_FEATHERSTONE;
}

void FeatherstoneEntity::GetAABB(btVector3& min, btVector3& max)
{
    
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
    world->addMultiBody(multiBody);
}

#pragma mark - Featherstone
void FeatherstoneEntity::AddLink(SolidEntity *solid, const btTransform& transform, btMultiBodyDynamicsWorld* world)
{
    if(solid != NULL)
    {
        links.push_back(FeatherstoneLink(solid, transform));
        links[(int)(links.size() - 1)].solid->BuildMultibodyLinkCollider(multiBody, (int)(links.size() - 1), world);
        
        if(links.size() > 1)
        {
            btTransform trans = links[(int)(links.size() -1)].solid->getLocalTransform() * transform;
            multiBody->getLink((int)links.size() - 2).m_collider->setWorldTransform(trans);
        }
        else
        {
            btTransform trans = links[0].solid->getLocalTransform() * transform;
            multiBody->getBaseCollider()->setWorldTransform(trans);
        }
    }
}

void FeatherstoneEntity::AddRevoluteJoint(unsigned int parent, unsigned int child, const btVector3& pivot, const btVector3& axis, bool collide)
{
    //No self joint possible and base cannot be a child
    if(parent == child || child == 0)
        return;
    
    //Check if links exist
    if(parent >= links.size() || child >= links.size())
        return;
    
    //Setup joint
    btQuaternion ornParentToChild = links[parent].transform.getRotation().inverse() * links[child].transform.getRotation();
    btVector3 parentComToChildPivotOffset = (links[parent].solid->getLocalTransform() * links[parent].transform).inverse() * pivot;
    btVector3 childPivotToChildComOffset = (links[child].solid->getLocalTransform() * links[child].transform).inverse() * pivot;
    
    multiBody->setupRevolute(child - 1, links[child].solid->getMass(), links[child].solid->getMomentsOfInertia(),
                             parent - 1, ornParentToChild, quatRotate(ornParentToChild, axis), parentComToChildPivotOffset, childPivotToChildComOffset, !collide);
    
    btTransform tr;
    tr.setRotation(multiBody->getParentToLocalRot(child -1));
    tr.setOrigin(links[child].solid->getLocalTransform() * multiBody->getRVector(child - 1));
    
    multiBody->getLink(child - 1).m_collider->setWorldTransform(tr);
    
    //multiBody->setJointPos(child - 1, btScalar(0.));
    //multiBody->setJointVel(child - 1, btScalar(0.));
}

void FeatherstoneEntity::AddPrismaticJoint(unsigned int linkA, unsigned int linkB, const btVector3& pivot, const btVector3& axis, bool collide)
{
}

void FeatherstoneEntity::AddCollider(SolidEntity *solid)
{
}

void FeatherstoneEntity::AddCollider(StaticEntity *stat)
{
}

void FeatherstoneEntity::DriveJoint(unsigned int child, btScalar forceTorque)
{
    if(child == 0)
        return;
    
    switch (multiBody->getLink(child - 1).m_jointType)
    {
        case btMultibodyLink::eRevolute:
            multiBody->addJointTorque(child - 1, UnitSystem::SetTorque(btVector3(forceTorque, 0., 0.)).x());
            break;
            
        case btMultibodyLink::ePrismatic:
            multiBody->addJointTorque(child - 1, UnitSystem::SetForce(btVector3(forceTorque, 0., 0.)).x());
            break;
            
        default:
            break;
    }
}

void FeatherstoneEntity::setBaseRenderable(bool render)
{
    baseRenderable = render;
}

void FeatherstoneEntity::setJointIC(unsigned int child, btScalar position, btScalar velocity)
{
    if(child == 0)
        return;
    
    switch (multiBody->getLink(child - 1).m_jointType)
    {
        case btMultibodyLink::eRevolute:
            multiBody->setJointPos(child - 1, UnitSystem::SetAngle(position));
            multiBody->setJointVel(child - 1, UnitSystem::SetAngle(velocity));
            break;
            
        case btMultibodyLink::ePrismatic:
            multiBody->setJointPos(child - 1, UnitSystem::SetLength(position));
            multiBody->setJointPos(child - 1, UnitSystem::SetLength(velocity));
            break;
            
        default:
            break;
    }
}

void FeatherstoneEntity::getJointPosition(unsigned int child, btScalar &position, btMultibodyLink::eFeatherstoneJointType &jointType)
{
    if(child == 0)
    {
        jointType = btMultibodyLink::eInvalid;
        position = btScalar(0.);
    }
    else
    {
        jointType = multiBody->getLink(child - 1).m_jointType;
        
        switch (jointType)
        {
            case btMultibodyLink::eRevolute:
                position = UnitSystem::GetAngle(multiBody->getJointPos(child - 1));
                break;
                
            case btMultibodyLink::ePrismatic:
                position = UnitSystem::GetLength(multiBody->getJointPos(child - 1));
                break;
                
            default:
                break;
        }
    }
}

void FeatherstoneEntity::getJointVelocity(unsigned int child, btScalar &velocity, btMultibodyLink::eFeatherstoneJointType &jointType)
{
    if(child == 0)
    {
        jointType = btMultibodyLink::eInvalid;
        velocity = btScalar(0.);
    }
    else
    {
        jointType = multiBody->getLink(child - 1).m_jointType;
        
        switch (jointType)
        {
            case btMultibodyLink::eRevolute:
                velocity = UnitSystem::GetAngle(multiBody->getJointVel(child - 1));
                break;
                
            case btMultibodyLink::ePrismatic:
                velocity = UnitSystem::GetLength(multiBody->getJointVel(child - 1));
                break;
                
            default:
                break;
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