//
//  Manipulator.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 17/05/17.
//  Copyright(c) 2017 Patryk Cieslak. All rights reserved.
//

#include "Manipulator.h"
#include "SimulationApp.h"
#include "FakeRotaryEncoder.h"

Manipulator::Manipulator(std::string uniqueName, unsigned int numOfLinks, SolidEntity* baseLink, const btTransform& geomToJoint) : SystemEntity(uniqueName)
{
	chain = new FeatherstoneEntity(uniqueName + "/FE", numOfLinks+1, baseLink, SimulationApp::getApp()->getSimulationManager()->getDynamicsWorld(), true);
	nTotalLinks = numOfLinks+1;
	nLinks = 1;
    DH.push_back(geomToJoint);
	chain->setSelfCollision(true); //Enable collision between links
	attach = NULL;
}

Manipulator::Manipulator(std::string uniqueName, unsigned int numOfLinks, SolidEntity* baseLink, const btTransform& geomToJoint, FeatherstoneEntity* attachment) : SystemEntity(uniqueName)
{
	chain = new FeatherstoneEntity(uniqueName + "/FE", numOfLinks+1, baseLink, SimulationApp::getApp()->getSimulationManager()->getDynamicsWorld(), false);
	nTotalLinks = numOfLinks+1;
	nLinks = 1;
	DH.push_back(geomToJoint);
	chain->setSelfCollision(true); //Enable collision between links
	attach = attachment;
}

Manipulator::~Manipulator()
{
	delete chain;
}

void Manipulator::AddRotLinkDH(SolidEntity* link, const btTransform& geomToJoint, btScalar d, btScalar a, btScalar alpha, btScalar lowerLimit, btScalar upperLimit)
{
	if(nLinks < nTotalLinks)
	{
		btMultiBodyDynamicsWorld* world = SimulationApp::getApp()->getSimulationManager()->getDynamicsWorld();
		
		//Link connected with parent in joint 
		btTransform trans = DH.back() * geomToJoint.inverse();
		
		//Rotary joint around Z axis
		btVector3 pivot = DH.back().getOrigin();
		
		//Update Featherstone chain
		chain->AddLink(link, trans, world);
		chain->AddRevoluteJoint(nLinks-1, nLinks, pivot, DH.back().getBasis().getColumn(2)); //Revolve always around local Z axis, no collision between joint links
        
        if(lowerLimit < upperLimit)
            chain->AddJointLimit(nLinks-1, lowerLimit, upperLimit);
		chain->AddJointMotor(nLinks-1);
        //chain->setJointDamping(nLinks-1, 0, 0.5);
		desiredPos.push_back(0);
        desiredVel.push_back(0);
        
		//Update trasformation matrix
		btTransform t1(btQuaternion::getIdentity(), btVector3(0,0,d));
		btTransform t2(btQuaternion(btVector3(1,0,0), alpha), btVector3(a,0,0));
		DH.push_back(DH.back() * t1 * t2);
		
		//Link successfully created
		++nLinks;
	}
}

void Manipulator::AddTransformDH(btScalar d, btScalar a, btScalar alpha)
{
    btTransform t1(btQuaternion::getIdentity(), btVector3(0,0,d));
    btTransform t2(btQuaternion(btVector3(1,0,0), alpha), btVector3(a,0,0));
    DH.push_back(DH.back() * t1 * t2);
}

void Manipulator::AddToDynamicsWorld(btMultiBodyDynamicsWorld* world, const btTransform& worldTransform)
{
	chain->AddToDynamicsWorld(world, worldTransform);
	
	if(attach != NULL)
	{
		FixedJoint* fixed = new FixedJoint(getName() + "/Fix", chain, attach);
		fixed->AddToDynamicsWorld(world);
	}
}

void Manipulator::UpdateAcceleration(btScalar dt)
{	
    for(unsigned int i=0; i<nLinks-1; ++i)
    {
        btScalar torque = chain->getJointTorque(i);
        btVector3 torque2;
        btVector3 force;
        chain->getJointFeedback(i, force, torque2);
        
        std::cout << "Joint" << i << ": F:" << force.x() << ", " << force.y() << ", " << force.z()  << " T:" << torque2.x() << ", " << torque2.y() << ", " << torque2.z() << " tau:" << torque << std::endl;
    }
}

void Manipulator::ApplyGravity(const btVector3& g)
{
	chain->ApplyGravity(g);
}

void Manipulator::ApplyDamping()
{
	chain->ApplyDamping();
}

SystemType Manipulator::getSystemType()
{
    return SYSTEM_MANIPULATOR;
}

btTransform Manipulator::getTransform() const
{
	return chain->getMultiBody()->getBaseWorldTransform();
}

std::vector<Renderable> Manipulator::Render()
{
    return chain->Render();
}

void Manipulator::GetAABB(btVector3& min, btVector3& max)
{
	chain->GetAABB(min, max);
}

void Manipulator::SetDesiredJointPosition(unsigned int jointId, btScalar position)
{
	if(jointId >= nLinks-1)
		return;
	
    desiredPos[jointId] = position;
    chain->MotorPositionSetpoint(jointId, position, btScalar(0.1));
}

void Manipulator::SetDesiredJointVelocity(unsigned int jointId, btScalar velocity)
{
	if(jointId >= nLinks-1)
		return;
	
    desiredVel[jointId] = velocity;
    chain->MotorVelocitySetpoint(jointId, velocity, btScalar(1.0));
}

btScalar Manipulator::GetJointPosition(unsigned int jointId)
{
	if(jointId >= nLinks-1)
		return btScalar(0);

    btScalar pos;
    btMultibodyLink::eFeatherstoneJointType link;
    chain->getJointPosition(jointId, pos, link);    
    return pos; 
}

btScalar Manipulator::GetJointTorque(unsigned int jointId)
{
    return btScalar(0);
}

btScalar Manipulator::GetDesiredJointPosition(unsigned int jointId)
{
	if(jointId >= nLinks-1)
		return btScalar(0);
		
    return desiredPos[jointId];
}

btScalar Manipulator::GetDesiredJointVelocity(unsigned int jointId)
{
	if(jointId >= nLinks-1)
		return btScalar(0);
		
    return desiredVel[jointId];
}

const std::vector<btTransform>& Manipulator::getDH()
{
    return DH;
}