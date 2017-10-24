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
	chain->EnableSelfCollision(); //Enable collision between links
	attach = NULL;
}

Manipulator::Manipulator(std::string uniqueName, unsigned int numOfLinks, SolidEntity* baseLink, const btTransform& geomToJoint, FeatherstoneEntity* attachment) : SystemEntity(uniqueName)
{
	chain = new FeatherstoneEntity(uniqueName + "/FE", numOfLinks+1, baseLink, SimulationApp::getApp()->getSimulationManager()->getDynamicsWorld(), false);
	nTotalLinks = numOfLinks+1;
	nLinks = 1;
	DH.push_back(geomToJoint);
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

void Manipulator::AddRotLinkDH(SolidEntity* link, Motor* motor, const btTransform& geomToJoint, btScalar d, btScalar a, btScalar alpha, btScalar lowerLimit, btScalar upperLimit)
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
		chain->setJointDamping(nLinks-1, 0, 0.5);
		
		//Update trasformation matrix
		btTransform t1(btQuaternion::getIdentity(), btVector3(0,0,d));
		btTransform t2(btQuaternion(btVector3(1,0,0), alpha), btVector3(a,0,0));
		DH.push_back(DH.back() * t1 * t2);
		
		//Add motor
        motor->AttachToJoint(chain, nLinks-1);
		motors.push_back(motor);
		
		//Add encoder
		FakeRotaryEncoder* enc = new FakeRotaryEncoder(getName() + "/Encoder" + std::to_string(nLinks), motor);
		encoders.push_back(enc);
		
		//Add servo controller
		ServoController* ctrl = new ServoController(getName() + "/Controller" + std::to_string(nLinks), motor, enc, btScalar(24.0));
		ctrl->SetPosition(0.0);
		ctrl->SetGains(10,1,0.1,10);
		ctrl->Start();
		controllers.push_back(ctrl);
        
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
}

void Manipulator::UpdateSensors(btScalar dt)
{
	if(encoders.size() != nLinks-1)
		return;
	
    for(unsigned int i=0; i<nLinks-1; ++i)
		encoders[i]->Update(dt);
}

void Manipulator::UpdateControllers(btScalar dt)
{
	if(controllers.size() != nLinks-1)
		return;
		
	for(unsigned int i=0; i<nLinks-1; ++i)
		controllers[i]->Update(dt);
}

void Manipulator::UpdateActuators(btScalar dt)
{
	if(motors.size() != nLinks-1)
		return;
	
    for(unsigned int i=0; i<nLinks-1; ++i)
		motors[i]->Update(dt);
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

btScalar Manipulator::getJointPosition(unsigned int jointId)
{
	if(jointId >= nLinks-1)
		return btScalar(0);

	return encoders[jointId]->getLastValueExternal(0);
}

btScalar Manipulator::getDesiredJointPosition(unsigned int jointId)
{
	if(jointId >= nLinks-1)
		return btScalar(0);
		
	return controllers[jointId]->getReferenceValues()[0];
}

void Manipulator::setDesiredJointPosition(unsigned int jointId, btScalar position)
{
	if(jointId >= nLinks-1)
		return;
		
	controllers[jointId]->SetPosition(position);
}

const std::vector<btTransform>& Manipulator::getDH()
{
    return DH;
}