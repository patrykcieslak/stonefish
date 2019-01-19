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
#include "entities/Entity.h"

namespace sf
{

    class SolidEntity;
    class StaticEntity;
    
struct FeatherstoneLink
{
    FeatherstoneLink(SolidEntity* s, const Transform& t) : solid(s), trans(t) {}
    
    SolidEntity* solid;
    Transform trans;
};

struct FeatherstoneJoint
{
    FeatherstoneJoint(std::string n, btMultibodyLink::eFeatherstoneJointType t, unsigned int p, unsigned int c)
                        : name(n), type(t), feedback(NULL), limit(NULL), motor(NULL), parent(p), child(c), sigDamping(Scalar(0)), velDamping(Scalar(0))  {}
    
    std::string name;
    btMultibodyLink::eFeatherstoneJointType type;
    btMultiBodyJointFeedback* feedback;
    btMultiBodyJointLimitConstraint* limit;
    btMultiBodyJointMotor* motor;
    
    unsigned int parent;
    unsigned int child;
    Vector3 axisInChild;
    Vector3 pivotInChild;
    Scalar sigDamping;
    Scalar velDamping;
};

//! Featherstone multi-body dynamics class.
/*!
	Class implements simplified creation of multi-body trees, using Roy Featherstone algorithm. 
 */
class FeatherstoneEntity : public Entity
{
public:
    FeatherstoneEntity(std::string uniqueName, unsigned int totalNumOfLinks, SolidEntity* baseSolid, bool fixedBase = false);
    virtual ~FeatherstoneEntity();
    
    //Multibody definition
    void AddLink(SolidEntity* solid, const Transform& transform);
    int AddRevoluteJoint(std::string name, unsigned int parent, unsigned int child, const Vector3& pivot, const Vector3& axis, bool collisionBetweenJointLinks = false);
    int AddPrismaticJoint(std::string name, unsigned int parent, unsigned int child, const Vector3& axis, bool collisionBetweenJointLinks = false);
	int AddFixedJoint(std::string name, unsigned int parent, unsigned int child, const Vector3& pivot);
    void AddJointMotor(unsigned int index, Scalar maxForceTorque);
    void AddJointLimit(unsigned int index, Scalar lower, Scalar upper);
	
    //Multibody control
    void MotorPositionSetpoint(unsigned int index, Scalar pos, Scalar kp);
    void MotorVelocitySetpoint(unsigned int index, Scalar vel, Scalar kd);
    void DriveJoint(unsigned int index, Scalar forceTorque);
    void ApplyGravity(const Vector3& g);
    void ApplyDamping();
    void AddLinkForce(unsigned int index, const Vector3& F);
    void AddLinkTorque(unsigned int index, const Vector3& tau);
	void UpdateAcceleration(Scalar dt);
    
    //Joints
    void setJointIC(unsigned int index, Scalar position, Scalar velocity);
    void setJointDamping(unsigned int  index, Scalar constantFactor, Scalar viscousFactor);
    std::string getJointName(unsigned int index);
    void getJointPosition(unsigned int index, Scalar& position, btMultibodyLink::eFeatherstoneJointType& jointType);
    void getJointVelocity(unsigned int index, Scalar& velocity, btMultibodyLink::eFeatherstoneJointType& jointType);
    Scalar getJointTorque(unsigned int index); //Only shows sum of manually applied torques
    Scalar getMotorForceTorque(unsigned int index); 
    Vector3 getJointAxis(unsigned int index);
    
    /*
     * Both vectors are in the CoG frame of the child link. 
     * The force is equal to the sum of reaction forces acting on the CoG.
     * The torque is calculated as a cross product of a vector from CoG to joint pivot and the force defined above. 
     * The force acting on every link causes a reaction force and a torque on this link. 
     * If the rection torque acts around the axis of the joint, then this torque does not transfer directly to previous joints (only force is transferred)
    */
    unsigned int getJointFeedback(unsigned int index, Vector3& force, Vector3& torque);
    
	//Links
	void setBaseTransform(const Transform& trans);
    void setBaseRenderable(bool render);
    FeatherstoneLink getLink(unsigned int index);
    Transform getLinkTransform(unsigned int index);
    Vector3 getLinkLinearVelocity(unsigned int index);
    Vector3 getLinkAngularVelocity(unsigned int index);
    unsigned int getNumOfJoints();
    unsigned int getNumOfMovingJoints();
    unsigned int getNumOfLinks();
	btMultiBody* getMultiBody();
    void setSelfCollision(bool enabled);
    
    //Common
    void AddToSimulation(SimulationManager* sm);
    void AddToSimulation(SimulationManager* sm, const Transform& origin);
    std::vector<Renderable> Render();
    void getAABB(Vector3& min, Vector3& max);
    EntityType getType();
    
private:
    btMultiBody* multiBody;
    std::vector<FeatherstoneLink> links;
    std::vector<FeatherstoneJoint> joints;
	bool baseRenderable;
};
    
}

#endif
