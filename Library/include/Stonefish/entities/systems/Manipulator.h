//
//  Manipulator.h
//  Stonefish
//
//  Created by Patryk Cieslak on 17/05/17.
//  Copyright(c) 2017 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_Manipulator__
#define __Stonefish_Manipulator__

#include <entities/SystemEntity.h>
#include <entities/FeatherstoneEntity.h>
#include <joints/FixedJoint.h>
#include <actuators/Motor.h>
#include <sensors/RotaryEncoder.h>
#include <controllers/ServoController.h>

//! A dynamical model of an electric manipulator
/*! 
 * This class implements a dynamical model of a rigid body manipulator. 
 * The manipulator can be controlled in position or velocity. It has force feedback on all joints.
 * It can be equipped with a gripper (Gripper class). 
 * The model is constructed from solids, using the Denavit-Hartenberg notation.
 */
class Manipulator : public SystemEntity
{
public:
    Manipulator(std::string uniqueName, unsigned int numOfLinks, SolidEntity* baseLink, const btTransform& geomToJoint); //Fixed base maniupulator
	Manipulator(std::string uniqueName, unsigned int numOfLinks, SolidEntity* baseLink, const btTransform& geomToJoint, FeatherstoneEntity* attachment); //Attached to base of multibody
    virtual ~Manipulator();
    
	//Manipulator
	//Denavit-Hartenberg notation
    void AddRotLinkDH(std::string jointName, SolidEntity* link, const btTransform& geomToJoint, btScalar d, btScalar a, btScalar alpha, 
                      btScalar lowerLimit = btScalar(1.0), btScalar upperLimit = btScalar(-1.0), btScalar maxTorque = btScalar(1000.0));
    void AddTransformDH(btScalar d, btScalar a, btScalar alpha);
    //URDF format
    void AddRotLinkURDF(std::string jointName, SolidEntity* link, const btTransform& trans, const btVector3& axis, 
                         btScalar lowerLimit = btScalar(1), btScalar upperLimit = btScalar(-1), btScalar maxTorque = btScalar(1000.0));
    
	void SetDesiredJointPosition(unsigned int jointId, btScalar position);
    void SetDesiredJointVelocity(unsigned int jointId, btScalar velocity);
    
    std::string getJointName(unsigned int jointId);
    btScalar getJointPosition(unsigned int jointId);
    btScalar getJointVelocity(unsigned int jointId);
    btScalar getJointTorque(unsigned int jointId);
    btScalar getDesiredJointPosition(unsigned int jointId);
    btScalar getDesiredJointVelocity(unsigned int jointId);
    const std::vector<btTransform>& getDH();
    FeatherstoneEntity* getChain();
    unsigned int getNumOfLinks();
    unsigned int getNumOfJoints();
    
	//System
	void AddToDynamicsWorld(btMultiBodyDynamicsWorld* world, const btTransform& worldTransform);
	void GetAABB(btVector3& min, btVector3& max);
    void UpdateAcceleration(btScalar dt);
    void UpdateSensors(btScalar dt);
    void ApplyGravity(const btVector3& g);
	void ApplyDamping();
    SystemType getSystemType();
    
    //Entity
    virtual std::vector<Renderable> Render();
    btTransform getTransform() const;
    
private:
    FeatherstoneEntity* chain;
	std::vector<btScalar> desiredPos;
    std::vector<btScalar> desiredVel;
    std::vector<btScalar> motorTorque;
	unsigned int nLinks;
    unsigned int nJoints;
	unsigned int nTotalLinks;
	
	std::vector<btTransform> DH; //Succesive transforms from DH notation (multiplied) 
	FeatherstoneEntity* attach;
};

#endif
