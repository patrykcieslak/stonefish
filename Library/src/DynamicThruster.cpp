//
//  DynamicThruster.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 16/09/17.
//  Copyright (c) 2017 Patryk Cieslak. All rights reserved.
//

#include "DynamicThruster.h"
#include "UnderwaterVehicle.h"
#include "DCMotor.h"

DynamicThruster::DynamicThruster(std::string uniqueName, UnderwaterVehicle* vehicle, SolidEntity* duct, SolidEntity* propeller, btScalar propDiameter, btScalar thrustCoeff) : SystemEntity(uniqueName)
{
	//Set parameters
	propLocation = btTransform::getIdentity();//UnitSystem::SetTransform(location);
	actuatedVehicle = vehicle;
	ductSolid = duct;
	propSolid = propeller;
	D = UnitSystem::SetLength(propDiameter);
	KT = thrustCoeff;
	thruster = NULL;
	
	ductSolid->setComputeHydrodynamics(false);
	propSolid->setComputeHydrodynamics(false);
}	

DynamicThruster::~DynamicThruster()
{
	//Delete components
	delete thruster;
	delete motor;
	delete enc;
	delete ctrl;
}

void DynamicThruster::SetDesiredSpeed(btScalar s)
{
	ctrl->SetSpeed(s);
}

void DynamicThruster::AddToDynamicsWorld(btMultiBodyDynamicsWorld* world, const btTransform& worldTransform)
{
	if(thruster != NULL)
		return;
	
	//Build thruster
	thruster = new FeatherstoneEntity(getName() + "/FE", 2, ductSolid, propLocation, world, false);
	thruster->AddLink(propSolid, propLocation, world);
	thruster->AddRevoluteJoint(0, 1, propLocation.getOrigin(), propLocation.getBasis().getColumn(0)); //Revolve always around local X axis, no collision between joint links
	thruster->setJointDamping(0, 0, 0);
	thruster->setBaseTransform(worldTransform);
	thruster->AddToDynamicsWorld(world);
	
	FixedJoint* fixed = new FixedJoint(getName() + "/Fix", actuatedVehicle->getVehicleBody(), thruster);
	fixed->AddToDynamicsWorld(world);
	
	DCMotor* dc = new DCMotor(getName() + "/Motor", thruster, 0, 0.3, 0.0744e-3, 1.0/408.0, 23.4e-3, 0); //new Motor(getName() + "/Motor", thruster, 0);
	dc->SetupGearbox(true, 5.0, 0.9);
	motor = dc;
	
	enc = new FakeRotaryEncoder(getName() + "/Encoder", thruster, 0);
	
	ctrl = new SpeedController(getName() + "/Controller", motor, enc, 12.0);
	ctrl->SetGains(1.0,0.5,0,100.0);
	ctrl->Start();
}

void DynamicThruster::UpdateAcceleration(btScalar dt) 
{
}

void DynamicThruster::UpdateSensors(btScalar dt)
{
	enc->Update(dt);
	//std::cout << "Speed: " << enc->getLastSample().getValue(1) << std::endl;
}
    
void DynamicThruster::UpdateControllers(btScalar dt)
{
	ctrl->Update(dt);
}
    
void DynamicThruster::UpdateActuators(btScalar dt)
{
	motor->Update(dt);
	
	//Calculate and apply thrust
	btScalar omega = enc->getLastSample().getValue(1);
	btScalar thrust = KT*1000.0*omega*omega*D*D*D*D;
	
	btTransform thTrans = thruster->getLinkTransform(0);
	btTransform vTrans = actuatedVehicle->getTransform();
	btVector3 F = thTrans.getBasis().getColumn(0) * thrust;
	
	actuatedVehicle->getVehicleBody()->getMultiBody()->addBaseForce(F);
	actuatedVehicle->getVehicleBody()->getMultiBody()->addBaseTorque((thTrans.getOrigin() - vTrans.getOrigin()).cross(F));
}
    
void DynamicThruster::ApplyGravity(const btVector3& g)
{
	thruster->ApplyGravity(g);
}

void DynamicThruster::ApplyDamping()
{
	thruster->ApplyDamping();
}
    
btTransform DynamicThruster::getTransform() const
{
	return thruster->getMultiBody()->getBaseWorldTransform();
}
    
std::vector<Renderable> DynamicThruster::Render()
{
	return thruster->Render();
}
	
void DynamicThruster::GetAABB(btVector3& min, btVector3& max)
{
	thruster->GetAABB(min, max);
}