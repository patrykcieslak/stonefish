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

Manipulator::Manipulator(std::string uniqueName, unsigned int numOfLinks, SolidEntity* baseLink, const btTransform& worldTrans) : SystemEntity(uniqueName)
{
	chain = new FeatherstoneEntity(uniqueName + "/FE", numOfLinks+1, baseLink, worldTrans, SimulationApp::getApp()->getSimulationManager()->getDynamicsWorld(), true);
	nTotalLinks = numOfLinks+1;
	nLinks = 1;
	lastDHTrans = worldTrans;
	chain->EnableSelfCollision(); //Enable collision between links
	attach = NULL;
}

Manipulator::Manipulator(std::string uniqueName, unsigned int numOfLinks, SolidEntity* baseLink, const btTransform& worldTrans, btRigidBody* attachment) : SystemEntity(uniqueName)
{
	chain = new FeatherstoneEntity(uniqueName + "/FE", numOfLinks+1, baseLink, worldTrans, SimulationApp::getApp()->getSimulationManager()->getDynamicsWorld(), false);
	nTotalLinks = numOfLinks+1;
	nLinks = 1;
	lastDHTrans = worldTrans;
	chain->EnableSelfCollision(); //Enable collision between links
	attach = attachment;
}

Manipulator::~Manipulator()
{
	delete chain;
	
	for(unsigned int i=0; i<nLinks-1; ++i)
	{
		delete motors[i];
		delete encoders[i];
		delete controllers[i];
	}
	motors.clear();
	encoders.clear();
	controllers.clear();
}

void Manipulator::AddRotLinkDH(SolidEntity* link, const btTransform& geomToJoint, btScalar d, btScalar a, btScalar alpha)
{
	if(nLinks < nTotalLinks)
	{
		btMultiBodyDynamicsWorld* world = SimulationApp::getApp()->getSimulationManager()->getDynamicsWorld();
		
		//Link connected with parent in joint 
		btTransform trans = lastDHTrans * geomToJoint.inverse() * link->getLocalTransform();
		
		//Rotary joint around Z axis
		btVector3 pivot = lastDHTrans.getOrigin();
		
		//Update Featherstone chain
		chain->AddLink(link, trans, world);
		chain->AddRevoluteJoint(nLinks-1, nLinks, pivot, btVector3(0,0,1)); //Revolve always around local Z axis, no collision between joint links
		
		//Update trasformation matrix
		btTransform t1(btQuaternion::getIdentity(), btVector3(0,0,d));
		btTransform t2(btQuaternion(btVector3(1,0,0), alpha), btVector3(a,0,0));
		lastDHTrans *= t1 * t2;
		
		//Add motor
		Motor* motor = new Motor(getName() + "/Motor" + std::to_string(nLinks), chain, nLinks-1);
		motors.push_back(motor);
		
		//Add encoder
		FakeRotaryEncoder* enc = new FakeRotaryEncoder(getName() + "/Encoder" + std::to_string(nLinks), chain, nLinks-1);
		encoders.push_back(enc);
		
		//Add servo controller
		ServoController* ctrl = new ServoController(getName() + "/Controller" + std::to_string(nLinks), motor, enc, btScalar(1000));
		ctrl->SetPosition(0);
		ctrl->SetGains(btScalar(200), btScalar(20), btScalar(5), btScalar(100));
		ctrl->Start();
		controllers.push_back(ctrl);
		
		//Link successfully created
		++nLinks;
	}
}

void Manipulator::AddToDynamicsWorld(btMultiBodyDynamicsWorld* world, const btTransform& worldTransform)
{
	chain->AddToDynamicsWorld(world);
	
	if(attach != NULL)
	{
		FixedJoint* fixed = new FixedJoint(getName() + "/Fix", chain->getMultiBody(), attach);
		fixed->AddToDynamicsWorld(world);
	}
}

void Manipulator::UpdateAcceleration()
{	
}

void Manipulator::UpdateSensors(btScalar dt)
{
    for(unsigned int i=0; i<nLinks-1; ++i)
		encoders[i]->Update(dt);
}

void Manipulator::UpdateControllers(btScalar dt)
{
    for(unsigned int i=0; i<nLinks-1; ++i)
		controllers[i]->Update(dt);
}

void Manipulator::UpdateActuators(btScalar dt)
{
    for(unsigned int i=0; i<nLinks-1; ++i)
		motors[i]->Update(dt);
}

void Manipulator::ApplyGravity(const btVector3& g)
{
    chain->ApplyGravity(g);
}

void Manipulator::ApplyFluidForces(Ocean* fluid)
{
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