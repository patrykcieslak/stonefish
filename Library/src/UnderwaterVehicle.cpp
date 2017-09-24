//
//  UnderwaterVehicle.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 17/05/17.
//  Copyright(c) 2017 Patryk Cieslak. All rights reserved.
//

#include "UnderwaterVehicle.h"
#include "SimulationApp.h"

UnderwaterVehicle::UnderwaterVehicle(std::string uniqueName, SolidEntity* bodySolid, const btTransform& worldTrans) : SystemEntity(uniqueName)
{
    vehicleBody = new FeatherstoneEntity(uniqueName + "/FE", 1, bodySolid, worldTrans, SimulationApp::getApp()->getSimulationManager()->getDynamicsWorld(), false);
    showInternals = false;
    lastLinearVel = btVector3(0,0,0);
    lastAngularVel = btVector3(0,0,0);
    linearAcc = btVector3(0,0,0);
    angularAcc = btVector3(0,0,0);
}

UnderwaterVehicle::~UnderwaterVehicle()
{
    //Destroy subsystems......
}

btTransform UnderwaterVehicle::getTransform() const
{
    if(vehicleBody != NULL)
        return vehicleBody->getLinkTransform(0);
    else
        return btTransform::getIdentity();
}

FeatherstoneEntity* UnderwaterVehicle::getVehicleBody()
{
	return vehicleBody;
}

void UnderwaterVehicle::AddToDynamicsWorld(btMultiBodyDynamicsWorld* world, const btTransform& worldTransform)
{
	if(vehicleBody != NULL)
		vehicleBody->AddToDynamicsWorld(world);
}

void UnderwaterVehicle::UpdateAcceleration(btScalar dt)
{
    if(vehicleBody != NULL)
    {
		btVector3 linearVel = vehicleBody->getLinkLinearVelocity(0);
		linearAcc = (linearVel - lastLinearVel)/dt;
		lastLinearVel = linearVel;
		
		btVector3 angularVel = vehicleBody->getLinkAngularVelocity(0);
		angularAcc = (angularVel - lastAngularVel)/dt;
		lastAngularVel = angularVel;
		
		//std::cout << "Acc: " << linearAcc.x() << ", " << linearAcc.y() << ", " << linearAcc.z() << std::endl;
    }
}

void UnderwaterVehicle::UpdateSensors(btScalar dt)
{
    for(unsigned int i=0; i<manipulators.size(); ++i)
        manipulators[i]->UpdateSensors(dt);
    
    for(unsigned int i=0; i<sensors.size(); ++i)
        sensors[i]->Update(dt);
}

void UnderwaterVehicle::UpdateControllers(btScalar dt)
{
    for(unsigned int i=0; i<manipulators.size(); ++i)
        manipulators[i]->UpdateControllers(dt);
}

void UnderwaterVehicle::UpdateActuators(btScalar dt)
{
    for(unsigned int i=0; i<manipulators.size(); ++i)
        manipulators[i]->UpdateActuators(dt);
    
    //for(unsigned int i=0; i<thrusters.size(); ++i)
    //    thrusters[i]->Update(dt);
}

void UnderwaterVehicle::ApplyGravity(const btVector3& g)
{
    if(vehicleBody != NULL)
			vehicleBody->ApplyGravity(g);
}

void UnderwaterVehicle::ApplyDamping()
{
}

std::vector<Renderable> UnderwaterVehicle::Render()
{
	if(vehicleBody != NULL)
		return vehicleBody->Render();
	else
	{
		std::vector<Renderable> items(0);
		return items;
	}
}

void UnderwaterVehicle::GetAABB(btVector3& min, btVector3& max)
{
	if(vehicleBody != NULL)
		vehicleBody->GetAABB(min, max);
	else
	{
		min = btVector3(0,0,0);
		max = btVector3(0,0,0);
	}
}