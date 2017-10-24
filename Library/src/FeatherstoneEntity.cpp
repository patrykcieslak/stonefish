//
//  FeatherstoneEntity.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 8/20/13.
//  Copyright (c) 2013-2017 Patryk Cieslak. All rights reserved.
//

#include "FeatherstoneEntity.h"
#include "MathsUtil.hpp"

FeatherstoneEntity::FeatherstoneEntity(std::string uniqueName, unsigned int totalNumOfLinks, SolidEntity* baseSolid, btMultiBodyDynamicsWorld* world, bool fixedBase) : Entity(uniqueName)
{
    //baseRenderable = fixedBase ? false : true;
    baseRenderable = true;
	
    multiBody = new btMultiBody(totalNumOfLinks - 1, baseSolid->getMass(), baseSolid->getMomentsOfInertia(), fixedBase, true);
    
    multiBody->setBaseWorldTransform(btTransform::getIdentity());
    multiBody->setAngularDamping(0.0);
    multiBody->setLinearDamping(0.0);
    multiBody->setMaxAppliedImpulse(100.0);
    multiBody->setMaxCoordinateVelocity(10000.0);
    multiBody->useRK4Integration(false); //Enabling RK4 causes unreallistic energy accumulation (strange motions in 0 gravity)
    multiBody->useGlobalVelocities(false); //See previous comment
    multiBody->setHasSelfCollision(false); //No self collision by default
    multiBody->setUseGyroTerm(true);
    multiBody->setCanSleep(true);
   
    AddLink(baseSolid, btTransform::getIdentity(), world);
	multiBody->finalizeMultiDof();
}

FeatherstoneEntity::~FeatherstoneEntity()
{
    multiBody = NULL;
    
    for(int i=0; i<links.size(); i++)
        delete links[i].solid;
    
    links.clear();
    joints.clear();
	jointFeedbacks.clear();
}

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

void FeatherstoneEntity::AddToDynamicsWorld(btMultiBodyDynamicsWorld* world)
{
    AddToDynamicsWorld(world, btTransform::getIdentity());
}

void FeatherstoneEntity::AddToDynamicsWorld(btMultiBodyDynamicsWorld* world, const btTransform& worldTransform)
{
    setBaseTransform(UnitSystem::SetTransform(worldTransform));
    multiBody->setBaseVel(btVector3(0,0,0));
    multiBody->setBaseOmega(btVector3(0,0,0));
    world->addMultiBody(multiBody);
    
    for(unsigned int i=0; i<jointLimits.size(); ++i)
        world->addMultiBodyConstraint(jointLimits[i]);
        
    for(unsigned int i=0; i<jointMotors.size(); ++i)
        world->addMultiBodyConstraint(jointMotors[i]);
}

void FeatherstoneEntity::EnableSelfCollision()
{
	multiBody->setHasSelfCollision(true);
}

void FeatherstoneEntity::DisableSelfCollision()
{
	multiBody->setHasSelfCollision(false);
}

void FeatherstoneEntity::setBaseRenderable(bool render)
{
    baseRenderable = render;
}

void FeatherstoneEntity::setBaseTransform(const btTransform& trans)
{
	btTransform T0 = trans * links[0].solid->getGeomToCOGTransform(); 
    multiBody->getBaseCollider()->setWorldTransform(T0);
	multiBody->setBaseWorldTransform(T0);
    
	for(int i=1; i<links.size(); ++i)
	{
		btTransform tr = links[i].solid->getMultibodyLinkCollider()->getWorldTransform();
		links[i].solid->getMultibodyLinkCollider()->setWorldTransform(trans * tr);
	}
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

void FeatherstoneEntity::getJointFeedback(unsigned int index, btVector3& force, btVector3& torque)
{
	if(index >= joints.size())
	{
		force.setZero();
		torque.setZero();
	}
	else
	{
		force = btVector3(jointFeedbacks[index]->m_reactionForces.m_topVec[0],
						  jointFeedbacks[index]->m_reactionForces.m_topVec[1],
						  jointFeedbacks[index]->m_reactionForces.m_topVec[2]);
						  
		torque = btVector3(jointFeedbacks[index]->m_reactionForces.m_bottomVec[0],
						   jointFeedbacks[index]->m_reactionForces.m_bottomVec[1],
						   jointFeedbacks[index]->m_reactionForces.m_bottomVec[2]);
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

btTransform FeatherstoneEntity::getLinkTransform(unsigned int index)
{
    if(index >= links.size())
        return btTransform::getIdentity();
    
    if(index == 0)
        return multiBody->getBaseWorldTransform();
    else
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

void FeatherstoneEntity::AddLink(SolidEntity *solid, const btTransform& transform, btMultiBodyDynamicsWorld* world)
{
    if(solid != NULL)
    {
        //Add link
        links.push_back(FeatherstoneLink(solid, UnitSystem::SetTransform(transform)));
        //Build collider
        links.back().solid->BuildMultibodyLinkCollider(multiBody, (int)(links.size() - 1), world);
        
        if(links.size() > 1) //If not base link
        {
            btTransform trans =  UnitSystem::SetTransform(transform) * links[links.size()-1].solid->getGeomToCOGTransform();
            links.back().solid->setTransform(trans);
            
			btMultiBodyJointFeedback* fb = new btMultiBodyJointFeedback();
			multiBody->getLink((int)links.size() - 2).m_jointFeedback = fb;
			jointFeedbacks.push_back(fb);
        }
        else
        {
            btTransform trans = UnitSystem::SetTransform(transform) * links[0].solid->getGeomToCOGTransform();
            links[0].solid->setTransform(trans);
            multiBody->setBaseWorldTransform(trans);
        }
    }
}

int FeatherstoneEntity::AddRevoluteJoint(unsigned int parent, unsigned int child, const btVector3& pivot, const btVector3& axis, bool collisionBetweenJointLinks)
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
    btVector3 parentComToPivotOffset = getLinkTransform(parent).getBasis().inverse() * (UnitSystem::SetPosition(pivot) - getLinkTransform(parent).getOrigin());
    btVector3 pivotToChildComOffset =  getLinkTransform(child).getBasis().inverse() * (getLinkTransform(child).getOrigin() - UnitSystem::SetPosition(pivot));
    
    multiBody->setupRevolute(child - 1, links[child].solid->getMass(), links[child].solid->getMomentsOfInertia(), parent - 1,
                             ornParentToChild, getLinkTransform(child).getBasis().inverse() * axis.normalized() , parentComToPivotOffset, pivotToChildComOffset, !collisionBetweenJointLinks);
    
    multiBody->finalizeMultiDof();
    
    multiBody->setJointPos(child - 1, btScalar(0.));
    multiBody->setJointVel(child - 1, btScalar(0.));
    
    FeatherstoneJoint joint(btMultibodyLink::eRevolute, parent, child);
    joints.push_back(joint);
    return ((int)joints.size() - 1);
}

int FeatherstoneEntity::AddPrismaticJoint(unsigned int parent, unsigned int child, const btVector3& axis, bool collisionBetweenJointLinks)
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
    btVector3 parentComToPivotOffset = btVector3(0,0,0);
    btVector3 pivotToChildComOffset = getLinkTransform(child).getBasis().inverse() * (getLinkTransform(child).getOrigin()-getLinkTransform(parent).getOrigin());
    
    //Check if pivot offset is ok!
    multiBody->setupPrismatic(child - 1, links[child].solid->getMass(), links[child].solid->getMomentsOfInertia(), parent - 1,
                              ornParentToChild, getLinkTransform(child).getBasis().inverse() * axis.normalized(), parentComToPivotOffset, pivotToChildComOffset, !collisionBetweenJointLinks);
    
    multiBody->finalizeMultiDof();
    
    multiBody->setJointPos(child - 1, btScalar(0.));
    multiBody->setJointVel(child - 1, btScalar(0.));
    
    FeatherstoneJoint joint(btMultibodyLink::ePrismatic, parent, child);
    joints.push_back(joint);
    return ((int)joints.size() - 1);
}

int FeatherstoneEntity::AddFixedJoint(unsigned int parent, unsigned int child)
{
	//No self joint possible and base cannot be a child
	if(parent == child || child == 0)
        return -1;
    
    //Check if links exist
    if(parent >= links.size() || child >= links.size())
        return -1;
    
	//Setup joint
	btQuaternion ornParentToChild =  getLinkTransform(child).getRotation().inverse() * getLinkTransform(parent).getRotation();
	btVector3 parentComToPivotOffset = btVector3(0,0,0);
    btVector3 pivotToChildComOffset = getLinkTransform(child).getBasis().inverse() * (getLinkTransform(child).getOrigin()-getLinkTransform(parent).getOrigin());
    
	multiBody->setupFixed(child - 1, links[child].solid->getMass(), links[child].solid->getMomentsOfInertia(), parent - 1,
						  ornParentToChild, parentComToPivotOffset, pivotToChildComOffset);

	multiBody->finalizeMultiDof();
	
	FeatherstoneJoint joint(btMultibodyLink::eFixed, parent, child);
	joints.push_back(joint);
	return ((int)joints.size() - 1);
}

void FeatherstoneEntity::AddJointLimit(unsigned int index, btScalar lower, btScalar upper)
{
    if(index >= joints.size())
        return;
        
    btMultiBodyJointLimitConstraint* jlc = new btMultiBodyJointLimitConstraint(multiBody, index, lower, upper);
    jointLimits.push_back(jlc);
}

void FeatherstoneEntity::AddJointMotor(unsigned int index)
{
    if(index >= joints.size())
        return;
    
    btMultiBodyJointMotor* jmc = new btMultiBodyJointMotor(multiBody, index, btScalar(0.), 1.0);
    jointMotors.push_back(jmc);
}

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
		{
            isSleeping = true;
			break;
		}
    } 

    if(!isSleeping)
    {
        multiBody->addBaseForce(g * multiBody->getBaseMass());

        for(int i=0; i<multiBody->getNumLinks(); ++i) 
		{
            multiBody->addLinkForce(i, g *multiBody->getLinkMass(i));
		}
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

void FeatherstoneEntity::AddLinkForce(unsigned int index, const btVector3& F)
{
    if(index >= links.size())
        return;
    
    if(index == 0)
        multiBody->addBaseForce(F);
    else
        multiBody->addLinkForce(index-1, F);
}
 
void FeatherstoneEntity::AddLinkTorque(unsigned int index, const btVector3& tau)
{
    if(index >= links.size())
        return;
        
    if(index == 0)
        multiBody->addBaseTorque(tau);
    else
        multiBody->addLinkTorque(index-1, tau);
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
    for(unsigned int i = 0; i < multiBody->getNumLinks(); ++i)
    {
		std::vector<Renderable> _link = links[i+1].solid->Render();
		items.insert(items.end(), _link.begin(), _link.end());
    }
	
	return items;
}