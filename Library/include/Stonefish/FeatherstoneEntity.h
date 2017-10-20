//
//  FeatherstoneEntity.h
//  Stonefish
//
//  Created by Patryk Cieslak on 8/20/13.
//  Copyright(c) 2013-2017 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_FeatherstoneEntity__
#define __Stonefish_FeatherstoneEntity__

#include <BulletDynamics/Featherstone/btMultiBody.h>
#include <BulletDynamics/Featherstone/btMultiBodyLinkCollider.h>
#include <BulletDynamics/Featherstone/btMultiBodyLink.h>
#include <BulletDynamics/Featherstone/btMultiBodyJointLimitConstraint.h>
#include <BulletDynamics/Featherstone/btMultiBodyJointFeedback.h>
#include <BulletDynamics/Featherstone/btMultiBodyJointMotor.h>
#include "Entity.h"
#include "SolidEntity.h"
#include "StaticEntity.h"

struct FeatherstoneLink
{
    FeatherstoneLink(SolidEntity* s, const btTransform& t) : solid(s), trans(t) {}
    
    SolidEntity* solid;
    btTransform trans;
};

struct FeatherstoneJoint
{
    FeatherstoneJoint(btMultibodyLink::eFeatherstoneJointType t, unsigned int p, unsigned int c)
                        : type(t), parent(p), child(c), sigDamping(btScalar(0.)), velDamping(btScalar(0.)) {}
    
    btMultibodyLink::eFeatherstoneJointType type;
    unsigned int parent;
    unsigned int child;
    btScalar sigDamping;
    btScalar velDamping;
};

//! Featherstone multi-body dynamics class.
/*!
	Class implements simplified creation of multi-body trees, using Roy Featherstone algorithm. 
 */
class FeatherstoneEntity : public Entity
{
public:
    FeatherstoneEntity(std::string uniqueName, unsigned int totalNumOfLinks, SolidEntity* baseSolid, btMultiBodyDynamicsWorld* world, bool fixedBase = false);
    virtual ~FeatherstoneEntity();
    
    void AddLink(SolidEntity* solid, const btTransform& transform, btMultiBodyDynamicsWorld* world);
    int AddRevoluteJoint(unsigned int parent, unsigned int child, const btVector3& pivot, const btVector3& axis, bool collisionBetweenJointLinks = false);
    int AddPrismaticJoint(unsigned int parent, unsigned int child, const btVector3& axis, bool collisionBetweenJointLinks = false);
	int AddFixedJoint(unsigned int parent, unsigned int child);
	void EnableSelfCollision();
	void DisableSelfCollision();
  
    void DriveJoint(unsigned int index, btScalar forceTorque);
    void ApplyGravity(const btVector3& g);
    void ApplyDamping();
    void AddLinkForce(unsigned int index, const btVector3& F);
    void AddLinkTorque(unsigned int index, const btVector3& tau);
    
	void setBaseTransform(const btTransform& trans);
    void setBaseRenderable(bool render);
    void setJointIC(unsigned int index, btScalar position, btScalar velocity);
    void setJointDamping(unsigned int  index, btScalar constantFactor, btScalar viscousFactor);
    void getJointPosition(unsigned int index, btScalar& position, btMultibodyLink::eFeatherstoneJointType& jointType);
    void getJointVelocity(unsigned int index, btScalar& velocity, btMultibodyLink::eFeatherstoneJointType& jointType);
	void getJointFeedback(unsigned int index, btVector3& force, btVector3& torque);
	FeatherstoneLink getLink(unsigned int index);
    btTransform getLinkTransform(unsigned int index);
    btVector3 getLinkLinearVelocity(unsigned int index);
    btVector3 getLinkAngularVelocity(unsigned int index);
    unsigned int getNumOfJoints();
    unsigned int getNumOfLinks();
	btMultiBody* getMultiBody();
    
    void AddToDynamicsWorld(btMultiBodyDynamicsWorld* world);
    void AddToDynamicsWorld(btMultiBodyDynamicsWorld* world, const btTransform& worldTransform);
    
    std::vector<Renderable> Render();
    EntityType getType();
    void GetAABB(btVector3& min, btVector3& max);
    
private:
    btMultiBody* multiBody;
    std::vector<FeatherstoneLink> links;
    std::vector<FeatherstoneJoint> joints;
	btAlignedObjectArray<btMultiBodyJointFeedback*> jointFeedbacks;
    bool baseRenderable;
};

#endif