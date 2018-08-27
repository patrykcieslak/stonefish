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
    nJoints = 0;
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

void Manipulator::AddRotLinkDH(std::string jointName, SolidEntity* link, const btTransform& geomToJoint, btScalar d, btScalar a, btScalar alpha, btScalar lowerLimit, btScalar upperLimit, btScalar maxTorque)
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
		chain->AddRevoluteJoint(jointName, nLinks-1, nLinks, pivot, DH.back().getBasis().getColumn(2)); //Revolve always around local Z axis, no collision between joint links
        
        if(lowerLimit < upperLimit)
            chain->AddJointLimit(nLinks-1, lowerLimit, upperLimit);
		
        chain->AddJointMotor(nLinks-1, maxTorque/SimulationApp::getApp()->getSimulationManager()->getStepsPerSecond());
        //chain->setJointDamping(nLinks-1, 0, 0.5);
		
        desiredPos.push_back(0);
        desiredVel.push_back(0);
        motorTorque.push_back(0);
        
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

void Manipulator::AddRotLinkURDF(std::string jointName, SolidEntity* link, const btTransform& trans, const btVector3& axis, btScalar lowerLimit, btScalar upperLimit, btScalar maxTorque)
{
    if(nLinks < nTotalLinks)
    {
        btMultiBodyDynamicsWorld* world = SimulationApp::getApp()->getSimulationManager()->getDynamicsWorld();
        
        btTransform linkTrans = DH.back() * trans;
        chain->AddLink(link, linkTrans, world);
		chain->AddRevoluteJoint(jointName, nLinks-1, nLinks, linkTrans.getOrigin(), linkTrans.getBasis() * axis);
        
        if(lowerLimit < upperLimit)
            chain->AddJointLimit(nLinks-1, lowerLimit, upperLimit);
		
        chain->AddJointMotor(nLinks-1, maxTorque/SimulationApp::getApp()->getSimulationManager()->getStepsPerSecond());
        //chain->setJointDamping(nLinks-1, 0, 0.5);
		
        desiredPos.push_back(0);
        desiredVel.push_back(0);
        motorTorque.push_back(0);
        
		//Update trasformation matrix
		DH.push_back(linkTrans);
        
		//Link successfully created
		++nLinks;
	}
}

void Manipulator::AddToDynamicsWorld(btMultiBodyDynamicsWorld* world, const btTransform& worldTransform)
{
	chain->AddToDynamicsWorld(world, worldTransform);
	
	if(attach != NULL)
	{
		FixedJoint* fixed = new FixedJoint(getName() + "/Fix", chain, attach, -1, -1, chain->getLinkTransform(0).getOrigin());
		fixed->AddToDynamicsWorld(world);
	}
}

void Manipulator::UpdateAcceleration(btScalar dt)
{	
	chain->UpdateAcceleration(dt);
}

void Manipulator::UpdateSensors(btScalar dt)
{
    for(unsigned int i=0; i<nLinks-1; ++i)
        motorTorque[i] = chain->getMotorImpulse(i)/dt;
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
    chain->MotorVelocitySetpoint(jointId, btScalar(0), btScalar(1.0));
}

void Manipulator::SetDesiredJointVelocity(unsigned int jointId, btScalar velocity)
{
	if(jointId >= nLinks-1)
		return;
	
    desiredVel[jointId] = velocity;
    chain->MotorVelocitySetpoint(jointId, velocity, btScalar(1.0));
    chain->MotorPositionSetpoint(jointId, btScalar(0), btScalar(0));
}

std::string Manipulator::getJointName(unsigned int jointId)
{
    return chain->getJointName(jointId);
}

btScalar Manipulator::getJointPosition(unsigned int jointId)
{
    btScalar pos;
    btMultibodyLink::eFeatherstoneJointType link;
    chain->getJointPosition(jointId, pos, link);    
    return pos; 
}

btScalar Manipulator::getJointVelocity(unsigned int jointId)
{
    btScalar pos;
    btMultibodyLink::eFeatherstoneJointType link;
    chain->getJointVelocity(jointId, pos, link);    
    return pos; 
}

btScalar Manipulator::getJointTorque(unsigned int jointId)
{
    return btScalar(0);
}

btScalar Manipulator::getDesiredJointPosition(unsigned int jointId)
{
	if(jointId >= nLinks-1)
		return btScalar(0);
		
    return desiredPos[jointId];
}

btScalar Manipulator::getDesiredJointVelocity(unsigned int jointId)
{
	if(jointId >= nLinks-1)
		return btScalar(0);
		
    return desiredVel[jointId];
}

const std::vector<btTransform>& Manipulator::getDH()
{
    return DH;
}

FeatherstoneEntity* Manipulator::getChain()
{
    return chain;
}

unsigned int Manipulator::getNumOfLinks()
{
    return nLinks;
}

unsigned int Manipulator::getNumOfJoints()
{
    return desiredPos.size();
}